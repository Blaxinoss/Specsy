cmake_minimum_required(VERSION 4.1)
project(codeforces)

set(CMAKE_CXX_STANDARD 20)

add_executable(codeforces main.cpp
        cpuInfo.h
        cpuInfo.cpp
        WinRing0Wrapper.h
        Wmi.cpp
        WmiClient.h
)


target_link_libraries(codeforces PowrProf wbemuuid
        ole32
        oleaut32)