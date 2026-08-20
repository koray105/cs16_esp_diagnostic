#ifndef ENGINE_RESOLVER_HPP
#define ENGINE_RESOLVER_HPP

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

    bool ResolveFunctions();
    bool IsTargetVisible(const Vec3& start, const Vec3& end, int ignoreEntIndex = 0, int targetEntIndex = 0);
}

#endif // ENGINE_RESOLVER_HPP
