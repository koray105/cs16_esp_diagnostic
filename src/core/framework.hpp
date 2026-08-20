#ifndef FRAMEWORK_HPP
#define FRAMEWORK_HPP

#include <windows.h>

namespace Framework {
    extern HMODULE g_hDll;
    extern bool    g_running;

    void Init(HMODULE hModule);
    DWORD WINAPI MainThread(LPVOID param);
    void Shutdown();
}

#endif // FRAMEWORK_HPP
