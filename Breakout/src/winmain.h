#ifndef WINMAIN_H
#define WINMAIN_H

#include "core.h"

/*
    For some reason clang preferably links against main() if available even though the build
    settings would link against WinMain().
    However in order to link against WinMain() I created this macro.
    GCC & MSVC properly link against WinMain().
    Premake will define all these macros for you if you select clang as compiler.
*/

#if defined TOOLCHAIN_CLANG && defined RELEASE && defined SYSTEM_WINDOWS
    #ifdef __cplusplus
        #ifdef _WINBASE_
            #define main() WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
        #else
            #define main() __stdcall WinMain(void*, void*, char*, int)
        #endif // _WINBASE_
    #else
        #ifdef _WINBASE_
            #define main() WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cmdline, int cmdshow)
        #else
            #define main() __stdcall WinMain(void* hInst, void* hInstPrev, char* cmdline, int cmdshow)
        #endif // _WINBASE_
    #endif // __cplusplus
#endif

#endif // WINMAIN_H