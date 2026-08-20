#pragma once

#include <windows.h>
#include "../sdk/sdk.hpp"

namespace Engine {
    extern uintptr_t g_hwBase;
    extern DWORD     g_hwSize;
    extern uintptr_t g_clientBase;
    extern DWORD     g_clientSize;
    extern addr_t    g_pEngfuncsAddr;
    extern uintptr_t* g_pEngfuncsTable;

    extern pfnClientCmd_t        g_fnClientCmd;
    extern pfnGetViewAngles_t   g_fnGetViewAngles;
    extern pfnSetViewAngles_t   g_fnSetViewAngles;
    extern pfnGetPlayerInfo_t    g_fnGetPlayerInfo;
    extern pfnTraceLine_t       g_fnTraceLine;
    extern pfnGetLocalPlayer_t   g_fnGetLocalPlayer;
    extern pfnGetEntityByIndex_t g_fnGetEntityByIndex;
    extern pfnHUD_GetPlayerTeam_t g_fnHUD_GetPlayerTeam;

    extern uint64_t g_lastActiveFrame[33];
    extern Vec3     g_activeOrigins[33];
    extern bool     g_playerHasC4[33];
    extern bool     g_addEntityHooked;

    extern PlayerData g_players[32];
    extern int        g_validPlayerCount;
    extern uint64_t   g_lastCacheFrame;

    extern WorldEntityData g_worldEntities[64];
    extern int             g_worldEntityCount;

    bool ResolveFunctions();
    int  GetTeamFromModelName(const char* modelName);
    int  GetPlayerTeam(int index);
    int  GetLocalPlayerIndex();
    int  GetLocalPlayerTeam();
    bool ReadPlayer(int index, PlayerData& out);
    void UpdateAllPlayers(uint64_t frameCount);
    studiohdr_t* GetStudioHeader(addr_t modelAddr);
    bool GetHitboxData(addr_t studioHdrAddr, int desiredGroup, mstudiohitbox_t& outHitbox, int& outHitboxIndex);
    bool GetHitboxWorldPosition(const PlayerData& p, int hitgroup, Vec3& outPos);
    bool GetPlayerHeadPosition(const PlayerData& p, Vec3& outHeadPos);
    bool IsTargetVisible(const Vec3& start, const Vec3& end, int ignoreEntIndex = 0, int targetEntIndex = 0);
    void RegisterWorldEntity(int index, void* ent, const char* modelname, uint64_t currentFrame);
    void PruneWorldEntities(uint64_t currentFrame);

    bool IsReadableFast(const void* addr, size_t size);
    bool SafeReadBytes(const void* src, void* dst, size_t size);

    template<typename T>
    inline bool SafeRead(addr_t addr, T& out) {
        return SafeReadBytes((const void*)(uintptr_t)addr, &out, sizeof(T));
    }
}

