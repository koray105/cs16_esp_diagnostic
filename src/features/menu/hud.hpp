#ifndef MENU_HUD_HPP
#define MENU_HUD_HPP

#include <cstdint>

namespace Menu {
    void RenderWatermark(int w, int h, float fps);
    void RenderKeybinds(int w, int h);
    void RenderHUD(int w, int h, float fps, uint64_t frameCount);
}

#endif // MENU_HUD_HPP
