//
// Created by Abdul on 2/3/2026.
//

#include "cpuInfo.h"

#include <array>
#include <windows.h>
#include <iostream>
#include <intrin.h>
#include <memory>

staticCPU cpu;
using namespace std;

staticCPU getStaticCPUInfo() {
    cpu.vendor = getCPUVendor();
    cpu.modelName = getCPUModelName();
    getFamilyModel(cpu.family, cpu.model, cpu.stepping);


    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    cpu.logicalThreads = sysInfo.dwNumberOfProcessors;
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

    auto buffer = static_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(malloc(len));


    if (GetLogicalProcessorInformation(buffer, &len)) {
        DWORD count = len / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
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
                        cpu.l1CacheMB += info.Cache.Size / 1024 / 1024;
                        break;

                    case 2:
                        cpu.l2CacheMB += info.Cache.Size / 1024 / 1024;
                        break;

                    case 3:
                        cpu.l3CacheMB += info.Cache.Size / 1024 / 1024;
                        break;

                }
            }
        }
        free(buffer);

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

