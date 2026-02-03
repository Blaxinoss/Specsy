//
// Created by Abdul on 2/3/2026.
//

#include "cpuInfo.h"

#include <array>
#include <windows.h>
#include <iostream>
#include <intrin.h>
#include <memory>
#include <powrprof.h>
#include <algorithm>
#include <vector>

typedef struct _PROCESSOR_POWER_INFORMATION {
    unsigned long Number;
    unsigned long MaxMhz;
    unsigned long CurrentMhz;
    unsigned long MhzLimit;
    unsigned long MaxIdleState;
    unsigned long CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;



using namespace std;
staticCPU cpu;



static double toGHz(int mhz) {
    return mhz / 1000.0;
}

static unsigned long long FileTimeToInt64(const FILETIME &ft) {
    return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}



void getCPUSpeeds(double &base, double &current, staticCPU& cpu) { // شيلنا max من هنا كمان

    int numProcessors = cpu.logicalThreads;
    if (numProcessors <= 0) return;


    // حجز مكان لعدد N من التقارير
    std::vector<PROCESSOR_POWER_INFORMATION> ppi(numProcessors);

    long status = CallNtPowerInformation(
        ProcessorInformation,
        nullptr,
        0,
        &ppi[0],
        sizeof(PROCESSOR_POWER_INFORMATION) * numProcessors
    );

    if (status == 0) {
        // ppi[0] تعني أننا نأخذ بيانات النواة الأولى فقط كعينة
        base = toGHz(ppi[0].MaxMhz);
        current = toGHz(ppi[0].CurrentMhz);
    }
}


float calculateCPULoad() {
    FILETIME idleTime, kernelTime, userTime;

    GetSystemTimes(&idleTime, &kernelTime, &userTime);

    unsigned long long idle1 = FileTimeToInt64(idleTime);
    unsigned long long kernel1 = FileTimeToInt64(kernelTime);
    unsigned long long user1 = FileTimeToInt64(userTime);

    Sleep(1000);

    GetSystemTimes(&idleTime, &kernelTime, &userTime);

    unsigned long long idle2 = FileTimeToInt64(idleTime);
    unsigned long long kernel2 = FileTimeToInt64(kernelTime);
    unsigned long long user2 = FileTimeToInt64(userTime);

    unsigned long long idleDelta = idle2 - idle1;
    unsigned long long kernelDelta = kernel2 - kernel1;
    unsigned long long userDelta = user2 - user1;


    // في ويندوز: KernelTime يشمل بداخله IdleTime
    // إذن الوقت الكلي = Kernel + User
    unsigned long long totalSys = kernelDelta + userDelta;

    if (totalSys == 0) return 0.0f;

    // (active = total - idle)
    float percent = (float)(totalSys - idleDelta) / totalSys;

    return percent * 100.0f;
}

liveCPU getLiveCPUInfo() {
    liveCPU live;
    live.loadPercent = calculateCPULoad();


    double dummyBase;
    getCPUSpeeds(dummyBase, live.currentGHz, cpu);

    return live;
}

staticCPU getStaticCPUInfo() {
    cpu.vendor = getCPUVendor();
    cpu.modelName = getCPUModelName();
    getFamilyModel(cpu.family, cpu.model, cpu.stepping);


    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    cpu.logicalThreads = sysInfo.dwNumberOfProcessors;

    getCPUSpeeds(cpu.baseGHz, cpu.currentGHz, cpu);


    switch(sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_INTEL:
            cpu.architecture = "x86";
            break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            cpu.architecture = "x64";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            cpu.architecture = "ARM";
            break;
        default:
            cpu.architecture = "Unknown";
            break;
    }

    DWORD len = 0;
    GetLogicalProcessorInformation(nullptr,&len);

    std::vector<char> bufferVec(len);
    auto buffer = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(bufferVec.data());

    if (GetLogicalProcessorInformation(buffer, &len)) {
        DWORD count = len / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

        cpu.physicalCores = 0;
        cpu.numaNodes = 0;
        cpu.l1CacheMB = 0;
        cpu.l2CacheMB = 0;
        cpu.l3CacheMB = 0;

        for (DWORD i = 0; i < count; i++) {
            SYSTEM_LOGICAL_PROCESSOR_INFORMATION info = buffer[i];
            if (info.Relationship == RelationProcessorCore) {
                cpu.physicalCores++;
            }

            if (info.Relationship == RelationNumaNode) {
                cpu.numaNodes++;
            }

            if (info.Relationship == RelationCache) {
                switch (info.Cache.Level) {
                    case 1:
                        cpu.l1CacheMB += info.Cache.Size / 1024.0 / 1024.0 ;
                        break;

                    case 2:
                        cpu.l2CacheMB += info.Cache.Size / 1024.0 / 1024.0 ;
                        break;

                    case 3:
                        cpu.l3CacheMB += info.Cache.Size / 1024.0 / 1024.0 ;
                        break;

                }
            }
        }

    }
    return cpu;
}



string getCPUVendor() {
    int CPU_REG_DATA[4];
    __cpuid(CPU_REG_DATA,0);
    char vendor[13];
    memcpy(vendor,&CPU_REG_DATA[1],4); // EBX
    memcpy(vendor+4,&CPU_REG_DATA[3],4); //EDX
    memcpy(vendor+8,&CPU_REG_DATA[2],4); // ECX
    vendor[12] = '\0';

    return string(vendor);

}

// حركة الـ pointer تعتمد على النوع
// int*  + 1   (1 here means 4 bytes)
// char* + 1   (1 here means 1 byte)
// std::string getCPUModelName() {
//     int CPUInfo[4];
//     char model[49];
//     for (int i = 0; i < 3; i++) {
//         __cpuid(CPUInfo, 0x80000002 + i);
//         memcpy(model + i*16, CPUInfo, 16);
//     }
//     model[48] = '\0';
//     return std::string(model);
// }

string getCPUModelName() {
    array<int, 4> CPUInfoVendor;  // [int 1 , int 2 , int 3 , int 4]
    array<char, 49> model{}; //['aaaaaaa.....'] char is 1 byte so we expecting 49 byte
      for (int i = 0; i < 3; i++) {  // 3 calls to get the full Model
         __cpuid(CPUInfoVendor.data(), 0x80000002 + i);  // 0x80000002..4
         ranges::copy_n(
             reinterpret_cast<char*>(CPUInfoVendor.data()),
             16,
             model.begin() + i*16 );
      }
    model[48] = '\0';
    return string(model.data());
}


void getFamilyModel(int &family, int &model, int &stepping) {
    array<int, 4> CPUInfoFamily;
    __cpuid(CPUInfoFamily.data(), 1);

    int rawFamily = (CPUInfoFamily[0] >> 8) & 0xF;
    int rawModel  = (CPUInfoFamily[0] >> 4) & 0xF;
    int steppingRaw = CPUInfoFamily[0] & 0xF;
    int extModel = (CPUInfoFamily[0] >> 16) & 0xF;
    int extFamily = (CPUInfoFamily[0] >> 20) & 0xFF;

    family = (rawFamily == 15) ? rawFamily + extFamily : rawFamily;
    model = ((rawFamily == 6 || rawFamily == 15) ? (extModel << 4) + rawModel : rawModel);
    stepping = steppingRaw;
}

