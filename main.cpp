#include <iostream>
#include <iomanip>
#include "cpuInfo.h"

int main() {
    std::cout << "Gathering System Info..." << std::endl;

    staticCPU info = getStaticCPUInfo();

    std::cout << "---------------------------------" << std::endl;
    std::cout << "CPU Model: " << info.modelName << std::endl;
    std::cout << "Vendor:    " << info.vendor << std::endl;
    std::cout << "Arch:      " << info.architecture << std::endl;
    std::cout << "Cores:     " << info.physicalCores << " Phys / " << info.logicalThreads << " Threads" << std::endl;
    std::cout << "Cache:     L1: " << info.l1CacheMB << "MB, L2: " << info.l2CacheMB << "MB, L3: " << info.l3CacheMB << "MB" << std::endl;
    std::cout << "Base Speed:" << info.baseGHz << " GHz" << std::endl;
    std::cout << "---------------------------------" << std::endl;

    std::cout << "Monitoring Live Data (Press Ctrl+C to stop)..." << std::endl;

    while (true) {
        liveCPU live = getLiveCPUInfo();

        std::cout << "\rLoad: " << std::fixed << std::setprecision(1) << live.loadPercent << "% "
                  << "| Speed: " << std::setprecision(2) << live.currentGHz << " GHz    " << std::flush;
    //calculateCPULoad has sleep on it no need to add it here more
    }

    return 0;
}