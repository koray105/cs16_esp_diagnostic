#include "misc.hpp"
#include "../core/math.hpp"
#include "../engine/engine.hpp"
#include "../render/renderer.hpp"
#include <cstdio>
#include <cmath>

namespace Misc {
    static bool  s_jumpHeld     = false;
    static DWORD s_lastJumpTick = 0;

    static DWORD s_c4PlantTick  = 0;
    static bool  s_c4WasPlanted = false;

    void Update(const MenuState& menu) {
        // High-Precision Engine-Synchronized BunnyHop
        if (menu.bhop) {
            bool spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;

            if (spaceDown) {
                s_jumpHeld = true;
                if (Engine::g_fnClientCmd) {
                    if (Math::g_localOnGround) {
                        Engine::g_fnClientCmd("+jump\n");
                    } else {
                        Engine::g_fnClientCmd("-jump\n");
                    }
                } else {
                    DWORD now = GetTickCount();
                    if (now - s_lastJumpTick >= 15) {
                        if (Math::g_localOnGround) {
                            keybd_event(VK_SPACE, 0x39, 0, 0);
                            keybd_event(VK_SPACE, 0x39, KEYEVENTF_KEYUP, 0);
                        }
                        s_lastJumpTick = now;
                    }
                }
            } else if (s_jumpHeld) {
                if (Engine::g_fnClientCmd) {
                    Engine::g_fnClientCmd("-jump\n");
                }
                s_jumpHeld = false;
            }
        }
    }

    void Render(int screenW, int screenH, const MenuState& menu) {
        float fontScale = (screenH >= 1080) ? 1.05f : 0.92f;
        float offMargin = (float)screenW * 0.25f;
        float cx = (float)screenW * 0.5f;
        float cy = (float)screenH * 0.5f;

        float ar = menu.accentR, ag = menu.accentG, ab = menu.accentB;
        if (ar <= 0.01f && ag <= 0.01f && ab <= 0.01f) {
            ar = 0.0f; ag = 0.92f; ab = 1.0f;
        }

        // 1. Recoil Follow Crosshair (Showing live bullet spread impact)
        if (menu.recoilCrosshair && Math::g_camValid) {
            float punchY = 0.0f, punchX = 0.0f;
            if (Engine::g_clientBase) {
                // Client-side punchangle or refdef punch
                float punchAngle[3] = {0,0,0};
                if (Engine::SafeRead(Engine::g_clientBase + 0x11D510, punchAngle)) {
                    punchY = punchAngle[0];
                    punchX = punchAngle[1];
                }
            }
            float pxOffset = (punchX / 90.0f) * (screenW * 0.5f);
            float pyOffset = (punchY / 90.0f) * (screenH * 0.5f);

            float rX = cx - pxOffset;
            float rY = cy + pyOffset;

            Render::DrawCircle(rX, rY, 4.0f, 12, 1.0f, 0.2f, 0.2f, 0.95f, 1.5f);
            Render::DrawFilledCircle(rX, rY, 2.0f, 8, 1.0f, 1.0f, 1.0f, 0.95f);
        }

        // 2. FOV Circle with Soft Glow
        if (menu.fovCircle) {
            float fovRad = (menu.fovRadius > 5.0f) ? menu.fovRadius : 80.0f;
            Render::DrawCircle(cx, cy, fovRad, 48, ar, ag, ab, 0.40f, 1.5f);
            Render::DrawCircle(cx, cy, fovRad + 1.0f, 48, 0.0f, 0.0f, 0.0f, 0.35f, 1.0f);
        }

        // 3. C4 Bomb and World Item Tracker
        bool  c4Found       = false;
        bool  isPlantedNow  = false;
        float closestC4Dist = 9999.0f;

        if (menu.c4Tracker || menu.bombTimer) {
            for (int i = 0; i < Engine::g_worldEntityCount; i++) {
                const WorldEntityData& wed = Engine::g_worldEntities[i];
                if (!wed.active) continue;

                if (wed.isC4) {
                    c4Found = true;
                    if (wed.isPlantedC4) isPlantedNow = true;
                    if (wed.distanceMeters < closestC4Dist) {
                        closestC4Dist = wed.distanceMeters;
                    }
                }

                if (!menu.c4Tracker) continue;

                Vec2 screenPos = {0, 0};
                float zDist = 0.0f;
                if (!Math::WorldToScreen(wed.origin, screenPos, screenW, screenH, &zDist)) continue;

                if (screenPos.x < -offMargin || screenPos.x > (float)screenW + offMargin ||
                    screenPos.y < -offMargin || screenPos.y > (float)screenH + offMargin) continue;

                if (wed.isC4) {
                    float c4Size = (screenH >= 1080) ? 28.0f : 24.0f;
                    float c4Half = c4Size * 0.5f;

                    // Modern C4 Icon Box
                    Render::DrawFilledRoundedBox(screenPos.x - c4Half, screenPos.y - c4Half, c4Size, c4Size, 4.0f, 0.15f, 0.08f, 0.0f, 0.90f);
                    Render::DrawRoundedBox(screenPos.x - c4Half, screenPos.y - c4Half, c4Size, c4Size, 4.0f, 1.0f, 0.8f, 0.0f, 0.95f, 1.6f);
                    Render::DrawString(screenPos.x - 7.0f, screenPos.y - 4.0f, 1.0f, 0.9f, 0.1f, "C4", 0.95f);

                    char label[64];
                    snprintf(label, sizeof(label), "%s | %.0fm", wed.isPlantedC4 ? "PLANTED C4" : "DROPPED C4", wed.distanceMeters);
                    float lblW = (float)strlen(label) * 7.5f * fontScale + 12.0f;
                    Render::DrawPillBadge(screenPos.x - lblW * 0.5f, screenPos.y + c4Half + 4.0f, lblW, 16.0f, 0.03f, 0.04f, 0.06f, 0.88f, 1.0f, 0.8f, 0.0f, 0.85f);
                    Render::DrawString(screenPos.x - lblW * 0.5f + 6.0f, screenPos.y + c4Half + 7.5f, 1.0f, 0.85f, 0.1f, label, fontScale * 0.85f);
                } else if (wed.isGrenade) {
                    float grnRadius = (screenH >= 1080) ? 8.0f : 6.5f;
                    Render::DrawFilledCircle(screenPos.x, screenPos.y, grnRadius, 14, 1.0f, 0.4f, 0.0f, 0.85f);
                    Render::DrawCircle(screenPos.x, screenPos.y, grnRadius, 14, 1.0f, 1.0f, 1.0f, 0.95f, 1.6f);

                    char label[64];
                    snprintf(label, sizeof(label), "%s | %.0fm", wed.displayName, wed.distanceMeters);
                    float lblW = (float)strlen(label) * 7.5f * fontScale + 10.0f;
                    Render::DrawPillBadge(screenPos.x - lblW * 0.5f, screenPos.y + grnRadius + 4.0f, lblW, 15.0f, 0.03f, 0.04f, 0.06f, 0.85f, 1.0f, 0.5f, 0.0f, 0.8f);
                    Render::DrawString(screenPos.x - lblW * 0.5f + 5.0f, screenPos.y + grnRadius + 7.0f, 1.0f, 0.85f, 0.3f, label, fontScale * 0.82f);
                } else if (wed.displayName[0]) {
                    char label[64];
                    snprintf(label, sizeof(label), "%s | %.0fm", wed.displayName, wed.distanceMeters);
                    float lblW = (float)strlen(label) * 7.5f * fontScale + 10.0f;
                    Render::DrawPillBadge(screenPos.x - lblW * 0.5f, screenPos.y - 8.0f, lblW, 16.0f, 0.03f, 0.04f, 0.06f, 0.85f, 0.3f, 0.6f, 1.0f, 0.8f);
                    Render::DrawString(screenPos.x - lblW * 0.5f + 5.0f, screenPos.y - 4.5f, 0.8f, 0.9f, 1.0f, label, fontScale * 0.85f);
                }
            }
        }

        // C4 Plant Timer State
        DWORD now = GetTickCount();
        if (isPlantedNow) {
            if (!s_c4WasPlanted) {
                s_c4PlantTick = now;
                s_c4WasPlanted = true;
            }
        } else {
            s_c4WasPlanted = false;
        }

        // 4. Modern Live C4 Bomb Timer Overlay HUD
        if (menu.bombTimer && isPlantedNow) {
            float bombTimeMax = 35.0f;
            float elapsedSec = (float)(now - s_c4PlantTick) / 1000.0f;
            float remainSec = bombTimeMax - elapsedSec;
            if (remainSec < 0.0f) remainSec = 0.0f;
            float frac = remainSec / bombTimeMax;

            float bw = 250.0f, bh = 54.0f;
            float bx = (menu.bombX > 10.0f && menu.bombX < (float)screenW - 100.0f) ? menu.bombX : ((float)screenW - bw) * 0.5f;
            float by = (menu.bombY > 10.0f && menu.bombY < (float)screenH - 50.0f) ? menu.bombY : 45.0f;

            float rFrac = (remainSec <= 10.0f) ? 1.0f : (1.0f - frac);
            float gFrac = (remainSec <= 10.0f) ? 0.2f : frac;

            // Glass Container Card
            Render::DrawFilledRoundedBox(bx, by, bw, bh, 6.0f, 0.02f, 0.03f, 0.05f, 0.94f);
            Render::DrawRoundedBox(bx, by, bw, bh, 6.0f, rFrac, gFrac, 0.1f, 0.95f, 1.6f);

            // Title
            char bombTitle[64];
            snprintf(bombTitle, sizeof(bombTitle), "C4 BOMB: %.1fs REMAINING", remainSec);
            Render::DrawString(bx + 14.0f, by + 8.0f, rFrac, gFrac, 0.1f, bombTitle, fontScale * 0.92f);

            // Progress Bar
            Render::DrawProgressBar(bx + 14.0f, by + 28.0f, bw - 28.0f, 14.0f, frac, rFrac, gFrac, 0.2f, 0.95f);
        }

        // 5. Modern Floating Spectator List HUD
        if (menu.spectatorList) {
            char specNames[16][32];
            int specCount = 0;

            if (Engine::g_fnGetPlayerInfo) {
                for (int i = 1; i <= 32 && specCount < 16; i++) {
                    hud_player_info_t info = {0};
                    if (Engine::g_fnGetPlayerInfo(i, &info) != 0 && info.name && info.name[0] != 0) {
                        if (info.thisplayer == 0 && info.spectator != 0) {
                            strncpy(specNames[specCount], info.name, 31);
                            specNames[specCount][31] = 0;
                            specCount++;
                        }
                    }
                }
            }

            float sw = 195.0f;
            float sh = 32.0f + (specCount > 0 ? (float)specCount * 20.0f : 22.0f);
            float sx = (menu.specX > 5.0f && menu.specX < (float)screenW - 100.0f) ? menu.specX : 15.0f;
            float sy = (menu.specY > 5.0f && menu.specY < (float)screenH - 50.0f) ? menu.specY : 180.0f;

            // Glass Container Card
            Render::DrawFilledRoundedBox(sx, sy, sw, sh, 5.0f, 0.02f, 0.03f, 0.05f, 0.92f);
            Render::DrawRoundedBox(sx, sy, sw, sh, 5.0f, ar, ag, ab, 0.85f, 1.2f);

            // Header Banner
            Render::DrawFilledRoundedBox(sx + 1.0f, sy + 1.0f, sw - 2.0f, 22.0f, 4.0f, 0.05f, 0.09f, 0.14f, 0.90f);
            char specHeader[48];
            snprintf(specHeader, sizeof(specHeader), "SPECTATORS (%d)", specCount);
            Render::DrawString(sx + 10.0f, sy + 5.0f, ar, ag, ab, specHeader, fontScale * 0.88f);

            if (specCount == 0) {
                Render::DrawString(sx + 12.0f, sy + 30.0f, 0.5f, 0.6f, 0.7f, "(None observing)", fontScale * 0.82f);
            } else {
                for (int s = 0; s < specCount; s++) {
                    Render::DrawFilledCircle(sx + 14.0f, sy + 32.0f + (float)s * 20.0f, 3.0f, 8, 1.0f, 0.8f, 0.2f, 0.95f);
                    Render::DrawString(sx + 24.0f, sy + 28.0f + (float)s * 20.0f, 1.0f, 0.9f, 0.8f, specNames[s], fontScale * 0.85f);
                }
            }
        }
    }
}
