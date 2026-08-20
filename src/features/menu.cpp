#include "menu.hpp"
#include "menu/widgets.hpp"
#include "menu/hud.hpp"
#include "menu/tabs.hpp"
#include "config.hpp"
#include "../engine/engine.hpp"
#include "../render/renderer.hpp"
#include "../core/logger.hpp"
#include "../core/input.hpp"
#include "../hooks/hooks.hpp"

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

    static Input::KeyTracker kInsert;
    static Input::KeyTracker kTab;
    static Input::KeyTracker kUp;
    static Input::KeyTracker kDown;

    static int   s_dragPanelIndex   = -1;
    static float s_dragOffsetX      = 0.0f;
    static float s_dragOffsetY      = 0.0f;

    static int   s_activeSliderPIdx = -1;
    static int   s_activeSliderItem = -1;
    static bool  s_prevMouseLeft    = false;

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

                if (mx >= px && mx <= px + pw && my >= py && my <= py + 34.0f) {
                    g_state.activeTab = pIdx;

                    if (mx >= (px + pw - 72.0f) && mx <= (px + pw - 40.0f)) {
                        panel.pinned = !panel.pinned;
                        return;
                    }
                    else if (mx >= (px + pw - 34.0f) && mx <= (px + pw - 8.0f)) {
                        panel.collapsed = !panel.collapsed;
                        return;
                    }
                    else {
                        s_dragPanelIndex = pIdx;
                        s_dragOffsetX = mx - px;
                        s_dragOffsetY = my - py;
                    }
                    return;
                }

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
                                case 3: s_activeSliderPIdx = 0; s_activeSliderItem = 3; break;
                                case 4: s_activeSliderPIdx = 0; s_activeSliderItem = 4; break;
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
                                case 9: s_activeSliderPIdx = 1; s_activeSliderItem = 9; break;
                                case 10: g_state.c4Tracker   = !g_state.c4Tracker;    break;
                            }
                        } else if (pIdx == 2) { // Movement & Radar
                            switch (itemIdx) {
                                case 0: g_state.bhop            = !g_state.bhop;            break;
                                case 1: g_state.crosshair       = !g_state.crosshair;       break;
                                case 2: g_state.sniperCrosshair = !g_state.sniperCrosshair; break;
                                case 3: g_state.recoilCrosshair = !g_state.recoilCrosshair; break;
                                case 4: g_state.fovCircle       = !g_state.fovCircle;       break;
                                case 5: s_activeSliderPIdx = 2; s_activeSliderItem = 5; break;
                                case 6: g_state.enemyOnly       = !g_state.enemyOnly;       break;
                                case 7: g_state.radar2D         = !g_state.radar2D;         break;
                                case 8: g_state.radarSweep      = !g_state.radarSweep;      break;
                                case 9: s_activeSliderPIdx = 2; s_activeSliderItem = 9; break;
                                case 10: g_state.spectatorList  = !g_state.spectatorList;  break;
                                case 11: g_state.keybindList    = !g_state.keybindList;    break;
                                case 12: g_state.bombTimer      = !g_state.bombTimer;      break;
                            }
                        } else if (pIdx == 3) { // Themes & Config
                            switch (itemIdx) {
                                case 0: g_state.themeIndex = (g_state.themeIndex + 1) % THEME_COUNT; break;
                                case 1: Config::ApplyPreset(g_state, 0); break;
                                case 2: Config::ApplyPreset(g_state, 1); break;
                                case 3: Config::ApplyPreset(g_state, 2); break;
                                case 4: Config::ApplyPreset(g_state, 3); break;
                                case 5: Config::Save(g_state); break;
                                case 6: Config::Load(g_state); break;
                                case 7: Config::ResetDefaults(g_state); break;
                                case 8: Hooks::Remove(); break;
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

    void Render(int w, int h) {
        float ar, ag, ab;
        GetActiveThemeColor(ar, ag, ab);

        RenderWatermark(w, h, Hooks::g_currentFps);
        RenderKeybinds(w, h);

        const char* panelTitles[] = {
            "COMBAT / AIMBOT",
            "RENDER / ESP",
            "MOVEMENT & RADAR",
            "THEMES & PRESETS"
        };

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

            if (isActiveWindow && g_state.visible) {
                Render::DrawFilledRoundedBox(px, py, pw, ph, 7.0f, 0.02f, 0.03f, 0.05f, 0.97f);
                Render::DrawRoundedBox(px, py, pw, ph, 7.0f, ar, ag, ab, 0.95f, 1.8f);

                Render::DrawFilledRoundedBox(px + 1.0f, py + 1.0f, pw - 2.0f, 32.0f, 6.0f, 0.05f, 0.10f, 0.16f, 0.95f);
                Render::DrawFilledCircle(px + 12.0f, py + 16.0f, 4.0f, 10, ar, ag, ab, 1.0f);
                Render::DrawString(px + 22.0f, py + 9.0f, 1.0f, 1.0f, 1.0f, panelTitles[pIdx], menuFont * 1.05f);
            } else {
                Render::DrawFilledRoundedBox(px, py, pw, ph, 7.0f, 0.02f, 0.03f, 0.04f, panel.pinned ? 0.94f : 0.85f);
                Render::DrawRoundedBox(px, py, pw, ph, 7.0f, panel.pinned ? 1.0f : 0.25f, panel.pinned ? 0.85f : 0.30f, panel.pinned ? 0.1f : 0.38f, 0.85f, 1.2f);

                Render::DrawFilledRoundedBox(px + 1.0f, py + 1.0f, pw - 2.0f, 32.0f, 6.0f, 0.05f, 0.07f, 0.10f, 0.90f);
                Render::DrawFilledCircle(px + 12.0f, py + 16.0f, 4.0f, 10, panel.pinned ? 1.0f : 0.5f, panel.pinned ? 0.8f : 0.55f, panel.pinned ? 0.2f : 0.6f, 0.9f);
                Render::DrawString(px + 22.0f, py + 9.0f, panel.pinned ? 1.0f : 0.8f, panel.pinned ? 0.9f : 0.85f, panel.pinned ? 0.3f : 0.9f, panelTitles[pIdx], menuFont);
            }

            float pinX = px + pw - 72.0f;
            Render::DrawFilledRoundedBox(pinX, py + 6.0f, 32.0f, 22.0f, 4.0f, panel.pinned ? 0.8f : 0.08f, panel.pinned ? 0.5f : 0.10f, panel.pinned ? 0.0f : 0.14f, 0.90f);
            Render::DrawRoundedBox(pinX, py + 6.0f, 32.0f, 22.0f, 4.0f, 1.0f, 0.85f, 0.2f, 0.9f, 1.0f);
            Render::DrawString(pinX + 5.0f, py + 9.0f, 1.0f, 1.0f, 1.0f, panel.pinned ? "PIN" : "FIX", menuFont * 0.80f);

            float colX = px + pw - 34.0f;
            Render::DrawFilledRoundedBox(colX, py + 6.0f, 26.0f, 22.0f, 4.0f, 0.10f, 0.13f, 0.17f, 0.90f);
            Render::DrawRoundedBox(colX, py + 6.0f, 26.0f, 22.0f, 4.0f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f);
            Render::DrawString(colX + 9.0f, py + 9.0f, 1.0f, 1.0f, 1.0f, panel.collapsed ? "+" : "-", menuFont);

            Render::DrawLine(px + 4, py + 34.0f, px + pw - 4, py + 34.0f, 0.12f, 0.18f, 0.25f, 0.8f);

            if (panel.collapsed) continue;

            float contentY = py + 40.0f;
            if (pIdx == 0) {
                RenderTabCombat(px, contentY, pw, rowStep, ar, ag, ab, menuFont, isActiveWindow);
            } else if (pIdx == 1) {
                RenderTabRender(px, contentY, pw, rowStep, ar, ag, ab, menuFont, isActiveWindow);
            } else if (pIdx == 2) {
                RenderTabMovement(px, contentY, pw, rowStep, ar, ag, ab, menuFont, isActiveWindow);
            } else if (pIdx == 3) {
                RenderTabThemes(px, contentY, pw, rowStep, ar, ag, ab, menuFont, isActiveWindow);
            }
        }

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
}
