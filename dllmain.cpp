// ================================================================
// V.I.I.B.E CS 1.6 Modular Diagnostics & ESP Suite v3.0
// Clean Modular Architecture (SDK, Math, Render, Engine, Hooks, Features)
// ================================================================

#include "src/sdk/sdk.hpp"
#include "src/core/logger.hpp"
#include "src/core/math.hpp"
#include "src/render/renderer.hpp"
#include "src/engine/engine.hpp"
#include "src/hooks/hooks.hpp"
#include "src/features/esp.hpp"
#include "src/features/radar.hpp"
#include "src/features/misc.hpp"
#include "src/features/config.hpp"
#include "src/features/menu.hpp"
#include <psapi.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "psapi.lib")

static HMODULE g_hDll    = NULL;
static bool    g_running = true;
static PVOID   g_pVeh    = NULL;

static LONG WINAPI VectoredCrashHandler(PEXCEPTION_POINTERS pEx) {
    if (pEx && pEx->ExceptionRecord) {
        DWORD code = pEx->ExceptionRecord->ExceptionCode;
        if (code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_ILLEGAL_INSTRUCTION || code == EXCEPTION_PRIV_INSTRUCTION) {
            uintptr_t crashAddr = (uintptr_t)pEx->ExceptionRecord->ExceptionAddress;
            Logger::Log("[CRASH TRAP] Exception 0x%08X at 0x%08X", (unsigned)code, (unsigned)crashAddr);
            if (pEx->ContextRecord) {
                Logger::Log("[CRASH DUMP] EAX:0x%08X EBX:0x%08X ECX:0x%08X EDX:0x%08X ESI:0x%08X EDI:0x%08X EIP:0x%08X",
                            (unsigned)pEx->ContextRecord->Eax, (unsigned)pEx->ContextRecord->Ebx,
                            (unsigned)pEx->ContextRecord->Ecx, (unsigned)pEx->ContextRecord->Edx,
                            (unsigned)pEx->ContextRecord->Esi, (unsigned)pEx->ContextRecord->Edi,
                            (unsigned)pEx->ContextRecord->Eip);
            }
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

struct KeyTracker {
    bool prev;
    bool Pressed(int vk) {
        bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
        bool hit = down && !prev;
        prev = down;
        return hit;
    }
};

static DWORD WINAPI MainThread(LPVOID param) {
    g_pVeh = AddVectoredExceptionHandler(1, VectoredCrashHandler);

    Logger::Init(g_hDll);
    Config::Init(g_hDll);
    Config::Load(Menu::g_state);

    Logger::Log("================================================================================");
    Logger::Log("[V.I.I.B.E Modular CS 1.6 Suite v3.0]");
    Logger::Log("================================================================================");
    Logger::Log("[+] Injected DLL Base: 0x%08X", (unsigned)(uintptr_t)g_hDll);

    Logger::Log("[+] Diagnostic Log Target: %s", Logger::GetLogPath());
    Logger::Log("[+] Config Path: %s", Config::GetConfigPath());

    while (g_running && !GetModuleHandleA("hw.dll") && !GetModuleHandleA("client.dll")) {
        Sleep(100);
    }
    if (!g_running) goto cleanup;

    {
        HMODULE hEngine = GetModuleHandleA("hw.dll");
        if (hEngine) {
            Engine::g_hwBase = (uintptr_t)hEngine;
            MODULEINFO mi = {0};
            if (GetModuleInformation(GetCurrentProcess(), hEngine, &mi, sizeof(mi))) {
                Engine::g_hwSize = mi.SizeOfImage;
            }
        }
    }

    Engine::ResolveFunctions();

    Logger::Log("[*] Installing Engine & OpenGL Hooks...");
    if (Hooks::Install()) {
        Logger::Log("[+] Hooks active. Controls: INSERT (Menu) | F11 (Log Dump) | END (Eject)");
    } else {
        Logger::Log("[-] FATAL: Hook installation failed — Aborting");
        goto cleanup;
    }

    {
        KeyTracker kF11 = {false};

        while (g_running) {
            if (GetAsyncKeyState(VK_END) & 0x8000) {
                Logger::Log("[+] END key detected — Ejecting V.I.I.B.E DLL");
                g_running = false;
                break;
            }

            if (kF11.Pressed(VK_F11)) {
                Logger::Log("[+] F11 pressed — Re-resolving engine functions & dumping");
                Engine::ResolveFunctions();
                Logger::DumpDiagnosticSnapshot(true, Hooks::g_frameCount, Hooks::g_currentFps, 1920, 1080,
                                               ESP::g_cachedPlayers, ESP::g_cachedValidCount, ESP::g_cachedOnScreenCount);
            }

            Menu::HandleInput();
            Sleep(16);
        }
    }

    if (Hooks::g_hooked) {
        Hooks::Remove();
        Sleep(100);
    }

cleanup:
    Config::Save(Menu::g_state);
    Logger::Log("[+] V.I.I.B.E DLL Shutting down. Total frames rendered: %llu", (unsigned long long)Hooks::g_frameCount);
    Logger::DumpDiagnosticSnapshot(true, Hooks::g_frameCount, Hooks::g_currentFps, 1920, 1080,
                                   ESP::g_cachedPlayers, ESP::g_cachedValidCount, ESP::g_cachedOnScreenCount);
    Logger::Shutdown();

    if (g_pVeh) {
        RemoveVectoredExceptionHandler(g_pVeh);
        g_pVeh = NULL;
    }

    FreeLibraryAndExitThread(g_hDll, 0);
    return 0;

}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hDll = hModule;
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
        if (hThread) CloseHandle(hThread);
    }
    return TRUE;
}
