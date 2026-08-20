#include "logger.hpp"
#include "math.hpp"
#include "../engine/engine.hpp"
#include <cstdio>
#include <cstdarg>
#include <cstring>

namespace Logger {
    static HANDLE   g_logFile         = INVALID_HANDLE_VALUE;
    static char     g_logPath[MAX_PATH] = {0};
    static DWORD    g_lastDiagLogTime = 0;

    void Init(HMODULE hDll) {
        GetModuleFileNameA(hDll, g_logPath, MAX_PATH);
        char* lastSlash = strrchr(g_logPath, '\\');
        if (lastSlash) strcpy(lastSlash + 1, "cs16_internal.log");
        else strcpy(g_logPath, "cs16_internal.log");

        g_logFile = CreateFileA(g_logPath, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    void Shutdown() {
        if (g_logFile != INVALID_HANDLE_VALUE) {
            CloseHandle(g_logFile);
            g_logFile = INVALID_HANDLE_VALUE;
        }
    }

    const char* GetLogPath() {
        return g_logPath;
    }

    void Log(const char* fmt, ...) {
        if (g_logFile == INVALID_HANDLE_VALUE) return;
        char buf[2048];
        SYSTEMTIME st;
        GetLocalTime(&st);
        int prefixLen = snprintf(buf, sizeof(buf), "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
                                 st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        va_list ap;
        va_start(ap, fmt);
        int len = vsnprintf(buf + prefixLen, sizeof(buf) - prefixLen - 3, fmt, ap);
        va_end(ap);
        if (len > 0) {
            int total = prefixLen + len;
            buf[total] = '\r'; buf[total + 1] = '\n'; buf[total + 2] = 0;
            DWORD written;
            WriteFile(g_logFile, buf, total + 2, &written, NULL);
        }
    }


    void DumpDiagnosticSnapshot(bool force, uint64_t frameCount, float currentFps, int renderW, int renderH,
                                const PlayerData cachedPlayers[32], int validCount, int onScreenCount) {
        DWORD now = GetTickCount();
        if (!force && (now - g_lastDiagLogTime < 2500)) return;
        g_lastDiagLogTime = now;

        float aspect = (renderH > 0) ? ((float)renderW / (float)renderH) : 1.777778f;

        Log("================================================================================");
        Log("[V.I.I.B.E TELEMETRY SNAPSHOT v3.1] Frame: %llu | RefDefFrames: %llu | FPS: %.1f | Viewport: %dx%d (Aspect: %.6f - %s)",
            (unsigned long long)frameCount, (unsigned long long)Math::g_refdefFrames, currentFps, renderW, renderH, aspect, Math::GetAspectRatioName(renderW, renderH));
        Log("================================================================================");
        Log("[+] Camera: Valid=%s | Pos:(%7.1f, %7.1f, %7.1f) | Angles:(%5.1f, %5.1f, %5.1f) | Fwd:(%5.2f, %5.2f, %5.2f) | FOV:%.1f",
            Math::g_camValid ? "YES" : "NO",
            Math::g_camPos.x, Math::g_camPos.y, Math::g_camPos.z,
            Math::g_camAngles.x, Math::g_camAngles.y, Math::g_camAngles.z,
            Math::g_camForward.x, Math::g_camForward.y, Math::g_camForward.z, Math::g_camFov);
        Log("[+] Active Players: %d | Visible On-Screen: %d", validCount, onScreenCount);

        for (int i = 0; i < 32; i++) {
            const PlayerData& p = cachedPlayers[i];
            if (p.alive || p.name[0]) {
                Vec2 sFeet = {0, 0}, sTop = {0, 0}, sOrigin = {0, 0};
                float zDist = 0.0f;
                const char* w2sFeetReason = nullptr;
                const char* w2sTopReason = nullptr;
                bool feetOnScreen = Math::WorldToScreen(p.feetPos, sFeet, renderW, renderH, &zDist, &w2sFeetReason);
                bool topOnScreen  = Math::WorldToScreen(p.topPos, sTop, renderW, renderH, nullptr, &w2sTopReason);
                Math::WorldToScreen(p.origin, sOrigin, renderW, renderH);

                float topY    = (sTop.y < sFeet.y) ? sTop.y : sFeet.y;
                float bottomY = (sTop.y > sFeet.y) ? sTop.y : sFeet.y;
                float boxH    = bottomY - topY;
                float boxW    = boxH * (p.isDucking ? 0.78f : 0.48f);
                float midX    = sOrigin.x;
                float boxL    = midX - boxW * 0.5f;
                float boxR    = midX + boxW * 0.5f;
                float boxT    = topY;
                float boxB    = bottomY;

                Vec3 espHeadWorld = {0, 0, 0};
                Vec3 aimTargetWorld = {0, 0, 0};
                Engine::GetPlayerHeadPosition(p, espHeadWorld);
                Engine::GetHitboxWorldPosition(p, HITGROUP_HEAD, aimTargetWorld);
                float syncDiff = espHeadWorld.Dist(aimTargetWorld);

                Vec2 sHead = {0, 0};
                bool headOnScreen = Math::WorldToScreen(espHeadWorld, sHead, renderW, renderH);

                Log("  [*] Slot %02d (Ent:0x%08X Mdl:0x%08X Hdr:0x%08X) | '%s' [%s] | Team: %s | Duck: %s",
                    i + 1, (unsigned)p.entAddr, (unsigned)p.modelAddr, (unsigned)p.studioHdrAddr,
                    p.name[0] ? p.name : "N/A", p.modelName[0] ? p.modelName : "N/A",
                    p.team == 1 ? "TERROR" : (p.team == 2 ? "CT" : "UNKNOWN"),
                    p.isDucking ? "YES" : "NO");
                if (p.hasStudioHitbox) {
                    Log("      HitboxHead:[Set:0 Idx:%02d Min:(%4.1f, %4.1f, %4.1f) Max:(%4.1f, %4.1f, %4.1f)]",
                        p.headHitboxIndex,
                        p.headHitboxMin.x, p.headHitboxMin.y, p.headHitboxMin.z,
                        p.headHitboxMax.x, p.headHitboxMax.y, p.headHitboxMax.z);
                }
                Log("      HeadWorld:(%7.1f, %7.1f, %7.1f) | AimbotTarget:(%7.1f, %7.1f, %7.1f) | SyncDiff:%.4f",
                    espHeadWorld.x, espHeadWorld.y, espHeadWorld.z,
                    aimTargetWorld.x, aimTargetWorld.y, aimTargetWorld.z, syncDiff);
                Log("      Origin:(%7.1f, %7.1f, %7.1f) | Feet:(%7.1f, %7.1f, %7.1f) | Top:(%7.1f, %7.1f, %7.1f)",
                    p.origin.x, p.origin.y, p.origin.z,
                    p.feetPos.x, p.feetPos.y, p.feetPos.z,
                    p.topPos.x, p.topPos.y, p.topPos.z);
                if (feetOnScreen && topOnScreen) {
                    Log("      ScreenFeet:(%5.1f, %5.1f) | ScreenTop:(%5.1f, %5.1f) | ScreenHead:(%5.1f, %5.1f)",
                        sFeet.x, sFeet.y, sTop.x, sTop.y, sHead.x, sHead.y);
                    Log("      Box:[L:%5.1f R:%5.1f T:%5.1f B:%5.1f] (W:%.1f H:%.1f) | W/Dist:%.1f | Status: ON-SCREEN",
                        boxL, boxR, boxT, boxB, boxW, boxH, zDist);
                } else {
                    Log("      W2S Failed | FeetReason: '%s' | TopReason: '%s' | W/Dist:%.1f | Status: OFF-SCREEN/BEHIND-CAM",
                        w2sFeetReason ? w2sFeetReason : "Unknown", w2sTopReason ? w2sTopReason : "Unknown", zDist);
                }
            }
        }
        Log("================================================================================");
    }
}
