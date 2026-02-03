//
// Created by Abdul on 2/3/2026.
//

#include <string>

#ifndef CODEFORCES_CPUINFO_H
#define CODEFORCES_CPUINFO_H

struct liveCPU {


    //  Live Metrics
     // double temperatureC;
    double loadPercent;
    double currentGHz;
    // double powerWatts;
};

struct staticCPU {
    std::string vendor;
    std::string modelName;
    std::string architecture;


    int family;
    int model;
    int stepping;


    int physicalCores;
    int logicalThreads;
    int numaNodes;

    double baseGHz;
    double currentGHz;


    double l1CacheMB;
    double l2CacheMB;
    double l3CacheMB;

};


staticCPU getStaticCPUInfo();
liveCPU getLiveCPUInfo();


std::string getCPUVendor();
std::string getCPUModelName();
void getFamilyModel(int &family, int &model, int &stepping);

float calculateCPULoad();
void getCPUSpeeds(double &base, double &current, staticCPU& cpu);


#endif //CODEFORCES_CPUINFO_H