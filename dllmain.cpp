// ================================================================
// V.I.I.B.E CS 1.6 Modular Diagnostics & ESP Suite v3.0
// DLL Entry Point
// ================================================================

#include "src/core/framework.hpp"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "psapi.lib")

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        Framework::Init(hModule);
    }
    return TRUE;
}
