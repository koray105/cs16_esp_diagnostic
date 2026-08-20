#pragma once

#include <windows.h>
#include <cstdint>

namespace Hooks {
    extern uint64_t g_frameCount;
    extern float    g_currentFps;
    extern bool     g_hooked;

    bool Install();
    void Remove();
}
