#include "menu.hpp"
#include "esp.hpp"
#include "aimbot.hpp"
#include "config.hpp"
#include "../engine/engine.hpp"
#include "../render/renderer.hpp"
#include "../core/math.hpp"
#include "../core/logger.hpp"
#include "../hooks/hooks.hpp"
#include <cstdio>
#include <cmath>
#include <cstring>

namespace Menu {
    MenuState g_state = {
        true,          // visible
        0,             // activeTab (0=Combat, 1=Render, 2=Movement, 3=Config)

        // 4 Multi-Panel Draggable Windows (Combat, Render, Movement & Radar, Themes & Presets)
        {
            { 25.0f,  45.0f, 310.0f, 380.0f, false, false }, // Panel 0: Combat / Aimbot
            { 350.0f, 45.0f, 310.0f, 440.0f, false, false }, // Panel 1: Render / ESP
            { 675.0f, 45.0f, 310.0f, 490.0f, false, false }, // Panel 2: Movement & Radar
            { 1000.0f, 45.0f, 310.0f, 380.0f, false, false }  // Panel 3: Themes & Presets
        },

        // Floating Widgets
        15.0f, 160.0f, // specX, specY
        15.0f, 290.0f, // kbX, kbY
        300.0f, 40.0f, // bombX, bombY

        // Tab 0: Aimbot
        true,   // aimEnable
        5,      // aimKey (5 = AUTO / ALWAYS ACTIVE)
        0,      // aimBone (0 = Head)
        3.0f,   // aimSmooth (responsive snap)
        25.0f,  // aimFov (25 degrees wide lock)
        true,   // aimEnemyOnly
        true,   // aimVisCheck (Line-of-Sight Visibility Check)
        true,   // aimRcs
        false,  // aimTrigger

        // Tab 1: Visuals
        BOX_CORNER,  // espBox (2 = Modern Corner Box)
        HP_GRADIENT, // espHealth (2 = Gradient Bar)
        true,        // espInfo
        true,        // snaplines
        true,        // headMarker
        true,        // boxChams
        0.22f,       // chamsAlpha
        true,        // c4Tracker
        true,        // skeletonEsp
        true,        // offscreenEsp
        true,        // hpText

        // Tab 2: Radar & HUD
        true,    // radar2D
        2000.0f, // radarRange
        true,    // radarSweep
        true,    // watermark
        true,    // spectatorList
        true,    // keybindList
        true,    // bombTimer
        true,    // diagHud

        // Tab 3: Misc
        true,   // bhop
        false,  // crosshair
        false,  // sniperCrosshair
        false,  // recoilCrosshair
        true,   // fovCircle
        true,   // enemyOnly
        80.0f,  // fovRadius

        // Tab 4: Themes & Color
        THEME_CYBER_CYAN, // themeIndex
        0.0f, 0.95f, 0.90f, // accentR, accentG, accentB

        0       // selected
    };

    struct KeyTracker {
        bool prev;
        bool Pressed(int vk) {
            bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
            bool hit = down && !prev;
            prev = down;
            return hit;
        }
    };

    static KeyTracker kInsert = {false};
    static KeyTracker kTab    = {false};
    static KeyTracker kUp     = {false};
    static KeyTracker kDown   = {false};
    static KeyTracker kLeft   = {false};
    static KeyTracker kRight  = {false};

    static int   s_dragPanelIndex = -1;
    static float s_dragOffsetX    = 0.0f;
    static float s_dragOffsetY    = 0.0f;

    static int   s_activeSliderPIdx = -1;
    static int   s_activeSliderItem = -1;
    static bool  s_prevMouseLeft    = false;

    static void GetActiveThemeColor(float& r, float& g, float& b) {
        switch (g_state.themeIndex) {
            case THEME_CYBER_CYAN:   r = 0.0f;  g = 0.92f; b = 1.0f;  break;
            case THEME_NEON_PURPLE:  r = 0.68f; g = 0.32f; b = 1.0f;  break;
            case THEME_MATRIX_GREEN: r = 0.15f; g = 1.0f;  b = 0.35f; break;
            case THEME_CRIMSON_RED:  r = 1.0f;  g = 0.22f; b = 0.25f; break;
            case THEME_GOLDEN_AMBER: r = 1.0f;  g = 0.75f; b = 0.10f; break;
            default:                 r = 0.0f;  g = 0.92f; b = 1.0f;  break;
        }
    }

    int GetTabItemCount(int tab) {
        switch (tab) {
            case 0: return 9;  // Combat
            case 1: return 11; // Render (added skeleton, offscreen, hpText)
            case 2: return 13; // Movement & Radar (added sniperCrosshair, recoilCrosshair, radarSweep)
            case 3: return 9;  // Themes & Config
            default: return 0;
        }
    }

    const char* GetKeyName(int keyIdx) {
        switch (keyIdx) {
            case 0: return "MOUSE1/2";
            case 1: return "MOUSE2";
            case 2: return "LSHIFT";
            case 3: return "LALT";
            case 4: return "LCTRL";
            case 5: return "AUTO (ALWAYS)";
            default: return "MOUSE1/2";
        }
    }

    const char* GetBoneName(int boneIdx) {
        switch (boneIdx) {
            case 0: return "HEAD (APEX)";
            case 1: return "NECK";
            case 2: return "CHEST";
            case 3: return "PELVIS / STOMACH";
            default: return "HEAD";
        }
    }

    const char* GetBoxName(int boxIdx) {
        switch (boxIdx) {
            case BOX_OFF:    return "DISABLED";
            case BOX_2D:     return "2D OUTLINE BOX";
            case BOX_CORNER: return "MODERN CORNER";
            default: return "MODERN CORNER";
        }
    }

    const char* GetHealthName(int hpIdx) {
        switch (hpIdx) {
            case HP_OFF:      return "DISABLED";
            case HP_SOLID:    return "SOLID BAR";
            case HP_GRADIENT: return "GRADIENT BAR";
            default: return "GRADIENT BAR";
        }
    }

    const char* GetThemeName(int themeIdx) {
        switch (themeIdx) {
            case THEME_CYBER_CYAN:   return "CYBER CYAN";
            case THEME_NEON_PURPLE:  return "NEON PURPLE";
            case THEME_MATRIX_GREEN: return "MATRIX GREEN";
            case THEME_CRIMSON_RED:  return "CRIMSON RED";
            case THEME_GOLDEN_AMBER: return "GOLDEN AMBER";
            default: return "CYBER CYAN";
        }
    }

    void HandleInput() {
        if (kInsert.Pressed(VK_INSERT)) {
            g_state.visible = !g_state.visible;
            s_dragPanelIndex = -1;
            s_activeSliderPIdx = -1;
            Logger::Log("[+] INSERT pressed — Tabbed Multi-Panel UI visible: %s", g_state.visible ? "ON" : "OFF");
        }

        POINT pt;
        GetCursorPos(&pt);
        HWND hWnd = GetForegroundWindow();
        if (hWnd) {
            ScreenToClient(hWnd, &pt);
        }
        float mx = (float)pt.x;
        float my = (float)pt.y;

        bool mouseLeftDown  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        bool mouseLeftClick = mouseLeftDown && !s_prevMouseLeft;
        s_prevMouseLeft = mouseLeftDown;

        if (!g_state.visible) return;

        // 1. Mouse Dragging Execution for Active Panel
        if (mouseLeftDown && s_dragPanelIndex >= 0 && s_dragPanelIndex < 4) {
            g_state.panels[s_dragPanelIndex].x = mx - s_dragOffsetX;
            g_state.panels[s_dragPanelIndex].y = my - s_dragOffsetY;
            if (g_state.panels[s_dragPanelIndex].x < 0.0f) g_state.panels[s_dragPanelIndex].x = 0.0f;
            if (g_state.panels[s_dragPanelIndex].y < 0.0f) g_state.panels[s_dragPanelIndex].y = 0.0f;
        } else {
            s_dragPanelIndex = -1;
        }

        // 2. Active Slider Dragging
        if (mouseLeftDown && s_activeSliderPIdx >= 0 && s_activeSliderPIdx < 4) {
            PanelState& panel = g_state.panels[s_activeSliderPIdx];
            float trackX = panel.x + panel.w - 140.0f;
            float trackW = 54.0f;
            float frac = (mx - trackX) / trackW;
            if (frac < 0.0f) frac = 0.0f;
            if (frac > 1.0f) frac = 1.0f;

            if (s_activeSliderPIdx == 0) { // Combat
                if (s_activeSliderItem == 3) g_state.aimSmooth = 1.0f + frac * 24.0f;
                else if (s_activeSliderItem == 4) g_state.aimFov = 1.0f + frac * 44.0f;
            } else if (s_activeSliderPIdx == 1) { // Render
                if (s_activeSliderItem == 9) g_state.chamsAlpha = 0.05f + frac * 0.75f;
            } else if (s_activeSliderPIdx == 2) { // Movement & Radar
                if (s_activeSliderItem == 5) g_state.fovRadius = 20.0f + frac * 280.0f;
                else if (s_activeSliderItem == 9) g_state.radarRange = 500.0f + frac * 3500.0f;
            }
        } else {
            s_activeSliderPIdx = -1;
            s_activeSliderItem = -1;
        }

        // 3. Mouse Click & Window Focus Handling
        if (mouseLeftClick) {
            for (int pIdx = 0; pIdx < 4; pIdx++) {
                PanelState& panel = g_state.panels[pIdx];
                float px = panel.x;
                float py = panel.y;
                float pw = panel.w;
                float ph = panel.collapsed ? 34.0f : panel.h;

                // Check Header Bar Click (Dragging & Focus & Pin)
                if (mx >= px && mx <= px + pw && my >= py && my <= py + 34.0f) {
                    g_state.activeTab = pIdx;

                    // Pin Lock Button (px + pw - 72, py + 6, 32, 22)
                    if (mx >= (px + pw - 72.0f) && mx <= (px + pw - 40.0f)) {
                        panel.pinned = !panel.pinned;
                        return;
                    }
                    // Collapse Button (px + pw - 34, py + 6, 26, 22)
                    else if (mx >= (px + pw - 34.0f) && mx <= (px + pw - 8.0f)) {
                        panel.collapsed = !panel.collapsed;
                        return;
                    }
                    // Drag Header
                    else {
                        s_dragPanelIndex = pIdx;
                        s_dragOffsetX = mx - px;
                        s_dragOffsetY = my - py;
                    }
                    return;
                }

                // Check Items Click inside Panel
                if (!panel.collapsed && mx >= px && mx <= px + pw && my >= py + 36.0f && my <= py + ph) {
                    g_state.activeTab = pIdx;
                    float rowStep = 34.0f;
                    int itemIdx = (int)((my - (py + 38.0f)) / rowStep);
                    int maxItems = GetTabItemCount(pIdx);

                    if (itemIdx >= 0 && itemIdx < maxItems) {
                        g_state.selected = itemIdx;

                        if (pIdx == 0) { // Combat
                            switch (itemIdx) {
                                case 0: g_state.aimEnable    = !g_state.aimEnable; break;
                                case 1: g_state.aimBone      = (g_state.aimBone + 1) % 4; break;
                                case 2: g_state.aimKey       = (g_state.aimKey + 1) % 6; break;
                                case 3: s_activeSliderPIdx = 0; s_activeSliderItem = 3; break; // Smooth
                                case 4: s_activeSliderPIdx = 0; s_activeSliderItem = 4; break; // FOV
                                case 5: g_state.aimEnemyOnly = !g_state.aimEnemyOnly; break;
                                case 6: g_state.aimVisCheck  = !g_state.aimVisCheck;  break;
                                case 7: g_state.aimRcs       = !g_state.aimRcs;       break;
                                case 8: g_state.aimTrigger   = !g_state.aimTrigger;   break;
                            }
                        } else if (pIdx == 1) { // Render
                            switch (itemIdx) {
                                case 0: g_state.espBox       = (g_state.espBox + 1) % BOX_COUNT; break;
                                case 1: g_state.espHealth    = (g_state.espHealth + 1) % HP_COUNT; break;
                                case 2: g_state.espInfo      = !g_state.espInfo;      break;
                                case 3: g_state.snaplines    = !g_state.snaplines;    break;
                                case 4: g_state.headMarker   = !g_state.headMarker;   break;
                                case 5: g_state.skeletonEsp  = !g_state.skeletonEsp;  break;
                                case 6: g_state.offscreenEsp = !g_state.offscreenEsp; break;
                                case 7: g_state.hpText       = !g_state.hpText;       break;
                                case 8: g_state.boxChams     = !g_state.boxChams;     break;
                                case 9: s_activeSliderPIdx = 1; s_activeSliderItem = 9; break; // Chams Alpha
                                case 10: g_state.c4Tracker   = !g_state.c4Tracker;    break;
                            }
                        } else if (pIdx == 2) { // Movement & Radar
                            switch (itemIdx) {
                                case 0: g_state.bhop            = !g_state.bhop;            break;
                                case 1: g_state.crosshair       = !g_state.crosshair;       break;
                                case 2: g_state.sniperCrosshair = !g_state.sniperCrosshair; break;
                                case 3: g_state.recoilCrosshair = !g_state.recoilCrosshair; break;
                                case 4: g_state.fovCircle       = !g_state.fovCircle;       break;
                                case 5: s_activeSliderPIdx = 2; s_activeSliderItem = 5; break; // FOV Radius
                                case 6: g_state.enemyOnly       = !g_state.enemyOnly;       break;
                                case 7: g_state.radar2D         = !g_state.radar2D;         break;
                                case 8: g_state.radarSweep      = !g_state.radarSweep;      break;
                                case 9: s_activeSliderPIdx = 2; s_activeSliderItem = 9; break; // Radar Range
                                case 10: g_state.spectatorList  = !g_state.spectatorList;  break;
                                case 11: g_state.keybindList    = !g_state.keybindList;    break;
                                case 12: g_state.bombTimer      = !g_state.bombTimer;      break;
                            }
                        } else if (pIdx == 3) { // Themes & Config
                            switch (itemIdx) {
                                case 0: g_state.themeIndex = (g_state.themeIndex + 1) % THEME_COUNT; break;
                                case 1: Config::ApplyPreset(g_state, 0); break; // Legit Match
                                case 2: Config::ApplyPreset(g_state, 1); break; // Semi-Rage
                                case 3: Config::ApplyPreset(g_state, 2); break; // HvH Rage
                                case 4: Config::ApplyPreset(g_state, 3); break; // Clean Visuals
                                case 5: Config::Save(g_state); break;
                                case 6: Config::Load(g_state); break;
                                case 7: Config::ResetDefaults(g_state); break;
                                case 8: Hooks::Remove(); break; // Panic Eject
                            }
                        }
                    }
                    return;
                }
            }
        }

        // 4. Keyboard Navigation Fallback
        if (kTab.Pressed(VK_TAB)) {
            g_state.activeTab = (g_state.activeTab + 1) % 4;
            g_state.selected = 0;
        }

        int maxItems = GetTabItemCount(g_state.activeTab);
        if (kUp.Pressed(VK_UP)) {
            g_state.selected = (g_state.selected - 1 + maxItems) % maxItems;
        }
        if (kDown.Pressed(VK_DOWN)) {
            g_state.selected = (g_state.selected + 1) % maxItems;
        }
    }

    void RenderWatermark(int w, int h, float fps) {
        if (!g_state.watermark) return;

        float fontScale = (h >= 1080) ? 1.05f : 0.92f;
        char wmText[128];
        snprintf(wmText, sizeof(wmText), "V.I.I.B.E PRO SUITE v5.0 | FPS: %.0f", fps);

        size_t len = strlen(wmText);
        float cardW = (float)len * 8.0f * fontScale + 24.0f;
        float cardH = 26.0f;
        float cardX = (float)w - cardW - 15.0f;
        float cardY = 10.0f;

        float ar, ag, ab;
        GetActiveThemeColor(ar, ag, ab);

        Render::DrawFilledRoundedBox(cardX, cardY, cardW, cardH, 4.0f, 0.02f, 0.03f, 0.05f, 0.92f);
        Render::DrawRoundedBox(cardX, cardY, cardW, cardH, 4.0f, ar, ag, ab, 0.85f, 1.2f);
        Render::DrawFilledRoundedBox(cardX + 2.0f, cardY + 2.0f, 3.0f, cardH - 4.0f, 1.5f, ar, ag, ab, 1.0f);

        Render::DrawString(cardX + 12.0f, cardY + 6.0f, 1.0f, 1.0f, 1.0f, wmText, fontScale);
    }

    void RenderKeybinds(int w, int h) {
        if (!g_state.keybindList) return;

        float ar, ag, ab;
        GetActiveThemeColor(ar, ag, ab);
        float fontScale = (h >= 1080) ? 1.05f : 0.92f;

        float kx = (g_state.kbX > 5.0f && g_state.kbX < (float)w - 100.0f) ? g_state.kbX : 15.0f;
        float ky = (g_state.kbY > 5.0f && g_state.kbY < (float)h - 50.0f) ? g_state.kbY : 290.0f;
        float kw = 195.0f;
        float kh = 104.0f;

        Render::DrawFilledRoundedBox(kx, ky, kw, kh, 5.0f, 0.02f, 0.03f, 0.05f, 0.92f);
        Render::DrawRoundedBox(kx, ky, kw, kh, 5.0f, ar, ag, ab, 0.85f, 1.2f);

        Render::DrawFilledRoundedBox(kx + 1.0f, ky + 1.0f, kw - 2.0f, 22.0f, 4.0f, 0.05f, 0.09f, 0.14f, 0.90f);
        Render::DrawString(kx + 10.0f, ky + 5.0f, ar, ag, ab, "ACTIVE KEYBINDS", fontScale * 0.88f);

        char line[64];
        snprintf(line, sizeof(line), "Aimbot:     [%s]", g_state.aimEnable ? (Aimbot::g_isLocked ? "LOCKED" : "ACTIVE") : "OFF");
        Render::DrawString(kx + 10.0f, ky + 28.0f, g_state.aimEnable ? 0.2f : 0.6f, g_state.aimEnable ? 1.0f : 0.6f, 0.3f, line, fontScale * 0.85f);

        snprintf(line, sizeof(line), "Triggerbot: [%s]", g_state.aimTrigger ? "ON (FIRE)" : "OFF");
        Render::DrawString(kx + 10.0f, ky + 46.0f, g_state.aimTrigger ? 1.0f : 0.6f, g_state.aimTrigger ? 0.8f : 0.6f, 0.1f, line, fontScale * 0.85f);

        snprintf(line, sizeof(line), "Bunnyhop:   [%s]", g_state.bhop ? "ON (AUTO)" : "OFF");
        Render::DrawString(kx + 10.0f, ky + 64.0f, g_state.bhop ? 0.2f : 0.6f, g_state.bhop ? 0.9f : 0.6f, 1.0f, line, fontScale * 0.85f);

        snprintf(line, sizeof(line), "Skeleton:   [%s]", g_state.skeletonEsp ? "ENABLED" : "OFF");
        Render::DrawString(kx + 10.0f, ky + 82.0f, g_state.skeletonEsp ? 0.2f : 0.6f, g_state.skeletonEsp ? 1.0f : 0.6f, 0.8f, line, fontScale * 0.85f);
    }

    static void DrawModernToggle(float ctrlX, float rowY, bool enabled, float ar, float ag, float ab, float menuFont) {
        float btnX = ctrlX + 24.0f;
        float btnY = rowY + 3.0f;
        float btnW = 58.0f;
        float btnH = 22.0f;

        if (enabled) {
            // Active ON Switch (Sleek Cyan/Accent Pill with illuminated knob on the right)
            Render::DrawFilledRoundedBox(btnX, btnY, btnW, btnH, 8.0f, 0.04f, 0.16f, 0.22f, 0.96f);
            Render::DrawRoundedBox(btnX, btnY, btnW, btnH, 8.0f, ar, ag, ab, 0.95f, 1.5f);

            // Bold Pure White "ON" Text
            Render::DrawString(btnX + 8.0f, btnY + 4.0f, 1.0f, 1.0f, 1.0f, "ON", menuFont);

            // Illuminated Toggle Knob on the right
            Render::DrawFilledCircle(btnX + 46.0f, btnY + 11.0f, 7.0f, 14, ar, ag, ab, 1.0f);
            Render::DrawFilledCircle(btnX + 46.0f, btnY + 11.0f, 3.5f, 10, 1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            // Inactive OFF Switch (Dark Sleek Pill with muted knob on the left)
            Render::DrawFilledRoundedBox(btnX, btnY, btnW, btnH, 8.0f, 0.04f, 0.05f, 0.07f, 0.92f);
            Render::DrawRoundedBox(btnX, btnY, btnW, btnH, 8.0f, 0.30f, 0.35f, 0.42f, 0.85f, 1.2f);

            // Muted Knob on the left
            Render::DrawFilledCircle(btnX + 12.0f, btnY + 11.0f, 7.0f, 14, 0.35f, 0.40f, 0.48f, 0.95f);
            Render::DrawFilledCircle(btnX + 12.0f, btnY + 11.0f, 3.5f, 10, 0.15f, 0.18f, 0.22f, 1.0f);

            // Clear Silver "OFF" Text
            Render::DrawString(btnX + 24.0f, btnY + 4.0f, 0.70f, 0.75f, 0.82f, "OFF", menuFont);
        }
    }

    static void DrawModernCombo(float ctrlX, float rowY, const char* text, float ar, float ag, float ab, float menuFont) {
        float comboX = ctrlX - 15.0f;
        float comboY = rowY + 3.0f;
        float comboW = 98.0f;
        float comboH = 22.0f;

        Render::DrawFilledRoundedBox(comboX, comboY, comboW, comboH, 5.0f, 0.03f, 0.06f, 0.10f, 0.95f);
        Render::DrawRoundedBox(comboX, comboY, comboW, comboH, 5.0f, ar, ag, ab, 0.85f, 1.2f);
        Render::DrawString(comboX + 6.0f, comboY + 4.0f, 1.0f, 1.0f, 1.0f, text, menuFont);
    }

    static void DrawModernSlider(float ctrlX, float rowY, float frac, const char* valStr, float ar, float ag, float ab, float menuFont) {
        float sX = ctrlX - 15.0f;
        float sY = rowY + 10.0f;
        float sW = 54.0f;
        float sH = 8.0f;

        // Background track
        Render::DrawFilledRoundedBox(sX, sY, sW, sH, 4.0f, 0.05f, 0.07f, 0.10f, 0.95f);
        Render::DrawRoundedBox(sX, sY, sW, sH, 4.0f, 0.25f, 0.30f, 0.38f, 0.85f, 1.0f);

        // Filled progress
        if (frac > 0.02f) {
            float fillW = (sW - 2.0f) * frac;
            Render::DrawFilledRoundedBox(sX + 1.0f, sY + 1.0f, fillW, sH - 2.0f, 3.0f, ar, ag, ab, 0.95f);
        }

        // Draggable Knob
        float knobX = sX + sW * frac;
        Render::DrawFilledCircle(knobX, sY + 4.0f, 6.5f, 14, 1.0f, 1.0f, 1.0f, 1.0f);
        Render::DrawCircle(knobX, sY + 4.0f, 6.5f, 14, ar, ag, ab, 1.0f, 1.5f);

        // Value Badge Container
        float badgeX = ctrlX + 44.0f;
        float badgeY = rowY + 3.0f;
        float badgeW = 38.0f;
        float badgeH = 22.0f;
        Render::DrawFilledRoundedBox(badgeX, badgeY, badgeW, badgeH, 4.0f, 0.03f, 0.05f, 0.08f, 0.95f);
        Render::DrawRoundedBox(badgeX, badgeY, badgeW, badgeH, 4.0f, ar, ag, ab, 0.75f, 1.0f);
        Render::DrawString(badgeX + 4.0f, badgeY + 4.0f, ar, ag, ab, valStr, menuFont);
    }

    void Render(int w, int h) {
        float ar, ag, ab;
        GetActiveThemeColor(ar, ag, ab);

        // 1. Watermark & Floating Widgets
        RenderWatermark(w, h, Hooks::g_currentFps);
        RenderKeybinds(w, h);

        const char* panelTitles[] = {
            "COMBAT / AIMBOT",
            "RENDER / ESP",
            "MOVEMENT & RADAR",
            "THEMES & PRESETS"
        };

        // Render 4 Multi-Panel Draggable Tab Windows
        for (int pIdx = 0; pIdx < 4; pIdx++) {
            PanelState& panel = g_state.panels[pIdx];

            if (panel.w < 100.0f) panel.w = 315.0f;
            if (pIdx == 1 && panel.h < 430.0f) panel.h = 440.0f;
            else if (pIdx == 2 && panel.h < 480.0f) panel.h = 490.0f;
            else if (panel.h < 360.0f) panel.h = 380.0f;

            if (panel.x <= 0.0f || panel.x > (float)w) panel.x = 25.0f + (float)pIdx * (panel.w + 15.0f);
            if (panel.y <= 0.0f || panel.y > (float)h) panel.y = 45.0f;

            bool isActiveWindow = (g_state.activeTab == pIdx);
            bool shouldRender = g_state.visible || panel.pinned;

            if (!shouldRender) continue;

            float px = panel.x;
            float py = panel.y;
            float pw = panel.w;
            float ph = panel.collapsed ? 34.0f : panel.h;
            float menuFont = (h >= 1080) ? 1.02f : 0.92f;
            float rowStep  = 34.0f;

            // Panel Container Modern Glass Body
            if (isActiveWindow && g_state.visible) {
                Render::DrawFilledRoundedBox(px, py, pw, ph, 7.0f, 0.02f, 0.03f, 0.05f, 0.97f);
                Render::DrawRoundedBox(px, py, pw, ph, 7.0f, ar, ag, ab, 0.95f, 1.8f);

                // Active Header Banner
                Render::DrawFilledRoundedBox(px + 1.0f, py + 1.0f, pw - 2.0f, 32.0f, 6.0f, 0.05f, 0.10f, 0.16f, 0.95f);
                Render::DrawFilledCircle(px + 12.0f, py + 16.0f, 4.0f, 10, ar, ag, ab, 1.0f);
                Render::DrawString(px + 22.0f, py + 9.0f, 1.0f, 1.0f, 1.0f, panelTitles[pIdx], menuFont * 1.05f);
            } else {
                Render::DrawFilledRoundedBox(px, py, pw, ph, 7.0f, 0.02f, 0.03f, 0.04f, panel.pinned ? 0.94f : 0.85f);
                Render::DrawRoundedBox(px, py, pw, ph, 7.0f, panel.pinned ? 1.0f : 0.25f, panel.pinned ? 0.85f : 0.30f, panel.pinned ? 0.1f : 0.38f, 0.85f, 1.2f);

                // Inactive / Pinned Header Banner
                Render::DrawFilledRoundedBox(px + 1.0f, py + 1.0f, pw - 2.0f, 32.0f, 6.0f, 0.05f, 0.07f, 0.10f, 0.90f);
                Render::DrawFilledCircle(px + 12.0f, py + 16.0f, 4.0f, 10, panel.pinned ? 1.0f : 0.5f, panel.pinned ? 0.8f : 0.55f, panel.pinned ? 0.2f : 0.6f, 0.9f);
                Render::DrawString(px + 22.0f, py + 9.0f, panel.pinned ? 1.0f : 0.8f, panel.pinned ? 0.9f : 0.85f, panel.pinned ? 0.3f : 0.9f, panelTitles[pIdx], menuFont);
            }

            // Pin Lock Button (px + pw - 72, py + 6)
            float pinX = px + pw - 72.0f;
            Render::DrawFilledRoundedBox(pinX, py + 6.0f, 32.0f, 22.0f, 4.0f, panel.pinned ? 0.8f : 0.08f, panel.pinned ? 0.5f : 0.10f, panel.pinned ? 0.0f : 0.14f, 0.90f);
            Render::DrawRoundedBox(pinX, py + 6.0f, 32.0f, 22.0f, 4.0f, 1.0f, 0.85f, 0.2f, 0.9f, 1.0f);
            Render::DrawString(pinX + 5.0f, py + 9.0f, 1.0f, 1.0f, 1.0f, panel.pinned ? "PIN" : "FIX", menuFont * 0.80f);

            // Collapse Button (px + pw - 34, py + 6)
            float colX = px + pw - 34.0f;
            Render::DrawFilledRoundedBox(colX, py + 6.0f, 26.0f, 22.0f, 4.0f, 0.10f, 0.13f, 0.17f, 0.90f);
            Render::DrawRoundedBox(colX, py + 6.0f, 26.0f, 22.0f, 4.0f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f);
            Render::DrawString(colX + 9.0f, py + 9.0f, 1.0f, 1.0f, 1.0f, panel.collapsed ? "+" : "-", menuFont);

            Render::DrawLine(px + 4, py + 34.0f, px + pw - 4, py + 34.0f, 0.12f, 0.18f, 0.25f, 0.8f);

            if (panel.collapsed) continue;

            // Render Panel Options Content
            float contentY = py + 40.0f;

            if (pIdx == 0) { // Combat / Aimbot
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
            } else if (pIdx == 1) { // Render / ESP
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
            } else if (pIdx == 2) { // Movement & Radar
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
            } else if (pIdx == 3) { // Themes & Config
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

        // Custom Modern Mouse Pointer Cursor
        POINT pt;
        GetCursorPos(&pt);
        HWND hWnd = GetForegroundWindow();
        if (hWnd) ScreenToClient(hWnd, &pt);
        float mx = (float)pt.x;
        float my = (float)pt.y;

        Render::DrawFilledCircle(mx, my, 4.0f, 12, ar, ag, ab, 0.95f);
        Render::DrawCircle(mx, my, 4.0f, 12, 1.0f, 1.0f, 1.0f, 1.0f, 1.2f);
        Render::DrawLine(mx, my, mx + 12.0f, my + 12.0f, ar, ag, ab, 0.95f, 2.0f);
    }

    void RenderHUD(int w, int h, float fps, uint64_t frameCount) {
        if (!g_state.diagHud) return;

        float ar, ag, ab;
        GetActiveThemeColor(ar, ag, ab);

        float dw = (w >= 1920) ? 490.0f : 440.0f;
        float dh = (h >= 1080) ? 310.0f : 260.0f;
        float dx = (float)(w - dw - 15.0f);
        float dy = 45.0f;
        float hudFont = (h >= 1080) ? 1.05f : 0.95f;
        float hudLineStep = (h >= 1080) ? 16.0f : 14.0f;

        Render::DrawFilledRoundedBox(dx, dy, dw, dh, 6.0f, 0.02f, 0.03f, 0.05f, 0.92f);
        Render::DrawRoundedBox(dx, dy, dw, dh, 6.0f, ar, ag, ab, 0.85f, 1.5f);

        Render::DrawString(dx + 12, dy + 8, ar, ag, ab, "[V.I.I.B.E TELEMETRY & DIAGNOSTICS]", hudFont);
        Render::DrawLine(dx + 8, dy + (22.0f * hudFont), dx + dw - 8, dy + (22.0f * hudFont), 0.0f, 0.5f, 0.6f, 0.8f);

        char buf[160];
        snprintf(buf, sizeof(buf), "FPS: %5.1f | Frames: %llu | Res: %dx%d (%s)",
                 fps, (unsigned long long)frameCount, w, h, Math::GetAspectRatioName(w, h));
        Render::DrawString(dx + 12, dy + 28, 0.9f, 0.9f, 0.9f, buf, hudFont);

        snprintf(buf, sizeof(buf), "Aim Status: %s | Locked Target: %s",
                 g_state.aimEnable ? "ACTIVE" : "DISABLED",
                 Aimbot::g_isLocked ? "YES (ACQUIRED)" : "NO (SEARCHING)");
        Render::DrawString(dx + 12, dy + 28 + hudLineStep, Aimbot::g_isLocked ? 0.2f : 0.8f, Aimbot::g_isLocked ? 1.0f : 0.8f, Aimbot::g_isLocked ? 0.2f : 0.8f, buf, hudFont);

        snprintf(buf, sizeof(buf), "Aim Key: %s | Target Bone: %s | VisCheck: %s",
                 GetKeyName(g_state.aimKey), GetBoneName(g_state.aimBone), g_state.aimVisCheck ? "ON" : "OFF");
        Render::DrawString(dx + 12, dy + 28 + hudLineStep * 2.0f, 0.8f, 0.85f, 0.9f, buf, hudFont);

        snprintf(buf, sizeof(buf), "Active Players: %d | Visible On-Screen: %d", ESP::g_cachedValidCount, ESP::g_cachedOnScreenCount);
        Render::DrawString(dx + 12, dy + 28 + hudLineStep * 3.0f, 0.2f, 1.0f, 0.5f, buf, hudFont);

        int maxPlayers = (h >= 1080) ? 10 : 7;
        int lineCount = 0;
        for (int i = 0; i < 32 && lineCount < maxPlayers; i++) {
            const PlayerData& p = ESP::g_cachedPlayers[i];
            if (p.alive || p.name[0]) {
                Vec2 sFeet = {0, 0};
                float dist = 0.0f;
                bool onScreen = Math::WorldToScreen(p.origin, sFeet, w, h, &dist);
                snprintf(buf, sizeof(buf), "#%02d: %-6.6s Dist:%.0fm Scrn(%.0f,%.0f) [%s]",
                         i + 1, p.name[0] ? p.name : (p.modelName[0] ? p.modelName : "Player"),
                         dist / 32.0f, sFeet.x, sFeet.y, onScreen ? "VIS" : "OFF");
                float pr = (p.team == 1) ? 1.0f : ((p.team == 2) ? 0.3f : 0.8f);
                float pg = (p.team == 1) ? 0.3f : ((p.team == 2) ? 0.7f : 0.8f);
                float pb = (p.team == 1) ? 0.3f : ((p.team == 2) ? 1.0f : 0.2f);
                Render::DrawString(dx + 12, dy + 32 + hudLineStep * (4.0f + lineCount), pr, pg, pb, buf, hudFont);
                lineCount++;
            }
        }

        Render::DrawLine(dx + 8, dy + dh - 22, dx + dw - 8, dy + dh - 22, 0.0f, 0.3f, 0.4f, 0.8f);
        Render::DrawString(dx + 12, dy + dh - 16, 0.5f, 0.7f, 0.8f, "F11: Dump Memory Logs | END: Eject", 0.9f);
    }
}


