#include "aimbot.hpp"
#include "esp.hpp"
#include "../core/math.hpp"
#include "../core/logger.hpp"
#include "../engine/engine.hpp"
#include "../render/renderer.hpp"
#include <cmath>
#include <cstdio>

namespace Aimbot {
    int   g_targetIndex  = -1;
    bool  g_isLocked     = false;
    Vec3  g_targetPos    = {0, 0, 0};
    float g_lastFovDelta = 999.0f;

    static DWORD s_lastTriggerTick = 0;

    static bool IsAimKeyDown(int keyIndex) {
        switch (keyIndex) {
            case 0: return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 || (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0; // Mouse 1 or Mouse 2
            case 1: return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            case 2: return (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;
            case 3: return (GetAsyncKeyState(VK_LMENU) & 0x8000) != 0;
            case 4: return (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
            case 5: return true; // Always active (Auto)
            default: return true;
        }
    }

    static Vec3 CalculateBonePosition(const PlayerData& p, int boneType) {
        Vec3 target = {0, 0, 0};
        if (boneType == 0) { // Head (True Studio Hitbox Center)
            if (p.headPos.IsValid() && !p.headPos.IsZero()) return p.headPos;
            if (Engine::GetPlayerHeadPosition(p, target)) return target;
            return p.origin + Vec3(0, 0, (p.isDucking ? 12.0f : 17.0f));
        } else if (boneType == 1) { // Neck (Upper Spine Hitbox)
            if (p.neckPos.IsValid() && !p.neckPos.IsZero()) return p.neckPos;
            if (p.upperSpinePos.IsValid() && !p.upperSpinePos.IsZero()) return p.upperSpinePos;
            return p.headPos.IsValid() ? p.headPos : (p.origin + Vec3(0, 0, 10.0f));
        } else if (boneType == 2) { // Chest (Chest Hitbox Group 2)
            if (p.chestPos.IsValid() && !p.chestPos.IsZero()) return p.chestPos;
            if (Engine::GetHitboxWorldPosition(p, HITGROUP_CHEST, target)) return target;
            return p.origin + Vec3(0, 0, 4.0f);
        } else { // Pelvis / Stomach (Hitbox Group 3)
            if (p.stomachPos.IsValid() && !p.stomachPos.IsZero()) return p.stomachPos;
            if (p.pelvisPos.IsValid() && !p.pelvisPos.IsZero()) return p.pelvisPos;
            if (Engine::GetHitboxWorldPosition(p, HITGROUP_STOMACH, target)) return target;
            return p.origin + Vec3(0, 0, -4.0f);
        }
    }

    void Update(const MenuState& state, ref_params_t* pparams) {
        g_isLocked = false;
        g_targetIndex = -1;
        g_lastFovDelta = 999.0f;

        if (!state.aimEnable || !Math::g_camValid) return;
        if (Math::g_camSpectator || (pparams && pparams->spectator != 0)) return; // Disable aimbot in spectator mode

        bool keyDown = IsAimKeyDown(state.aimKey);

        // Resolve Local Player Index and Team
        int localIdx = Engine::GetLocalPlayerIndex();
        int localTeam = Engine::GetLocalPlayerTeam();
        int viewEntity = Math::g_camViewEntity;

        int bestIdx = -1;
        float minFovDelta = state.aimFov > 0.5f ? state.aimFov : 25.0f;
        Vec3 bestBonePos = {0, 0, 0};

        // Scan active players from centralized zero-lag cache
        for (int i = 1; i <= 32; i++) {
            if (i == localIdx || (viewEntity > 0 && i == viewEntity)) continue;

            const PlayerData& p = Engine::g_players[i - 1];
            if (!p.alive || p.isLocal) continue;

            // Target Enemy Only: Strict friendly fire prevention
            bool filterTeammates = state.aimEnemyOnly || state.enemyOnly;
            if (filterTeammates && localTeam != 0) {
                int targetTeam = p.team != 0 ? p.team : Engine::GetPlayerTeam(i);
                if (targetTeam == localTeam) continue;
                if (targetTeam == 0 && p.modelName[0] != '\0') {
                    if (Engine::GetTeamFromModelName(p.modelName) == localTeam) continue;
                }
            }

            Vec3 bonePos = CalculateBonePosition(p, state.aimBone);
            Vec3 delta = bonePos - Math::g_camPos;

            // Early Rejection 1: Behind camera plane
            float dotFwd = delta.x * Math::g_camForward.x + delta.y * Math::g_camForward.y + delta.z * Math::g_camForward.z;
            if (dotFwd <= 0.0f) continue;

            float hyp = sqrtf(delta.x * delta.x + delta.y * delta.y);
            if (hyp < 1.0f) continue;

            float reqPitch = -atan2f(delta.z, hyp) * Math::RAD2DEG;
            float reqYaw   =  atan2f(delta.y, delta.x) * Math::RAD2DEG;

            float deltaPitch = reqPitch - Math::g_camAngles.x;
            float deltaYaw   = reqYaw   - Math::g_camAngles.y;

            Math::NormalizeAngles(deltaPitch, deltaYaw);

            float fovDelta = sqrtf(deltaPitch * deltaPitch + deltaYaw * deltaYaw);
            if (fovDelta > minFovDelta) continue;

            // Line-of-Sight Visibility Check (Wall Check)
            if (state.aimVisCheck) {
                if (!Engine::IsTargetVisible(Math::g_camPos, bonePos, localIdx, i)) {
                    continue; // Skip target hidden behind wall/geometry
                }
            }

            minFovDelta = fovDelta;
            bestIdx = i;
            bestBonePos = bonePos;
        }

        if (bestIdx != -1) {
            g_targetIndex = bestIdx;
            g_targetPos = bestBonePos;
            g_lastFovDelta = minFovDelta;
            g_isLocked = true;

            // Triggerbot Auto-Fire execution
            if (state.aimTrigger && minFovDelta <= 3.5f) {
                DWORD now = GetTickCount();
                if (now - s_lastTriggerTick >= 75) {
                    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                    s_lastTriggerTick = now;
                }
            }

            // Aim adjustment execution if key held
            if (keyDown) {
                Vec3 delta = bestBonePos - Math::g_camPos;
                float hyp = sqrtf(delta.x * delta.x + delta.y * delta.y);
                if (hyp >= 1.0f) {
                    float reqPitch = -atan2f(delta.z, hyp) * Math::RAD2DEG;
                    float reqYaw   =  atan2f(delta.y, delta.x) * Math::RAD2DEG;

                    float currentAngles[3] = { Math::g_camAngles.x, Math::g_camAngles.y, Math::g_camAngles.z };
                    if (Engine::g_fnGetViewAngles) {
                        Engine::g_fnGetViewAngles(currentAngles);
                    }

                    float deltaPitch = reqPitch - currentAngles[0];
                    float deltaYaw   = reqYaw   - currentAngles[1];

                    Math::NormalizeAngles(deltaPitch, deltaYaw);

                    float smooth = (state.aimSmooth >= 1.0f) ? state.aimSmooth : 1.0f;
                    float stepPitch = deltaPitch / smooth;
                    float stepYaw   = deltaYaw   / smooth;

                    // Apply Recoil Compensation System (RCS)
                    if (state.aimRcs && pparams && Engine::IsReadableFast(pparams, sizeof(ref_params_t))) {
                        stepPitch -= pparams->punchangle[0] * 0.75f;
                        stepYaw   -= pparams->punchangle[1] * 0.75f;
                    }

                    currentAngles[0] += stepPitch;
                    currentAngles[1] += stepYaw;

                    Math::NormalizeAngles(currentAngles[0], currentAngles[1]);

                    // 1. Direct Engine ViewAngle Writing
                    if (Engine::g_fnSetViewAngles) {
                        Engine::g_fnSetViewAngles(currentAngles);
                    }

                    // 2. Refdef Camera Synchronization
                    if (pparams && Engine::IsReadableFast(pparams, sizeof(ref_params_t))) {
                        pparams->cl_viewangles[0] = currentAngles[0];
                        pparams->cl_viewangles[1] = currentAngles[1];
                        pparams->viewangles[0]    = currentAngles[0];
                        pparams->viewangles[1]    = currentAngles[1];
                    }

                    Math::g_camAngles.x = currentAngles[0];
                    Math::g_camAngles.y = currentAngles[1];
                    Math::AngleVectors(Math::g_camAngles, Math::g_camForward, Math::g_camRight, Math::g_camUp);

                    // 3. Fallback Hardware Mouse Delta Emulation
                    if (!Engine::g_fnSetViewAngles || state.aimSmooth > 1.0f) {
                        const float goldsrcSens = 0.022f;
                        long mDx = (long)(stepYaw / goldsrcSens);
                        long mDy = (long)(stepPitch / goldsrcSens);

                        if (labs(mDx) > 0 || labs(mDy) > 0) {
                            mouse_event(MOUSEEVENTF_MOVE, mDx, mDy, 0, 0);
                        }
                    }
                }
            }
        }
    }



    void Render(int screenW, int screenH, const MenuState& state) {
        if (!state.aimEnable) return;

        float cx = (float)screenW * 0.5f;
        float cy = (float)screenH * 0.5f;
        float fontScale = (screenH >= 1080) ? 1.05f : 0.92f;

        float ar = state.accentR, ag = state.accentG, ab = state.accentB;
        if (ar <= 0.01f && ag <= 0.01f && ab <= 0.01f) {
            ar = 0.0f; ag = 0.92f; ab = 1.0f;
        }

        // 1. Draw Aim FOV Circle on HUD with Soft Halo
        if (state.aimFov > 0.5f && Math::g_camFov > 0.0f) {
            float radPixel = (state.aimFov / Math::g_camFov) * (screenW * 0.5f);
            if (radPixel < 10.0f) radPixel = 10.0f;
            if (radPixel > (float)screenH * 0.45f) radPixel = (float)screenH * 0.45f;

            float r = g_isLocked ? 1.0f : ar;
            float g = g_isLocked ? 0.25f : ag;
            float b = g_isLocked ? 0.25f : ab;

            Render::DrawCircle(cx, cy, radPixel, 48, r, g, b, 0.45f, 1.5f);
            Render::DrawCircle(cx, cy, radPixel + 1.0f, 48, 0.0f, 0.0f, 0.0f, 0.35f, 1.0f);
        }

        // 2. High-Tech Target Lock Visual Marker & Tracer
        if (g_isLocked && g_targetIndex >= 1 && g_targetIndex <= 32) {
            Vec2 screenTarget = {0, 0};
            float dist = 0.0f;
            if (Math::WorldToScreen(g_targetPos, screenTarget, screenW, screenH, &dist)) {
                // Sleek Neon Tracer Line to Locked Target Head
                Render::DrawGlowLine(cx, cy, screenTarget.x, screenTarget.y, 1.0f, 0.2f, 0.2f, 0.65f, 1.4f);

                // Modern Target Lock Diamond Reticle
                float markSize = (screenH >= 1080) ? 14.0f : 11.0f;
                Render::DrawModernCornerBox(screenTarget.x - markSize * 0.5f, screenTarget.y - markSize * 0.5f, markSize, markSize, 1.0f, 0.2f, 0.2f, 0.95f, 1.8f);
                Render::DrawFilledCircle(screenTarget.x, screenTarget.y, 2.5f, 8, 1.0f, 0.9f, 0.0f, 0.95f);

                char lockInfo[48];
                snprintf(lockInfo, sizeof(lockInfo), "LOCKED [%.1f°]", g_lastFovDelta);
                float infoW = (float)strlen(lockInfo) * 7.5f * fontScale + 10.0f;
                Render::DrawPillBadge(screenTarget.x + markSize + 4.0f, screenTarget.y - 8.0f, infoW, 16.0f, 0.03f, 0.04f, 0.06f, 0.88f, 1.0f, 0.2f, 0.2f, 0.85f);
                Render::DrawString(screenTarget.x + markSize + 9.0f, screenTarget.y - 4.5f, 1.0f, 0.4f, 0.4f, lockInfo, fontScale * 0.85f);
            }
        }
    }
}
