#include "resolver.hpp"
#include "memory.hpp"
#include "../core/logger.hpp"
#include <psapi.h>

namespace Engine {
    uintptr_t g_hwBase          = 0;
    DWORD     g_hwSize          = 0;
    uintptr_t g_clientBase      = 0;
    DWORD     g_clientSize      = 0;
    addr_t    g_pEngfuncsAddr   = 0;
    uintptr_t* g_pEngfuncsTable = nullptr;

    pfnClientCmd_t        g_fnClientCmd        = nullptr;
    pfnGetViewAngles_t   g_fnGetViewAngles    = nullptr;
    pfnSetViewAngles_t   g_fnSetViewAngles    = nullptr;
    pfnGetPlayerInfo_t    g_fnGetPlayerInfo    = nullptr;
    pfnTraceLine_t       g_fnTraceLine        = nullptr;
    pfnGetLocalPlayer_t   g_fnGetLocalPlayer   = nullptr;
    pfnGetEntityByIndex_t g_fnGetEntityByIndex = nullptr;
    pfnHUD_GetPlayerTeam_t g_fnHUD_GetPlayerTeam = nullptr;

    bool ResolveFunctions() {
        HMODULE hClient = GetModuleHandleA("client.dll");
        if (!hClient) {
            Logger::Log("[-] client.dll not found in process space yet");
            return false;
        }
        g_clientBase = (uintptr_t)hClient;
        MODULEINFO mi = {0};
        if (GetModuleInformation(GetCurrentProcess(), hClient, &mi, sizeof(mi))) {
            g_clientSize = mi.SizeOfImage;
        }
        if (!g_clientSize) g_clientSize = 0x300000;

        g_fnHUD_GetPlayerTeam = (pfnHUD_GetPlayerTeam_t)GetProcAddress(hClient, "HUD_GetPlayerTeam");

        BYTE* pInit = (BYTE*)GetProcAddress(hClient, "Initialize");
        if (pInit) {
            Logger::Log("[+] client.dll::Initialize at 0x%08X", (unsigned)(uintptr_t)pInit);

            for (int i = 0; i < 48; i++) {
                if (pInit[i] == 0xBF) {
                    addr_t candidate = *(addr_t*)(pInit + i + 1);
                    if (candidate >= 0x10000 && candidate <= 0x7FFF0000 && IsReadableFast((const void*)(uintptr_t)candidate, 532)) {
                        g_pEngfuncsAddr = candidate;
                        g_pEngfuncsTable = (uintptr_t*)candidate;
                        Logger::Log("[+] Found gEngfuncs table via 'mov edi, 0x%08X'", (unsigned)candidate);
                        break;
                    }
                }
                if (pInit[i] == 0xA3) {
                    addr_t candidate = *(addr_t*)(pInit + i + 1);
                    if (candidate >= 0x10000 && candidate <= 0x7FFF0000 && IsReadableFast((const void*)(uintptr_t)candidate, 532)) {
                        g_pEngfuncsAddr = candidate;
                        g_pEngfuncsTable = (uintptr_t*)candidate;
                        Logger::Log("[+] Found gEngfuncs pointer via 'mov [0x%08X], eax'", (unsigned)candidate);
                        break;
                    }
                }
            }
        }

        if (!g_pEngfuncsTable) {
            addr_t knownRVA = 0x00121BA0;
            addr_t candidate = g_clientBase + knownRVA;
            if (IsReadableFast((const void*)(uintptr_t)candidate, 532)) {
                g_pEngfuncsAddr = candidate;
                g_pEngfuncsTable = (uintptr_t*)candidate;
                Logger::Log("[+] Resolved gEngfuncs table via validated RVA 0x%08X (VA: 0x%08X)", (unsigned)knownRVA, (unsigned)candidate);
            }
        }

        if (!g_pEngfuncsTable && g_hwBase) {
            addr_t candidate = g_hwBase + 0x166A98;
            if (IsReadableFast((const void*)(uintptr_t)candidate, 532)) {
                g_pEngfuncsAddr = candidate;
                g_pEngfuncsTable = (uintptr_t*)candidate;
                Logger::Log("[+] Resolved gEngfuncs table via hw.dll source table (VA: 0x%08X)", (unsigned)candidate);
            }
        }

        if (g_pEngfuncsTable) {
            if (IsReadableFast((const void*)g_pEngfuncsTable[20], 16)) g_fnClientCmd = (pfnClientCmd_t)g_pEngfuncsTable[20];
            if (IsReadableFast((const void*)g_pEngfuncsTable[21], 16)) g_fnGetPlayerInfo = (pfnGetPlayerInfo_t)g_pEngfuncsTable[21];

            for (int idx = 31; idx <= 34; idx++) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[idx], 16)) {
                    pfnGetViewAngles_t pTestGet = (pfnGetViewAngles_t)g_pEngfuncsTable[idx];
                    float testAngles[3] = { -9999.0f, -9999.0f, -9999.0f };
                    pTestGet(testAngles);
                    if (testAngles[0] > -180.0f && testAngles[0] < 180.0f &&
                        testAngles[1] > -360.0f && testAngles[1] < 360.0f) {
                        g_fnGetViewAngles = pTestGet;
                        if (IsReadableFast((const void*)g_pEngfuncsTable[idx + 1], 16)) {
                            g_fnSetViewAngles = (pfnSetViewAngles_t)g_pEngfuncsTable[idx + 1];
                        }
                        break;
                    }
                }
            }
            if (!g_fnGetViewAngles) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[33], 16)) g_fnGetViewAngles = (pfnGetViewAngles_t)g_pEngfuncsTable[33];
                else if (IsReadableFast((const void*)g_pEngfuncsTable[32], 16)) g_fnGetViewAngles = (pfnGetViewAngles_t)g_pEngfuncsTable[32];
            }
            if (!g_fnSetViewAngles) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[34], 16)) g_fnSetViewAngles = (pfnSetViewAngles_t)g_pEngfuncsTable[34];
                else if (IsReadableFast((const void*)g_pEngfuncsTable[33], 16)) g_fnSetViewAngles = (pfnSetViewAngles_t)g_pEngfuncsTable[33];
            }

            for (int idx = 49; idx <= 52; idx++) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[idx], 16)) {
                    pfnGetLocalPlayer_t pTestLocal = (pfnGetLocalPlayer_t)g_pEngfuncsTable[idx];
                    void* pLocal = pTestLocal();
                    if (pLocal && IsReadableFast(pLocal, 0x100)) {
                        int entIndex = *(int*)pLocal;
                        if (entIndex >= 1 && entIndex <= 32) {
                            g_fnGetLocalPlayer = pTestLocal;
                            break;
                        }
                    }
                }
            }
            if (!g_fnGetLocalPlayer) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[51], 16)) g_fnGetLocalPlayer = (pfnGetLocalPlayer_t)g_pEngfuncsTable[51];
                else if (IsReadableFast((const void*)g_pEngfuncsTable[50], 16)) g_fnGetLocalPlayer = (pfnGetLocalPlayer_t)g_pEngfuncsTable[50];
            }

            for (int idx = 51; idx <= 55; idx++) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[idx], 16)) {
                    pfnGetEntityByIndex_t pTestEnt = (pfnGetEntityByIndex_t)g_pEngfuncsTable[idx];
                    void* pEnt1 = pTestEnt(1);
                    if (pEnt1 && IsReadableFast(pEnt1, 0x100)) {
                        int entIndex1 = *(int*)pEnt1;
                        if (entIndex1 == 1) {
                            g_fnGetEntityByIndex = pTestEnt;
                            break;
                        }
                    }
                }
            }
            if (!g_fnGetEntityByIndex) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[53], 16)) g_fnGetEntityByIndex = (pfnGetEntityByIndex_t)g_pEngfuncsTable[53];
                else if (IsReadableFast((const void*)g_pEngfuncsTable[52], 16)) g_fnGetEntityByIndex = (pfnGetEntityByIndex_t)g_pEngfuncsTable[52];
            }

            if (IsReadableFast((const void*)g_pEngfuncsTable[57], 16)) {
                g_fnTraceLine = (pfnTraceLine_t)g_pEngfuncsTable[57];
            } else if (IsReadableFast((const void*)g_pEngfuncsTable[56], 16)) {
                g_fnTraceLine = (pfnTraceLine_t)g_pEngfuncsTable[56];
            }

            Logger::Log("[+] Engine function bindings: ClientCmd=0x%08X | GetViewAngles=0x%08X | SetViewAngles=0x%08X | TraceLine=0x%08X | GetPlayerInfo=0x%08X | GetEntityByIndex=0x%08X | GetLocalPlayer=0x%08X",
                        (unsigned)(uintptr_t)g_fnClientCmd,
                        (unsigned)(uintptr_t)g_fnGetViewAngles,
                        (unsigned)(uintptr_t)g_fnSetViewAngles,
                        (unsigned)(uintptr_t)g_fnTraceLine,
                        (unsigned)(uintptr_t)g_fnGetPlayerInfo,
                        (unsigned)(uintptr_t)g_fnGetEntityByIndex,
                        (unsigned)(uintptr_t)g_fnGetLocalPlayer);

            return (g_fnGetPlayerInfo != nullptr && g_fnGetEntityByIndex != nullptr);
        }

        Logger::Log("[-] Failed to locate gEngfuncs table");
        return false;
    }

    bool IsTargetVisible(const Vec3& start, const Vec3& end, int ignoreEntIndex, int targetEntIndex) {
        if (!g_fnTraceLine) return true;

        float vStart[3] = { start.x, start.y, start.z };
        float vEnd[3]   = { end.x,   end.y,   end.z };

        pmtrace_t tr = {0};
        g_fnTraceLine(vStart, vEnd, 1, 0, ignoreEntIndex, &tr);

        if (tr.allsolid == 0 && tr.fraction >= 0.95f) return true;
        if (targetEntIndex > 0 && tr.ent == targetEntIndex) return true;

        return false;
    }
}
