#ifndef MENU_TABS_HPP
#define MENU_TABS_HPP

namespace Menu {
    void RenderTabCombat(float px, float contentY, float pw, float rowStep, float ar, float ag, float ab, float menuFont, bool isActiveWindow);
    void RenderTabRender(float px, float contentY, float pw, float rowStep, float ar, float ag, float ab, float menuFont, bool isActiveWindow);
    void RenderTabMovement(float px, float contentY, float pw, float rowStep, float ar, float ag, float ab, float menuFont, bool isActiveWindow);
    void RenderTabThemes(float px, float contentY, float pw, float rowStep, float ar, float ag, float ab, float menuFont, bool isActiveWindow);
}

#endif // MENU_TABS_HPP
