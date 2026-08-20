#include "hooks.hpp"
#include "../sdk/sdk.hpp"
#include "../core/math.hpp"
#include "../core/logger.hpp"
#include "../render/renderer.hpp"
#include "../engine/engine.hpp"
#include "../features/esp.hpp"
#include "../features/radar.hpp"
#include "../features/misc.hpp"
#include "../features/aimbot.hpp"
#include "../features/menu.hpp"

typedef BOOL(WINAPI* SwapBuffersFn)(HDC);

namespace Hooks {
    uint64_t g_frameCount  = 0;
    float    g_currentFps  = 0.0f;
    bool     g_hooked      = false;

    static uint64_t      g_fpsCounter     = 0;
    static DWORD         g_lastFpsTime    = 0;

    static SwapBuffersFn g_origSwap       = nullptr;
    static BYTE          g_savedSwapBytes[5]  = {0};
    static BYTE*         g_pSwapTarget    = nullptr;

    static V_CalcRefdefFn g_origCalcRefdef = nullptr;
    static uintptr_t*    g_pCalcRefdefSlot = nullptr;

    static HUD_AddEntityFn g_origAddEntity = nullptr;
    static uintptr_t*    g_pAddEntitySlot  = nullptr;

    static void HkCalcRefdef(ref_params_t* pparams) {
        if (g_origCalcRefdef && Engine::IsReadableFast((const void*)g_origCalcRefdef, 16)) {
            g_origCalcRefdef(pparams);
        }

        try {
            Math::g_refdefFrames++;
            if (pparams && Engine::IsReadableFast(pparams, sizeof(ref_params_t))) {
                if (pparams->intermission != 0 || pparams->paused != 0) {
                    Math::g_camValid = false;
                    return;
                }

                Math::g_camPos.x = pparams->vieworg[0];
                Math::g_camPos.y = pparams->vieworg[1];
                Math::g_camPos.z = pparams->vieworg[2];

                Math::g_camAngles.x = pparams->viewangles[0];
                Math::g_camAngles.y = pparams->viewangles[1];
                Math::g_camAngles.z = pparams->viewangles[2];

                // ALWAYS calculate true orthonormal camera basis vectors directly from viewangles
                // In Spectator Free Look / Roaming mode, pparams->forward is not updated by the engine!
                Math::AngleVectors(Math::g_camAngles, Math::g_camForward, Math::g_camRight, Math::g_camUp);


                // Capture live ground contact and spectator flags directly from engine ref_params
                // In GoldSrc: onground == -1 is in air, onground >= 0 is on ground (0=worldspawn, >0=entity)
                Math::g_localOnGround = (pparams->onground >= 0);
                Math::g_camSpectator  = (pparams->spectator != 0);
                Math::g_camViewEntity = pparams->viewentity;

                // Dynamically fetch live zoom FOV from client.dll (AUG, SG552, AWP, Scout)
                float fov = 90.0f;
                if (!Math::g_camSpectator && Engine::g_clientBase) {
                    int iFov = 0;
                    if (Engine::SafeRead(Engine::g_clientBase + 0x11D490, iFov) && iFov > 0 && iFov <= 160) {
                        fov = (float)iFov;
                    }
                }
                Math::g_camFov = fov;

                Math::g_camValid = (fabsf(Math::g_camForward.x) > 0.0001f ||
                                    fabsf(Math::g_camForward.y) > 0.0001f ||
                                    fabsf(Math::g_camForward.z) > 0.0001f);

                // Execute high-precision aimbot angle adjustments during camera refdef
                Aimbot::Update(Menu::g_state, pparams);

                // Execute engine-synchronized Bhop inside refdef frame tick
                Misc::Update(Menu::g_state);
            }
        } catch (...) {
        }
    }

    static BOOL WINAPI HkSwapBuffers(HDC hdc) {
        if (!g_origSwap) return TRUE;

        HGLRC ctx = wglGetCurrentContext();
        if (ctx != NULL) {
            try {
                g_frameCount++;
                g_fpsCounter++;

                DWORD now = GetTickCount();
                if (now - g_lastFpsTime >= 1000) {
                    g_currentFps = (float)g_fpsCounter * 1000.0f / (float)(now - g_lastFpsTime);
                    g_fpsCounter = 0;
                    g_lastFpsTime = now;
                }

                // Prune world entities that are no longer active
                Engine::PruneWorldEntities(g_frameCount);

                // Execute engine-integrated BunnyHop and misc updates
                Misc::Update(Menu::g_state);

                int w = 800, h = 600;
                if (Render::Begin2D(w, h, hdc)) {
                    // Precalculate high-speed projection parameters for this viewport and FOV
                    Math::UpdateProjection(w, h, Math::g_camFov);

                    // Synchronize unified player frame cache once per render frame
                    Engine::UpdateAllPlayers(g_frameCount);

                    // 1. In-game Visual ESP (Boxes, Snaplines, Chams, Head Markers, Weapon Tags)
                    ESP::Render(w, h, Menu::g_state);

                    // 2. Tactical 2D Rotational Radar
                    Radar::Render(w, h, Menu::g_state);

                    // 3. Tactical Misc (FOV Circle, C4 Bomb & Grenade World Tracker)
                    Misc::Render(w, h, Menu::g_state);

                    // 4. Precision Aimbot FOV & Lock Overlay
                    Aimbot::Render(w, h, Menu::g_state);

                    // 5. Telemetry Diagnostics HUD
                    Menu::RenderHUD(w, h, g_currentFps, g_frameCount);

                    // 6. Interactive Full Multi-Tab Configuration Menu
                    Menu::Render(w, h);

                    Render::End2D();
                }

                Logger::DumpDiagnosticSnapshot(false, g_frameCount, g_currentFps, w, h,
                                               ESP::g_cachedPlayers, ESP::g_cachedValidCount, ESP::g_cachedOnScreenCount);
            } catch (...) {
            }
        }

        return g_origSwap(hdc);
    }

    static int HkAddEntity(int type, void* ent, const char* modelname) {
        try {
            if (ent && Engine::IsReadableFast(ent, 0x300)) {
                int idx = *(int*)ent;
                if (idx >= 1 && idx <= 32) {
                    Engine::g_lastActiveFrame[idx] = Hooks::g_frameCount;
                    Vec3 entOrigin;
                    if (Engine::SafeRead((addr_t)(uintptr_t)ent + 0x02C0, entOrigin)) {
                        if (entOrigin.IsValid() && !entOrigin.IsZero()) {
                            Engine::g_activeOrigins[idx] = entOrigin;
                        }
                    }
                    if (modelname) {
                        char lower[64];
                        size_t len = strlen(modelname);
                        if (len > 60) len = 60;
                        for (size_t i = 0; i < len; i++) lower[i] = (char)tolower(modelname[i]);
                        lower[len] = 0;
                        if (strstr(lower, "backpack") || strstr(lower, "w_c4") || strstr(lower, "p_c4")) {
                            Engine::g_playerHasC4[idx] = true;
                        }
                    }
                } else {
                    // World entity (C4 on ground, planted bomb, grenade, dropped weapon)
                    Engine::RegisterWorldEntity(idx, ent, modelname, Hooks::g_frameCount);
                }
            }
        } catch (...) {
        }

        if (g_origAddEntity && Engine::IsReadableFast((const void*)g_origAddEntity, 16)) {
            return g_origAddEntity(type, ent, modelname);
        }
        return 1;
    }

    static uintptr_t FindHwClientSlot(uintptr_t hwBase, DWORD hwSize, const char* funcName) {
        if (!hwBase || !hwSize) return 0;
        size_t sLen = strlen(funcName);
        for (size_t i = 0; i < hwSize - sLen; i++) {
            if (memcmp((void*)(hwBase + i), funcName, sLen) == 0 && ((char*)(hwBase + i))[sLen] == 0) {
                uintptr_t strVA = hwBase + i;
                for (size_t t = 0; t < 0x150000 && (t + 32) < hwSize; t++) {
                    BYTE* p = (BYTE*)(hwBase + 0x1000 + t);
                    if (p[0] == 0x68 && *(uintptr_t*)(p + 1) == strVA) {
                        for (int k = 5; k < 25; k++) {
                            if (p[k] == 0xA3) {
                                return *(uintptr_t*)(p + k + 1);
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }

    bool Install() {
        // 1. OpenGL Hook (5-byte prologue)
        HMODULE hGL = GetModuleHandleA("opengl32.dll");
        if (!hGL) {
            hGL = LoadLibraryA("opengl32.dll");
            if (!hGL) { Logger::Log("[-] opengl32.dll not found in process space"); return false; }
        }

        Render::Init();

        BYTE* targetSwap = (BYTE*)GetProcAddress(hGL, "wglSwapBuffers");
        if (!targetSwap) { Logger::Log("[-] wglSwapBuffers export not found in opengl32.dll"); return false; }
        g_pSwapTarget = targetSwap;

        BYTE* trampolineSwap = (BYTE*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!trampolineSwap) { Logger::Log("[-] Trampoline allocation failed (error %lu)", GetLastError()); return false; }

        BYTE standardPrologue[5] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC };
        if (targetSwap[0] == 0xE9) {
            // Already hooked: use standard Win32 5-byte prologue
            memcpy(trampolineSwap, standardPrologue, 5);
            memcpy(g_savedSwapBytes, standardPrologue, 5);
        } else {
            memcpy(trampolineSwap, targetSwap, 5);
            memcpy(g_savedSwapBytes, targetSwap, 5);
        }

        trampolineSwap[5] = 0xE9;
        uintptr_t jmpBackSwap = ((uintptr_t)targetSwap + 5) - ((uintptr_t)trampolineSwap + 10);
        memcpy(&trampolineSwap[6], &jmpBackSwap, 4);

        g_origSwap = (SwapBuffersFn)trampolineSwap;

        DWORD oldProt;
        if (!VirtualProtect(targetSwap, 5, PAGE_EXECUTE_READWRITE, &oldProt)) {
            VirtualFree(trampolineSwap, 0, MEM_RELEASE);
            return false;
        }

        targetSwap[0] = 0xE9;
        uintptr_t relJmpSwap = (uintptr_t)HkSwapBuffers - (uintptr_t)(targetSwap + 5);
        memcpy(&targetSwap[1], &relJmpSwap, 4);
        VirtualProtect(targetSwap, 5, oldProt, &oldProt);

        Logger::Log("[+] wglSwapBuffers hook installed successfully");

        // 2. Safe Pointer Table Hooking in hw.dll cldll_func_t
        HMODULE hHw = GetModuleHandleA("hw.dll");
        if (hHw) {
            uintptr_t hwBase = (uintptr_t)hHw;
            DWORD hwSize = Engine::g_hwSize ? Engine::g_hwSize : 0x1400000;

            // Resolve V_CalcRefdef slot in hw.dll dispatch table
            uintptr_t refdefSlotVA = FindHwClientSlot(hwBase, hwSize, "V_CalcRefdef");
            if (!refdefSlotVA) {
                refdefSlotVA = hwBase + 0x11fe36c; // Validated RVA fallback
            }

            if (Engine::IsReadableFast((const void*)refdefSlotVA, sizeof(uintptr_t))) {
                g_pCalcRefdefSlot = (uintptr_t*)refdefSlotVA;
                uintptr_t currentVal = *g_pCalcRefdefSlot;

                if (currentVal >= Engine::g_clientBase && currentVal < (Engine::g_clientBase + Engine::g_clientSize)) {
                    g_origCalcRefdef = (V_CalcRefdefFn)currentVal;
                } else if (!g_origCalcRefdef) {
                    uintptr_t exp = (uintptr_t)GetProcAddress((HMODULE)Engine::g_clientBase, "V_CalcRefdef");
                    if (exp) g_origCalcRefdef = (V_CalcRefdefFn)exp;
                    else g_origCalcRefdef = (V_CalcRefdefFn)(Engine::g_clientBase + 0x3E480);
                }

                if (VirtualProtect(g_pCalcRefdefSlot, sizeof(uintptr_t), PAGE_READWRITE, &oldProt)) {
                    *g_pCalcRefdefSlot = (uintptr_t)HkCalcRefdef;
                    VirtualProtect(g_pCalcRefdefSlot, sizeof(uintptr_t), oldProt, &oldProt);
                    Logger::Log("[+] hw.dll::V_CalcRefdef dispatch slot hooked at 0x%08X (Target: 0x%08X)",
                                (unsigned)refdefSlotVA, (unsigned)(uintptr_t)g_origCalcRefdef);
                }
            }

            // Resolve HUD_AddEntity slot in hw.dll dispatch table
            uintptr_t addEntitySlotVA = FindHwClientSlot(hwBase, hwSize, "HUD_AddEntity");
            if (!addEntitySlotVA) {
                addEntitySlotVA = hwBase + 0x11fe370; // Validated RVA fallback
            }

            if (Engine::IsReadableFast((const void*)addEntitySlotVA, sizeof(uintptr_t))) {
                g_pAddEntitySlot = (uintptr_t*)addEntitySlotVA;
                uintptr_t currentVal = *g_pAddEntitySlot;

                if (currentVal >= Engine::g_clientBase && currentVal < (Engine::g_clientBase + Engine::g_clientSize)) {
                    g_origAddEntity = (HUD_AddEntityFn)currentVal;
                } else if (!g_origAddEntity) {
                    uintptr_t exp = (uintptr_t)GetProcAddress((HMODULE)Engine::g_clientBase, "HUD_AddEntity");
                    if (exp) g_origAddEntity = (HUD_AddEntityFn)exp;
                    else g_origAddEntity = (HUD_AddEntityFn)(Engine::g_clientBase + 0x16800);
                }

                if (VirtualProtect(g_pAddEntitySlot, sizeof(uintptr_t), PAGE_READWRITE, &oldProt)) {
                    *g_pAddEntitySlot = (uintptr_t)HkAddEntity;
                    VirtualProtect(g_pAddEntitySlot, sizeof(uintptr_t), oldProt, &oldProt);
                    Engine::g_addEntityHooked = true;
                    Logger::Log("[+] hw.dll::HUD_AddEntity dispatch slot hooked at 0x%08X (Target: 0x%08X)",
                                (unsigned)addEntitySlotVA, (unsigned)(uintptr_t)g_origAddEntity);
                }
            }
        }

        g_hooked = true;
        return true;
    }


    void Remove() {
        DWORD oldProt;
        if (g_pSwapTarget && g_savedSwapBytes[0]) {
            VirtualProtect(g_pSwapTarget, 5, PAGE_EXECUTE_READWRITE, &oldProt);
            memcpy(g_pSwapTarget, g_savedSwapBytes, 5);
            VirtualProtect(g_pSwapTarget, 5, oldProt, &oldProt);
            Logger::Log("[+] wglSwapBuffers hook unhooked");
        }
        if (g_pCalcRefdefSlot && g_origCalcRefdef) {
            VirtualProtect(g_pCalcRefdefSlot, sizeof(uintptr_t), PAGE_READWRITE, &oldProt);
            *g_pCalcRefdefSlot = (uintptr_t)g_origCalcRefdef;
            VirtualProtect(g_pCalcRefdefSlot, sizeof(uintptr_t), oldProt, &oldProt);
            Logger::Log("[+] hw.dll::V_CalcRefdef slot restored");
        }
        if (g_pAddEntitySlot && g_origAddEntity) {
            VirtualProtect(g_pAddEntitySlot, sizeof(uintptr_t), PAGE_READWRITE, &oldProt);
            *g_pAddEntitySlot = (uintptr_t)g_origAddEntity;
            VirtualProtect(g_pAddEntitySlot, sizeof(uintptr_t), oldProt, &oldProt);
            Logger::Log("[+] hw.dll::HUD_AddEntity slot restored");
        }
        g_hooked = false;
    }
}
