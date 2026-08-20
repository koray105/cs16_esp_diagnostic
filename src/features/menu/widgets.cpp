#include "widgets.hpp"
#include "../../render/renderer.hpp"

namespace Menu {
    void GetActiveThemeColor(float& r, float& g, float& b) {
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
            case 1: return 11; // Render
            case 2: return 13; // Movement & Radar
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

    void DrawModernToggle(float ctrlX, float rowY, bool enabled, float ar, float ag, float ab, float menuFont) {
        float btnX = ctrlX + 24.0f;
        float btnY = rowY + 3.0f;
        float btnW = 58.0f;
        float btnH = 22.0f;

        if (enabled) {
            Render::DrawFilledRoundedBox(btnX, btnY, btnW, btnH, 8.0f, 0.04f, 0.16f, 0.22f, 0.96f);
            Render::DrawRoundedBox(btnX, btnY, btnW, btnH, 8.0f, ar, ag, ab, 0.95f, 1.5f);
            Render::DrawString(btnX + 8.0f, btnY + 4.0f, 1.0f, 1.0f, 1.0f, "ON", menuFont);
            Render::DrawFilledCircle(btnX + 46.0f, btnY + 11.0f, 7.0f, 14, ar, ag, ab, 1.0f);
            Render::DrawFilledCircle(btnX + 46.0f, btnY + 11.0f, 3.5f, 10, 1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            Render::DrawFilledRoundedBox(btnX, btnY, btnW, btnH, 8.0f, 0.04f, 0.05f, 0.07f, 0.92f);
            Render::DrawRoundedBox(btnX, btnY, btnW, btnH, 8.0f, 0.30f, 0.35f, 0.42f, 0.85f, 1.2f);
            Render::DrawFilledCircle(btnX + 12.0f, btnY + 11.0f, 7.0f, 14, 0.35f, 0.40f, 0.48f, 0.95f);
            Render::DrawFilledCircle(btnX + 12.0f, btnY + 11.0f, 3.5f, 10, 0.15f, 0.18f, 0.22f, 1.0f);
            Render::DrawString(btnX + 24.0f, btnY + 4.0f, 0.70f, 0.75f, 0.82f, "OFF", menuFont);
        }
    }

    void DrawModernCombo(float ctrlX, float rowY, const char* text, float ar, float ag, float ab, float menuFont) {
        float comboX = ctrlX - 15.0f;
        float comboY = rowY + 3.0f;
        float comboW = 98.0f;
        float comboH = 22.0f;

        Render::DrawFilledRoundedBox(comboX, comboY, comboW, comboH, 5.0f, 0.03f, 0.06f, 0.10f, 0.95f);
        Render::DrawRoundedBox(comboX, comboY, comboW, comboH, 5.0f, ar, ag, ab, 0.85f, 1.2f);
        Render::DrawString(comboX + 6.0f, comboY + 4.0f, 1.0f, 1.0f, 1.0f, text, menuFont);
    }

    void DrawModernSlider(float ctrlX, float rowY, float frac, const char* valStr, float ar, float ag, float ab, float menuFont) {
        float sX = ctrlX - 15.0f;
        float sY = rowY + 10.0f;
        float sW = 54.0f;
        float sH = 8.0f;

        Render::DrawFilledRoundedBox(sX, sY, sW, sH, 4.0f, 0.05f, 0.07f, 0.10f, 0.95f);
        Render::DrawRoundedBox(sX, sY, sW, sH, 4.0f, 0.25f, 0.30f, 0.38f, 0.85f, 1.0f);

        if (frac > 0.02f) {
            float fillW = (sW - 2.0f) * frac;
            Render::DrawFilledRoundedBox(sX + 1.0f, sY + 1.0f, fillW, sH - 2.0f, 3.0f, ar, ag, ab, 0.95f);
        }

        float knobX = sX + sW * frac;
        Render::DrawFilledCircle(knobX, sY + 4.0f, 6.5f, 14, 1.0f, 1.0f, 1.0f, 1.0f);
        Render::DrawCircle(knobX, sY + 4.0f, 6.5f, 14, ar, ag, ab, 1.0f, 1.5f);

        float badgeX = ctrlX + 44.0f;
        float badgeY = rowY + 3.0f;
        float badgeW = 38.0f;
        float badgeH = 22.0f;
        Render::DrawFilledRoundedBox(badgeX, badgeY, badgeW, badgeH, 4.0f, 0.03f, 0.05f, 0.08f, 0.95f);
        Render::DrawRoundedBox(badgeX, badgeY, badgeW, badgeH, 4.0f, ar, ag, ab, 0.75f, 1.0f);
        Render::DrawString(badgeX + 4.0f, badgeY + 4.0f, ar, ag, ab, valStr, menuFont);
    }
}
