#ifndef INPUT_HPP
#define INPUT_HPP

#include <windows.h>

namespace Input {
    class KeyTracker {
    private:
        bool m_prev;
    public:
        KeyTracker() : m_prev(false) {}
        bool Pressed(int vk);
    };
}

#endif // INPUT_HPP
