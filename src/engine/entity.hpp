#ifndef ENGINE_ENTITY_HPP
#define ENGINE_ENTITY_HPP

#include <windows.h>
#include <cstdint>
#include "../sdk/sdk.hpp"

namespace Engine {
    extern WorldEntityData g_worldEntities[64];
    extern int             g_worldEntityCount;
    extern bool            g_playerHasC4[33];

    void ExtractWeaponName(const char* modelStr, char* outName, size_t outSize);
    void RegisterWorldEntity(int index, void* ent, const char* modelname, uint64_t currentFrame);
    void PruneWorldEntities(uint64_t currentFrame);
    bool StrContainsCaseInsensitive(const char* src, const char* sub);
}

#endif // ENGINE_ENTITY_HPP
