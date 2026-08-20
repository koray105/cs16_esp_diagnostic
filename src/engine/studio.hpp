#ifndef ENGINE_STUDIO_HPP
#define ENGINE_STUDIO_HPP

#include <windows.h>
#include "../sdk/sdk.hpp"

namespace Engine {
    studiohdr_t* GetStudioHeader(addr_t modelAddr);
    bool GetHitboxData(addr_t studioHdrAddr, int desiredGroup, mstudiohitbox_t& outHitbox, int& outHitboxIndex);
}

#endif // ENGINE_STUDIO_HPP
