#include "tabs.hpp"
#include "widgets.hpp"
#include "../../render/renderer.hpp"
#include <cstdio>

namespace Menu {
    void RenderTabCombat(float px, float contentY, float pw, float rowStep, float ar, float ag, float ab, float menuFont, bool isActiveWindow) {
        const char* labels[] = {
            "Aimbot Master", "Target Hitbox", "Aim Key",
            "Smooth Factor", "FOV Angle", "Target Enemy Only",
            "Visibility Check", "Recoil Control (RCS)", "Triggerbot (Auto-Fire)"
        };

        for (int i = 0; i < 9; i++) {
            float rowY = contentY + (float)i * rowStep;
            bool isSel = isActiveWindow && (i == g_state.selected);

            if (isSel) {
                Render::DrawFilledRoundedBox(px + 4.0f, rowY, pw - 8.0f, 28.0f, 4.0f, 0.06f, 0.12f, 0.18f, 0.75f);
                Render::DrawFilledRoundedBox(px + 4.0f, rowY, 3.0f, 28.0f, 1.5f, ar, ag, ab, 1.0f);
            }

            Render::DrawString(px + 12.0f, rowY + 7.0f, isSel ? 1.0f : 0.88f, isSel ? 1.0f : 0.90f, isSel ? 1.0f : 0.94f, labels[i], menuFont);

            float ctrlX = px + pw - 125.0f;
            switch (i) {
                case 0: DrawModernToggle(ctrlX, rowY, g_state.aimEnable, ar, ag, ab, menuFont); break;
                case 1: DrawModernCombo(ctrlX, rowY, GetBoneName(g_state.aimBone), ar, ag, ab, menuFont); break;
                case 2: DrawModernCombo(ctrlX, rowY, GetKeyName(g_state.aimKey), ar, ag, ab, menuFont); break;
                case 3:
                    {
                        float frac = (g_state.aimSmooth - 1.0f) / 24.0f;
                        char valStr[20];
                        snprintf(valStr, sizeof(valStr), "%.1fx", g_state.aimSmooth);
                        DrawModernSlider(ctrlX, rowY, frac, valStr, ar, ag, ab, menuFont);
                    }
                    break;
                case 4:
                    {
                        float frac = (g_state.aimFov - 1.0f) / 44.0f;
                        char valStr[20];
                        snprintf(valStr, sizeof(valStr), "%.0fd", g_state.aimFov);
                        DrawModernSlider(ctrlX, rowY, frac, valStr, ar, ag, ab, menuFont);
                    }
                    break;
                case 5: DrawModernToggle(ctrlX, rowY, g_state.aimEnemyOnly, ar, ag, ab, menuFont); break;
                case 6: DrawModernToggle(ctrlX, rowY, g_state.aimVisCheck, ar, ag, ab, menuFont); break;
                case 7: DrawModernToggle(ctrlX, rowY, g_state.aimRcs, ar, ag, ab, menuFont); break;
                case 8: DrawModernToggle(ctrlX, rowY, g_state.aimTrigger, ar, ag, ab, menuFont); break;
            }
        }
    }

    void RenderTabRender(float px, float contentY, float pw, float rowStep, float ar, float ag, float ab, float menuFont, bool isActiveWindow) {
        const char* labels[] = {
            "ESP Box", "Health Bar", "Info & Weapon", "Snaplines",
            "Head Marker", "Skeleton ESP", "Offscreen Threat", "Health Value Pill",
            "Box Chams", "Chams Opacity", "C4 & Items"
        };

        for (int i = 0; i < 11; i++) {
            float rowY = contentY + (float)i * rowStep;
            bool isSel = isActiveWindow && (i == g_state.selected);

            if (isSel) {
                Render::DrawFilledRoundedBox(px + 4.0f, rowY, pw - 8.0f, 28.0f, 4.0f, 0.06f, 0.12f, 0.18f, 0.75f);
                Render::DrawFilledRoundedBox(px + 4.0f, rowY, 3.0f, 28.0f, 1.5f, ar, ag, ab, 1.0f);
            }

            Render::DrawString(px + 12.0f, rowY + 7.0f, isSel ? 1.0f : 0.88f, isSel ? 1.0f : 0.90f, isSel ? 1.0f : 0.94f, labels[i], menuFont);

            float ctrlX = px + pw - 125.0f;
            switch (i) {
                case 0: DrawModernCombo(ctrlX, rowY, GetBoxName(g_state.espBox), ar, ag, ab, menuFont); break;
                case 1: DrawModernCombo(ctrlX, rowY, GetHealthName(g_state.espHealth), ar, ag, ab, menuFont); break;
                case 2: DrawModernToggle(ctrlX, rowY, g_state.espInfo, ar, ag, ab, menuFont); break;
                case 3: DrawModernToggle(ctrlX, rowY, g_state.snaplines, ar, ag, ab, menuFont); break;
                case 4: DrawModernToggle(ctrlX, rowY, g_state.headMarker, ar, ag, ab, menuFont); break;
                case 5: DrawModernToggle(ctrlX, rowY, g_state.skeletonEsp, ar, ag, ab, menuFont); break;
                case 6: DrawModernToggle(ctrlX, rowY, g_state.offscreenEsp, ar, ag, ab, menuFont); break;
                case 7: DrawModernToggle(ctrlX, rowY, g_state.hpText, ar, ag, ab, menuFont); break;
                case 8: DrawModernToggle(ctrlX, rowY, g_state.boxChams, ar, ag, ab, menuFont); break;
                case 9:
                    {
                        float frac = (g_state.chamsAlpha - 0.05f) / 0.75f;
                        char valStr[20];
                        snprintf(valStr, sizeof(valStr), "%.0f%%", g_state.chamsAlpha * 100.0f);
                        DrawModernSlider(ctrlX, rowY, frac, valStr, ar, ag, ab, menuFont);
                    }
                    break;
                case 10: DrawModernToggle(ctrlX, rowY, g_state.c4Tracker, ar, ag, ab, menuFont); break;
            }
        }
    }

    void RenderTabMovement(float px, float contentY, float pw, float rowStep, float ar, float ag, float ab, float menuFont, bool isActiveWindow) {
        const char* labels[] = {
            "BunnyHop (Bhop)", "Crosshair Overlay", "Sniper Crosshair", "Recoil Crosshair",
            "FOV Circle", "FOV Radius (px)", "Enemy Only", "2D Radar", "Radar Sweep Anim",
            "Radar Range", "Spectator List", "Keybinds HUD", "Bomb Timer HUD"
        };

        for (int i = 0; i < 13; i++) {
            float rowY = contentY + (float)i * rowStep;
            bool isSel = isActiveWindow && (i == g_state.selected);

            if (isSel) {
                Render::DrawFilledRoundedBox(px + 4.0f, rowY, pw - 8.0f, 28.0f, 4.0f, 0.06f, 0.12f, 0.18f, 0.75f);
                Render::DrawFilledRoundedBox(px + 4.0f, rowY, 3.0f, 28.0f, 1.5f, ar, ag, ab, 1.0f);
            }

            Render::DrawString(px + 12.0f, rowY + 7.0f, isSel ? 1.0f : 0.88f, isSel ? 1.0f : 0.90f, isSel ? 1.0f : 0.94f, labels[i], menuFont);

            float ctrlX = px + pw - 125.0f;
            switch (i) {
                case 0: DrawModernToggle(ctrlX, rowY, g_state.bhop, ar, ag, ab, menuFont); break;
                case 1: DrawModernToggle(ctrlX, rowY, g_state.crosshair, ar, ag, ab, menuFont); break;
                case 2: DrawModernToggle(ctrlX, rowY, g_state.sniperCrosshair, ar, ag, ab, menuFont); break;
                case 3: DrawModernToggle(ctrlX, rowY, g_state.recoilCrosshair, ar, ag, ab, menuFont); break;
                case 4: DrawModernToggle(ctrlX, rowY, g_state.fovCircle, ar, ag, ab, menuFont); break;
                case 5:
                    {
                        float frac = (g_state.fovRadius - 20.0f) / 280.0f;
                        char valStr[20];
                        snprintf(valStr, sizeof(valStr), "%.0fp", g_state.fovRadius);
                        DrawModernSlider(ctrlX, rowY, frac, valStr, ar, ag, ab, menuFont);
                    }
                    break;
                case 6: DrawModernToggle(ctrlX, rowY, g_state.enemyOnly, ar, ag, ab, menuFont); break;
                case 7: DrawModernToggle(ctrlX, rowY, g_state.radar2D, ar, ag, ab, menuFont); break;
                case 8: DrawModernToggle(ctrlX, rowY, g_state.radarSweep, ar, ag, ab, menuFont); break;
                case 9:
                    {
                        float frac = (g_state.radarRange - 500.0f) / 3500.0f;
                        char valStr[20];
                        snprintf(valStr, sizeof(valStr), "%.0fm", g_state.radarRange / 32.0f);
                        DrawModernSlider(ctrlX, rowY, frac, valStr, ar, ag, ab, menuFont);
                    }
                    break;
                case 10: DrawModernToggle(ctrlX, rowY, g_state.spectatorList, ar, ag, ab, menuFont); break;
                case 11: DrawModernToggle(ctrlX, rowY, g_state.keybindList, ar, ag, ab, menuFont); break;
                case 12: DrawModernToggle(ctrlX, rowY, g_state.bombTimer, ar, ag, ab, menuFont); break;
            }
        }
    }

    void RenderTabThemes(float px, float contentY, float pw, float rowStep, float ar, float ag, float ab, float menuFont, bool isActiveWindow) {
        const char* actions[] = {
            "Theme Palette", "[ LOAD: LEGIT MATCH ]", "[ LOAD: SEMI-RAGE ]",
            "[ LOAD: HVH / RAGE ]", "[ LOAD: CLEAN VISUALS ]",
            "[ SAVE CONFIG TO INI ]", "[ LOAD CONFIG FROM INI ]",
            "[ RESET DEFAULTS ]", "[ PANIC / UNLOAD DLL ]"
        };

        for (int i = 0; i < 9; i++) {
            float rowY = contentY + (float)i * rowStep;
            bool isSel = isActiveWindow && (i == g_state.selected);

            if (isSel) {
                Render::DrawFilledRoundedBox(px + 4.0f, rowY, pw - 8.0f, 28.0f, 4.0f, 0.06f, 0.12f, 0.18f, 0.75f);
                Render::DrawFilledRoundedBox(px + 4.0f, rowY, 3.0f, 28.0f, 1.5f, ar, ag, ab, 1.0f);
            }

            float br = (i == 8) ? 1.0f : (i >= 1 && i <= 4 ? 1.0f : 0.90f);
            float bg = (i == 8) ? 0.35f : (i >= 1 && i <= 4 ? 0.95f : 0.92f);
            float bb = (i == 8) ? 0.35f : (i >= 1 && i <= 4 ? 0.95f : 0.96f);

            Render::DrawString(px + 12.0f, rowY + 7.0f, br, bg, bb, actions[i], menuFont);

            if (i == 0) {
                float ctrlX = px + pw - 125.0f;
                DrawModernCombo(ctrlX, rowY, GetThemeName(g_state.themeIndex), ar, ag, ab, menuFont);
            }
        }
    }
}
