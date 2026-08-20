#ifndef ENGINE_PLAYER_HPP
#define ENGINE_PLAYER_HPP

#include <windows.h>
#include <cstdint>
#include "../sdk/sdk.hpp"

namespace Engine {
    extern uint64_t g_lastActiveFrame[33];
    extern Vec3     g_activeOrigins[33];
    extern bool     g_addEntityHooked;

    extern PlayerData g_players[32];
    extern int        g_validPlayerCount;
    extern uint64_t   g_lastCacheFrame;

    int  GetTeamFromModelName(const char* modelName);
    int  GetPlayerTeam(int index);
    int  GetLocalPlayerIndex();
    int  GetLocalPlayerTeam();
    bool ReadPlayer(int index, PlayerData& out);
    void UpdateAllPlayers(uint64_t frameCount);
    bool GetHitboxWorldPosition(const PlayerData& p, int hitgroup, Vec3& outPos);
    bool GetPlayerHeadPosition(const PlayerData& p, Vec3& outHeadPos);
}

#endif // ENGINE_PLAYER_HPP
