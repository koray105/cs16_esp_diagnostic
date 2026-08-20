#pragma once

#include "../sdk/sdk.hpp"

namespace Config {
    void Init(HMODULE hDll);
    bool Save(const MenuState& state);
    bool Load(MenuState& state);
    void ApplyPreset(MenuState& state, int presetIndex);
    void ResetDefaults(MenuState& state);
    const char* GetConfigPath();
}
