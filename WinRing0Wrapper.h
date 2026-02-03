// //
// // Created by Abdul on 2/3/2026.
// //
//
// #ifndef CODEFORCES_WINRING0WRAPPER_H
// #define CODEFORCES_WINRING0WRAPPER_H
// #ifndef WINRING0_WRAPPER_H
// #define WINRING0_WRAPPER_H
//
// #include <windows.h>
// #include <iostream>
//
// typedef int (WINAPI *_InitializeOls)();
// typedef void (WINAPI *_DeinitializeOls)();
// typedef int (WINAPI *_Rdmsr)(DWORD index, DWORD *eax, DWORD *edx);
//
// class WinRing0 {
//     HMODULE hModule;
//     _InitializeOls InitializeOls;
//     _DeinitializeOls DeinitializeOls;
//     _Rdmsr Rdmsr;
//
// public:
//     bool isLoaded = false;
//
//     WinRing0() {
//         hModule = LoadLibrary("WinRing0x64.dll");
//         if (hModule) {
//             InitializeOls = (_InitializeOls)GetProcAddress(hModule, "InitializeOls");
//             DeinitializeOls = (_DeinitializeOls)GetProcAddress(hModule, "DeinitializeOls");
//             Rdmsr = (_Rdmsr)GetProcAddress(hModule, "Rdmsr");
//
//             if (InitializeOls && Rdmsr) {
//                 // محاولة تشغيل الدرايفر
//                 if (InitializeOls() != 0) {
//                     isLoaded = true;
//                 }
//             }
//         }
//     }
//
//     ~WinRing0() {
//         if (isLoaded && DeinitializeOls) {
//             DeinitializeOls();
//         }
//         if (hModule) {
//             FreeLibrary(hModule);
//         }
//     }
//
//     bool readMSR(DWORD index, DWORD &eax, DWORD &edx) {
//         if (!isLoaded) return false;
//         return Rdmsr(index, &eax, &edx) != 0;
//     }
// };
//
// #endif
// #endif //CODEFORCES_WINRING0WRAPPER_H