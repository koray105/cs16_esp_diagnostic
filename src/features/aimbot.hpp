#pragma once

#include "../sdk/sdk.hpp"

namespace Aimbot {
    extern int  g_targetIndex;
    extern bool g_isLocked;
    extern Vec3 g_targetPos;
    extern float g_lastFovDelta;

    void Update(const MenuState& state, ref_params_t* pparams = nullptr);
    void Render(int screenW, int screenH, const MenuState& state);
}
