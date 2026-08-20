#pragma once

#include "../sdk/sdk.hpp"

namespace Misc {
    void Render(int screenW, int screenH, const MenuState& menu);
    void Update(const MenuState& menu);
}
