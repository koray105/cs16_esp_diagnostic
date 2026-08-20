#include "hud.hpp"
#include "widgets.hpp"
#include "../esp.hpp"
#include "../aimbot.hpp"
#include "../../render/renderer.hpp"
#include "../../core/math.hpp"
#include "../../hooks/hooks.hpp"
#include <cstdio>
#include <cstring>

namespace Menu {
    void RenderWatermark(int w, int h, float fps) {
        if (!g_state.watermark) return;

        float fontScale = (h >= 1080) ? 1.05f : 0.92f;
        char wmText[128];
        snprintf(wmText, sizeof(wmText), "V.I.I.B.E PRO SUITE v5.0 | FPS: %.0f", fps);

        size_t len = strlen(wmText);
        float cardW = (float)len * 8.0f * fontScale + 24.0f;
        float cardH = 26.0f;
        float cardX = (float)w - cardW - 15.0f;
        float cardY = 10.0f;

        float ar, ag, ab;
        GetActiveThemeColor(ar, ag, ab);

        Render::DrawFilledRoundedBox(cardX, cardY, cardW, cardH, 4.0f, 0.02f, 0.03f, 0.05f, 0.92f);
        Render::DrawRoundedBox(cardX, cardY, cardW, cardH, 4.0f, ar, ag, ab, 0.85f, 1.2f);
        Render::DrawFilledRoundedBox(cardX + 2.0f, cardY + 2.0f, 3.0f, cardH - 4.0f, 1.5f, ar, ag, ab, 1.0f);

        Render::DrawString(cardX + 12.0f, cardY + 6.0f, 1.0f, 1.0f, 1.0f, wmText, fontScale);
    }

    void RenderKeybinds(int w, int h) {
        if (!g_state.keybindList) return;

        float ar, ag, ab;
        GetActiveThemeColor(ar, ag, ab);
        float fontScale = (h >= 1080) ? 1.05f : 0.92f;

        float kx = (g_state.kbX > 5.0f && g_state.kbX < (float)w - 100.0f) ? g_state.kbX : 15.0f;
        float ky = (g_state.kbY > 5.0f && g_state.kbY < (float)h - 50.0f) ? g_state.kbY : 290.0f;
        float kw = 195.0f;
        float kh = 104.0f;

        Render::DrawFilledRoundedBox(kx, ky, kw, kh, 5.0f, 0.02f, 0.03f, 0.05f, 0.92f);
        Render::DrawRoundedBox(kx, ky, kw, kh, 5.0f, ar, ag, ab, 0.85f, 1.2f);

        Render::DrawFilledRoundedBox(kx + 1.0f, ky + 1.0f, kw - 2.0f, 22.0f, 4.0f, 0.05f, 0.09f, 0.14f, 0.90f);
        Render::DrawString(kx + 10.0f, ky + 5.0f, ar, ag, ab, "ACTIVE KEYBINDS", fontScale * 0.88f);

        char line[64];
        snprintf(line, sizeof(line), "Aimbot:     [%s]", g_state.aimEnable ? (Aimbot::g_isLocked ? "LOCKED" : "ACTIVE") : "OFF");
        Render::DrawString(kx + 10.0f, ky + 28.0f, g_state.aimEnable ? 0.2f : 0.6f, g_state.aimEnable ? 1.0f : 0.6f, 0.3f, line, fontScale * 0.85f);

        snprintf(line, sizeof(line), "Triggerbot: [%s]", g_state.aimTrigger ? "ON (FIRE)" : "OFF");
        Render::DrawString(kx + 10.0f, ky + 46.0f, g_state.aimTrigger ? 1.0f : 0.6f, g_state.aimTrigger ? 0.8f : 0.6f, 0.1f, line, fontScale * 0.85f);

        snprintf(line, sizeof(line), "Bunnyhop:   [%s]", g_state.bhop ? "ON (AUTO)" : "OFF");
        Render::DrawString(kx + 10.0f, ky + 64.0f, g_state.bhop ? 0.2f : 0.6f, g_state.bhop ? 0.9f : 0.6f, 1.0f, line, fontScale * 0.85f);

        snprintf(line, sizeof(line), "Skeleton:   [%s]", g_state.skeletonEsp ? "ENABLED" : "OFF");
        Render::DrawString(kx + 10.0f, ky + 82.0f, g_state.skeletonEsp ? 0.2f : 0.6f, g_state.skeletonEsp ? 1.0f : 0.6f, 0.8f, line, fontScale * 0.85f);
    }

    void RenderHUD(int w, int h, float fps, uint64_t frameCount) {
        if (!g_state.diagHud) return;

        float ar, ag, ab;
        GetActiveThemeColor(ar, ag, ab);

        float dw = (w >= 1920) ? 490.0f : 440.0f;
        float dh = (h >= 1080) ? 310.0f : 260.0f;
        float dx = (float)(w - dw - 15.0f);
        float dy = 45.0f;
        float hudFont = (h >= 1080) ? 1.05f : 0.95f;
        float hudLineStep = (h >= 1080) ? 16.0f : 14.0f;

        Render::DrawFilledRoundedBox(dx, dy, dw, dh, 6.0f, 0.02f, 0.03f, 0.05f, 0.92f);
        Render::DrawRoundedBox(dx, dy, dw, dh, 6.0f, ar, ag, ab, 0.85f, 1.5f);

        Render::DrawString(dx + 12, dy + 8, ar, ag, ab, "[V.I.I.B.E TELEMETRY & DIAGNOSTICS]", hudFont);
        Render::DrawLine(dx + 8, dy + (22.0f * hudFont), dx + dw - 8, dy + (22.0f * hudFont), 0.0f, 0.5f, 0.6f, 0.8f);

        char buf[160];
        snprintf(buf, sizeof(buf), "FPS: %5.1f | Frames: %llu | Res: %dx%d (%s)",
                 fps, (unsigned long long)frameCount, w, h, Math::GetAspectRatioName(w, h));
        Render::DrawString(dx + 12, dy + 28, 0.9f, 0.9f, 0.9f, buf, hudFont);

        snprintf(buf, sizeof(buf), "Aim Status: %s | Locked Target: %s",
                 g_state.aimEnable ? "ACTIVE" : "DISABLED",
                 Aimbot::g_isLocked ? "YES (ACQUIRED)" : "NO (SEARCHING)");
        Render::DrawString(dx + 12, dy + 28 + hudLineStep, Aimbot::g_isLocked ? 0.2f : 0.8f, Aimbot::g_isLocked ? 1.0f : 0.8f, Aimbot::g_isLocked ? 0.2f : 0.8f, buf, hudFont);

        snprintf(buf, sizeof(buf), "Aim Key: %s | Target Bone: %s | VisCheck: %s",
                 GetKeyName(g_state.aimKey), GetBoneName(g_state.aimBone), g_state.aimVisCheck ? "ON" : "OFF");
        Render::DrawString(dx + 12, dy + 28 + hudLineStep * 2.0f, 0.8f, 0.85f, 0.9f, buf, hudFont);

        snprintf(buf, sizeof(buf), "Active Players: %d | Visible On-Screen: %d", ESP::g_cachedValidCount, ESP::g_cachedOnScreenCount);
        Render::DrawString(dx + 12, dy + 28 + hudLineStep * 3.0f, 0.2f, 1.0f, 0.5f, buf, hudFont);

        int maxPlayers = (h >= 1080) ? 10 : 7;
        int lineCount = 0;
        for (int i = 0; i < 32 && lineCount < maxPlayers; i++) {
            const PlayerData& p = ESP::g_cachedPlayers[i];
            if (p.alive || p.name[0]) {
                Vec2 sFeet = {0, 0};
                float dist = 0.0f;
                bool onScreen = Math::WorldToScreen(p.origin, sFeet, w, h, &dist);
                snprintf(buf, sizeof(buf), "#%02d: %-6.6s Dist:%.0fm Scrn(%.0f,%.0f) [%s]",
                         i + 1, p.name[0] ? p.name : (p.modelName[0] ? p.modelName : "Player"),
                         dist / 32.0f, sFeet.x, sFeet.y, onScreen ? "VIS" : "OFF");
                float pr = (p.team == 1) ? 1.0f : ((p.team == 2) ? 0.3f : 0.8f);
                float pg = (p.team == 1) ? 0.3f : ((p.team == 2) ? 0.7f : 0.8f);
                float pb = (p.team == 1) ? 0.3f : ((p.team == 2) ? 1.0f : 0.2f);
                Render::DrawString(dx + 12, dy + 32 + hudLineStep * (4.0f + lineCount), pr, pg, pb, buf, hudFont);
                lineCount++;
            }
        }

        Render::DrawLine(dx + 8, dy + dh - 22, dx + dw - 8, dy + dh - 22, 0.0f, 0.3f, 0.4f, 0.8f);
        Render::DrawString(dx + 12, dy + dh - 16, 0.5f, 0.7f, 0.8f, "F11: Dump Memory Logs | END: Eject", 0.9f);
    }
}
