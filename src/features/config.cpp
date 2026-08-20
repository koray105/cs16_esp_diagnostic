#include "config.hpp"
#include "../core/logger.hpp"
#include <cstdio>
#include <cstring>

namespace Config {
    static char s_configPath[MAX_PATH] = "viibe_config.ini";

    void Init(HMODULE hDll) {
        if (hDll) {
            char dllPath[MAX_PATH];
            if (GetModuleFileNameA(hDll, dllPath, MAX_PATH)) {
                char* lastSlash = strrchr(dllPath, '\\');
                if (lastSlash) {
                    *(lastSlash + 1) = '\0';
                    snprintf(s_configPath, sizeof(s_configPath), "%sviibe_config.ini", dllPath);
                    return;
                }
            }
        }
        snprintf(s_configPath, sizeof(s_configPath), "cs16_esp_config.ini");
    }

    const char* GetConfigPath() {
        return s_configPath;
    }

    void ResetDefaults(MenuState& state) {
        state.visible = true;
        state.activeTab = TAB_AIMBOT;

        // Default Multi-Panel Tab Windows
        state.panels[0] = { 25.0f,  45.0f, 300.0f, 360.0f, false, false }; // Combat / Aimbot
        state.panels[1] = { 340.0f, 45.0f, 300.0f, 360.0f, false, false }; // Render / ESP
        state.panels[2] = { 655.0f, 45.0f, 300.0f, 360.0f, false, false }; // Movement & Radar
        state.panels[3] = { 970.0f, 45.0f, 300.0f, 360.0f, false, false }; // Themes & Presets

        state.specX = 15.0f;
        state.specY = 160.0f;
        state.kbX   = 15.0f;
        state.kbY   = 290.0f;
        state.bombX = 300.0f;
        state.bombY = 40.0f;

        // Aimbot defaults
        state.aimEnable    = true;
        state.aimKey       = 5; // Auto
        state.aimBone      = 0; // Head
        state.aimSmooth    = 3.0f;
        state.aimFov       = 25.0f;
        state.aimEnemyOnly = true;
        state.aimVisCheck  = true;
        state.aimRcs       = true;
        state.aimTrigger   = false;

        // Visuals defaults
        state.espBox       = BOX_CORNER;
        state.espHealth    = HP_GRADIENT;
        state.espInfo      = true;
        state.snaplines    = true;
        state.headMarker   = true;
        state.boxChams     = true;
        state.chamsAlpha   = 0.22f;
        state.c4Tracker    = true;
        state.skeletonEsp  = true;
        state.offscreenEsp = true;
        state.hpText       = true;

        // Radar & HUD defaults
        state.radar2D       = true;
        state.radarRange    = 2000.0f;
        state.radarSweep    = true;
        state.watermark     = true;
        state.spectatorList = true;
        state.keybindList   = true;
        state.bombTimer     = true;
        state.diagHud       = true;

        // Misc defaults
        state.bhop            = true;
        state.crosshair       = false;
        state.sniperCrosshair = false;
        state.recoilCrosshair = false;
        state.fovCircle       = true;
        state.enemyOnly       = true;
        state.fovRadius       = 80.0f;

        // Theme defaults
        state.themeIndex   = THEME_CYBER_CYAN;
        state.accentR      = 0.0f;
        state.accentG      = 0.95f;
        state.accentB      = 0.90f;
        state.selected     = 0;
    }

    void ApplyPreset(MenuState& state, int presetIndex) {
        switch (presetIndex) {
            case 0: // Legit Match
                state.aimEnable    = true;
                state.aimKey       = 0; // Mouse1/2
                state.aimBone      = 0; // Head
                state.aimSmooth    = 12.0f; // Very smooth, non-snapping
                state.aimFov       = 4.5f;  // Tight legitimate FOV
                state.aimEnemyOnly = true;
                state.aimVisCheck  = true;
                state.aimRcs       = true;
                state.aimTrigger   = false;

                state.espBox       = BOX_CORNER;
                state.espHealth    = HP_SOLID;
                state.espInfo      = true;
                state.snaplines    = false;
                state.headMarker   = false;
                state.skeletonEsp  = false;
                state.offscreenEsp = false;
                state.hpText       = false;
                state.boxChams     = false;
                state.chamsAlpha   = 0.15f;
                state.c4Tracker    = true;

                state.radar2D       = true;
                state.radarRange    = 1600.0f;
                state.radarSweep    = false;
                state.watermark     = true;
                state.spectatorList = true;
                state.keybindList   = false;
                state.bombTimer     = true;
                state.diagHud       = false;

                state.bhop            = true;
                state.crosshair       = false;
                state.sniperCrosshair = false;
                state.recoilCrosshair = false;
                state.fovCircle       = false;
                state.enemyOnly       = true;
                state.themeIndex      = THEME_CYBER_CYAN;
                break;

            case 1: // Semi-Rage
                state.aimEnable    = true;
                state.aimKey       = 0; // Mouse1/2
                state.aimBone      = 0; // Head
                state.aimSmooth    = 3.5f;  // Responsive snap
                state.aimFov       = 18.0f; // Medium wide FOV
                state.aimEnemyOnly = true;
                state.aimVisCheck  = true;
                state.aimRcs       = true;
                state.aimTrigger   = true;

                state.espBox       = BOX_CORNER;
                state.espHealth    = HP_GRADIENT;
                state.espInfo      = true;
                state.snaplines    = true;
                state.headMarker   = true;
                state.skeletonEsp  = true;
                state.offscreenEsp = true;
                state.hpText       = true;
                state.boxChams     = true;
                state.chamsAlpha   = 0.30f;
                state.c4Tracker    = true;

                state.radar2D       = true;
                state.radarRange    = 2200.0f;
                state.radarSweep    = true;
                state.watermark     = true;
                state.spectatorList = true;
                state.keybindList   = true;
                state.bombTimer     = true;
                state.diagHud       = true;

                state.bhop            = true;
                state.crosshair       = true;
                state.sniperCrosshair = true;
                state.recoilCrosshair = false;
                state.fovCircle       = true;
                state.enemyOnly       = true;
                state.themeIndex      = THEME_NEON_PURPLE;
                break;

            case 2: // HvH Rage
                state.aimEnable    = true;
                state.aimKey       = 5; // Auto (Always active)
                state.aimBone      = 0; // Head
                state.aimSmooth    = 1.0f;  // 1-frame instant lock
                state.aimFov       = 45.0f; // Maximum FOV lock
                state.aimEnemyOnly = true;
                state.aimVisCheck  = false; // Target through penetrable walls
                state.aimRcs       = true;
                state.aimTrigger   = true;

                state.espBox       = BOX_2D;
                state.espHealth    = HP_GRADIENT;
                state.espInfo      = true;
                state.snaplines    = true;
                state.headMarker   = true;
                state.skeletonEsp  = true;
                state.offscreenEsp = true;
                state.hpText       = true;
                state.boxChams     = true;
                state.chamsAlpha   = 0.50f;
                state.c4Tracker    = true;

                state.radar2D       = true;
                state.radarRange    = 3000.0f;
                state.radarSweep    = true;
                state.watermark     = true;
                state.spectatorList = true;
                state.keybindList   = true;
                state.bombTimer     = true;
                state.diagHud       = true;

                state.bhop            = true;
                state.crosshair       = true;
                state.sniperCrosshair = true;
                state.recoilCrosshair = true;
                state.fovCircle       = true;
                state.enemyOnly       = true;
                state.themeIndex      = THEME_CRIMSON_RED;
                break;

            case 3: // Clean Visuals Only
                state.aimEnable    = false;
                state.aimTrigger   = false;

                state.espBox       = BOX_CORNER;
                state.espHealth    = HP_GRADIENT;
                state.espInfo      = true;
                state.snaplines    = false;
                state.headMarker   = false;
                state.skeletonEsp  = true;
                state.offscreenEsp = false;
                state.hpText       = true;
                state.boxChams     = false;
                state.c4Tracker    = true;

                state.radar2D       = true;
                state.radarRange    = 1800.0f;
                state.radarSweep    = true;
                state.watermark     = true;
                state.spectatorList = true;
                state.keybindList   = false;
                state.bombTimer     = true;
                state.diagHud       = false;

                state.bhop            = true;
                state.crosshair       = true;
                state.sniperCrosshair = false;
                state.recoilCrosshair = false;
                state.fovCircle       = false;
                state.enemyOnly       = true;
                state.themeIndex      = THEME_MATRIX_GREEN;
                break;

            default:
                ResetDefaults(state);
                break;
        }
    }

    bool Save(const MenuState& state) {
        FILE* f = fopen(s_configPath, "w");
        if (!f) return false;

        fprintf(f, "[V.I.I.B.E Commercial Settings]\n");
        fprintf(f, "specX=%.1f\n", state.specX);
        fprintf(f, "specY=%.1f\n", state.specY);
        fprintf(f, "kbX=%.1f\n", state.kbX);
        fprintf(f, "kbY=%.1f\n", state.kbY);
        fprintf(f, "bombX=%.1f\n", state.bombX);
        fprintf(f, "bombY=%.1f\n", state.bombY);

        fprintf(f, "aimEnable=%d\n", state.aimEnable ? 1 : 0);
        fprintf(f, "aimKey=%d\n", state.aimKey);
        fprintf(f, "aimBone=%d\n", state.aimBone);
        fprintf(f, "aimSmooth=%.1f\n", state.aimSmooth);
        fprintf(f, "aimFov=%.1f\n", state.aimFov);
        fprintf(f, "aimEnemyOnly=%d\n", state.aimEnemyOnly ? 1 : 0);
        fprintf(f, "aimVisCheck=%d\n", state.aimVisCheck ? 1 : 0);
        fprintf(f, "aimRcs=%d\n", state.aimRcs ? 1 : 0);
        fprintf(f, "aimTrigger=%d\n", state.aimTrigger ? 1 : 0);

        fprintf(f, "espBox=%d\n", state.espBox);
        fprintf(f, "espHealth=%d\n", state.espHealth);
        fprintf(f, "espInfo=%d\n", state.espInfo ? 1 : 0);
        fprintf(f, "snaplines=%d\n", state.snaplines ? 1 : 0);
        fprintf(f, "headMarker=%d\n", state.headMarker ? 1 : 0);
        fprintf(f, "boxChams=%d\n", state.boxChams ? 1 : 0);
        fprintf(f, "chamsAlpha=%.2f\n", state.chamsAlpha);
        fprintf(f, "c4Tracker=%d\n", state.c4Tracker ? 1 : 0);
        fprintf(f, "skeletonEsp=%d\n", state.skeletonEsp ? 1 : 0);
        fprintf(f, "offscreenEsp=%d\n", state.offscreenEsp ? 1 : 0);
        fprintf(f, "hpText=%d\n", state.hpText ? 1 : 0);

        fprintf(f, "radar2D=%d\n", state.radar2D ? 1 : 0);
        fprintf(f, "radarRange=%.1f\n", state.radarRange);
        fprintf(f, "radarSweep=%d\n", state.radarSweep ? 1 : 0);
        fprintf(f, "watermark=%d\n", state.watermark ? 1 : 0);
        fprintf(f, "spectatorList=%d\n", state.spectatorList ? 1 : 0);
        fprintf(f, "keybindList=%d\n", state.keybindList ? 1 : 0);
        fprintf(f, "bombTimer=%d\n", state.bombTimer ? 1 : 0);
        fprintf(f, "diagHud=%d\n", state.diagHud ? 1 : 0);

        fprintf(f, "bhop=%d\n", state.bhop ? 1 : 0);
        fprintf(f, "crosshair=%d\n", state.crosshair ? 1 : 0);
        fprintf(f, "sniperCrosshair=%d\n", state.sniperCrosshair ? 1 : 0);
        fprintf(f, "recoilCrosshair=%d\n", state.recoilCrosshair ? 1 : 0);
        fprintf(f, "fovCircle=%d\n", state.fovCircle ? 1 : 0);
        fprintf(f, "enemyOnly=%d\n", state.enemyOnly ? 1 : 0);
        fprintf(f, "fovRadius=%.1f\n", state.fovRadius);

        for (int p = 0; p < 4; p++) {
            fprintf(f, "panel_%d_x=%.1f\n", p, state.panels[p].x);
            fprintf(f, "panel_%d_y=%.1f\n", p, state.panels[p].y);
            fprintf(f, "panel_%d_pinned=%d\n", p, state.panels[p].pinned ? 1 : 0);
            fprintf(f, "panel_%d_collapsed=%d\n", p, state.panels[p].collapsed ? 1 : 0);
        }

        fprintf(f, "themeIndex=%d\n", state.themeIndex);
        fprintf(f, "accentR=%.3f\n", state.accentR);
        fprintf(f, "accentG=%.3f\n", state.accentG);
        fprintf(f, "accentB=%.3f\n", state.accentB);

        fclose(f);
        Logger::Log("[+] Commercial config saved to %s", s_configPath);
        return true;
    }

    bool Load(MenuState& state) {
        FILE* f = fopen(s_configPath, "r");
        if (!f) return false;

        char line[128];
        while (fgets(line, sizeof(line), f)) {
            int val = 0;
            float fval = 0.0f;
            int pIdx = 0;
            if (sscanf(line, "specX=%f", &fval) == 1) state.specX = fval;
            else if (sscanf(line, "specY=%f", &fval) == 1) state.specY = fval;
            else if (sscanf(line, "kbX=%f", &fval) == 1) state.kbX = fval;
            else if (sscanf(line, "kbY=%f", &fval) == 1) state.kbY = fval;
            else if (sscanf(line, "bombX=%f", &fval) == 1) state.bombX = fval;
            else if (sscanf(line, "bombY=%f", &fval) == 1) state.bombY = fval;
            else if (sscanf(line, "aimEnable=%d", &val) == 1) state.aimEnable = (val != 0);
            else if (sscanf(line, "aimKey=%d", &val) == 1) state.aimKey = val;
            else if (sscanf(line, "aimBone=%d", &val) == 1) state.aimBone = val;
            else if (sscanf(line, "aimSmooth=%f", &fval) == 1) state.aimSmooth = fval;
            else if (sscanf(line, "aimFov=%f", &fval) == 1) state.aimFov = fval;
            else if (sscanf(line, "aimEnemyOnly=%d", &val) == 1) state.aimEnemyOnly = (val != 0);
            else if (sscanf(line, "aimVisCheck=%d", &val) == 1) state.aimVisCheck = (val != 0);
            else if (sscanf(line, "aimRcs=%d", &val) == 1) state.aimRcs = (val != 0);
            else if (sscanf(line, "aimTrigger=%d", &val) == 1) state.aimTrigger = (val != 0);
            else if (sscanf(line, "espBox=%d", &val) == 1) state.espBox = val;
            else if (sscanf(line, "espHealth=%d", &val) == 1) state.espHealth = val;
            else if (sscanf(line, "espInfo=%d", &val) == 1) state.espInfo = (val != 0);
            else if (sscanf(line, "snaplines=%d", &val) == 1) state.snaplines = (val != 0);
            else if (sscanf(line, "headMarker=%d", &val) == 1) state.headMarker = (val != 0);
            else if (sscanf(line, "boxChams=%d", &val) == 1) state.boxChams = (val != 0);
            else if (sscanf(line, "chamsAlpha=%f", &fval) == 1) state.chamsAlpha = fval;
            else if (sscanf(line, "c4Tracker=%d", &val) == 1) state.c4Tracker = (val != 0);
            else if (sscanf(line, "skeletonEsp=%d", &val) == 1) state.skeletonEsp = (val != 0);
            else if (sscanf(line, "offscreenEsp=%d", &val) == 1) state.offscreenEsp = (val != 0);
            else if (sscanf(line, "hpText=%d", &val) == 1) state.hpText = (val != 0);
            else if (sscanf(line, "radar2D=%d", &val) == 1) state.radar2D = (val != 0);
            else if (sscanf(line, "radarRange=%f", &fval) == 1) state.radarRange = fval;
            else if (sscanf(line, "radarSweep=%d", &val) == 1) state.radarSweep = (val != 0);
            else if (sscanf(line, "watermark=%d", &val) == 1) state.watermark = (val != 0);
            else if (sscanf(line, "spectatorList=%d", &val) == 1) state.spectatorList = (val != 0);
            else if (sscanf(line, "keybindList=%d", &val) == 1) state.keybindList = (val != 0);
            else if (sscanf(line, "bombTimer=%d", &val) == 1) state.bombTimer = (val != 0);
            else if (sscanf(line, "diagHud=%d", &val) == 1) state.diagHud = (val != 0);
            else if (sscanf(line, "bhop=%d", &val) == 1) state.bhop = (val != 0);
            else if (sscanf(line, "crosshair=%d", &val) == 1) state.crosshair = (val != 0);
            else if (sscanf(line, "sniperCrosshair=%d", &val) == 1) state.sniperCrosshair = (val != 0);
            else if (sscanf(line, "recoilCrosshair=%d", &val) == 1) state.recoilCrosshair = (val != 0);
            else if (sscanf(line, "fovCircle=%d", &val) == 1) state.fovCircle = (val != 0);
            else if (sscanf(line, "enemyOnly=%d", &val) == 1) state.enemyOnly = (val != 0);
            else if (sscanf(line, "fovRadius=%f", &fval) == 1) state.fovRadius = fval;
            else if (sscanf(line, "panel_%d_x=%f", &pIdx, &fval) == 2 && pIdx >= 0 && pIdx < 4) state.panels[pIdx].x = fval;
            else if (sscanf(line, "panel_%d_y=%f", &pIdx, &fval) == 2 && pIdx >= 0 && pIdx < 4) state.panels[pIdx].y = fval;
            else if (sscanf(line, "panel_%d_pinned=%d", &pIdx, &val) == 2 && pIdx >= 0 && pIdx < 4) state.panels[pIdx].pinned = (val != 0);
            else if (sscanf(line, "panel_%d_collapsed=%d", &pIdx, &val) == 2 && pIdx >= 0 && pIdx < 4) state.panels[pIdx].collapsed = (val != 0);
            else if (sscanf(line, "themeIndex=%d", &val) == 1) state.themeIndex = val;
            else if (sscanf(line, "accentR=%f", &fval) == 1) state.accentR = fval;
            else if (sscanf(line, "accentG=%f", &fval) == 1) state.accentG = fval;
            else if (sscanf(line, "accentB=%f", &fval) == 1) state.accentB = fval;
        }

        fclose(f);
        Logger::Log("[+] Commercial config loaded from %s", s_configPath);
        return true;
    }
}
