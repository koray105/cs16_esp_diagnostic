#pragma once

#include "../sdk/sdk.hpp"

namespace ESP {
    extern PlayerData g_cachedPlayers[32];
    extern int        g_cachedValidCount;
    extern int        g_cachedOnScreenCount;

    void Render(int w, int h, const MenuState& menu);
}
