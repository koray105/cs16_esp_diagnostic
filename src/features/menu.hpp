#pragma once

#include "../sdk/sdk.hpp"

namespace Menu {
    extern MenuState g_state;

    void Init();
    int GetTabItemCount(int tab);
    void Render(int w, int h);
    void RenderHUD(int w, int h, float fps, uint64_t frameCount);
    void RenderWatermark(int w, int h, float fps);
    void RenderKeybinds(int w, int h);
    void HandleInput();
}
