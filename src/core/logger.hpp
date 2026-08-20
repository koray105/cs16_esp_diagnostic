#pragma once

#include <windows.h>
#include "../sdk/sdk.hpp"

namespace Logger {
    void Init(HMODULE hDll);
    void Shutdown();
    void Log(const char* fmt, ...);
    void DumpDiagnosticSnapshot(bool force, uint64_t frameCount, float currentFps, int renderW, int renderH,
                                const PlayerData cachedPlayers[32], int validCount, int onScreenCount);
    const char* GetLogPath();
}
