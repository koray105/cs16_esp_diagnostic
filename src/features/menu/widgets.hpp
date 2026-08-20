#ifndef MENU_WIDGETS_HPP
#define MENU_WIDGETS_HPP

#include <windows.h>
#include "../../../src/sdk/sdk.hpp"

namespace Menu {
    extern MenuState g_state;

    void GetActiveThemeColor(float& r, float& g, float& b);
    int  GetTabItemCount(int tab);
    const char* GetKeyName(int keyIdx);
    const char* GetBoneName(int boneIdx);
    const char* GetBoxName(int boxIdx);
    const char* GetHealthName(int hpIdx);
    const char* GetThemeName(int themeIdx);

    void DrawModernToggle(float ctrlX, float rowY, bool enabled, float ar, float ag, float ab, float menuFont);
    void DrawModernCombo(float ctrlX, float rowY, const char* text, float ar, float ag, float ab, float menuFont);
    void DrawModernSlider(float ctrlX, float rowY, float frac, const char* valStr, float ar, float ag, float ab, float menuFont);
}

#endif // MENU_WIDGETS_HPP
