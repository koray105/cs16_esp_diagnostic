#include "input.hpp"

namespace Input {
    bool KeyTracker::Pressed(int vk) {
        bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
        bool hit = down && !m_prev;
        m_prev = down;
        return hit;
    }
}
