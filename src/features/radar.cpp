#include "radar.hpp"
#include "esp.hpp"
#include "../core/math.hpp"
#include "../engine/engine.hpp"
#include "../render/renderer.hpp"
#include <cstdio>
#include <cmath>

namespace Radar {
    void Render(int screenW, int screenH, const MenuState& menu) {
        if (!menu.radar2D) return;
        if (!Math::g_camValid) return;

        float radarRadius = (screenH >= 1080) ? 98.0f : ((screenH >= 720) ? 88.0f : 78.0f);
        float marginX = (screenW >= 1920) ? 32.0f : 22.0f;
        float marginY = (screenH >= 1080) ? 44.0f : 36.0f;
        float centerX = marginX + radarRadius;
        float centerY = marginY + radarRadius;
        float maxRangeUnits = (menu.radarRange > 100.0f) ? menu.radarRange : 2000.0f;
        float fontScale = (screenH >= 1080) ? 1.05f : 0.92f;
        float blipRadius = (screenH >= 1080) ? 4.5f : 3.5f;

        float ar = menu.accentR, ag = menu.accentG, ab = menu.accentB;
        if (ar <= 0.01f && ag <= 0.01f && ab <= 0.01f) {
            ar = 0.0f; ag = 0.92f; ab = 1.0f;
        }

        // 1. Radar Glass Housing Background & Outer Border
        Render::DrawFilledCircle(centerX, centerY, radarRadius + 2.0f, 36, 0.01f, 0.02f, 0.04f, 0.92f);
        Render::DrawCircle(centerX, centerY, radarRadius + 2.0f, 36, 0.0f, 0.0f, 0.0f, 0.95f, 2.5f);
        Render::DrawCircle(centerX, centerY, radarRadius, 36, ar, ag, ab, 0.85f, 1.8f);

        // 2. Concentric Distance Grid Rings
        Render::DrawCircle(centerX, centerY, radarRadius * 0.33f, 28, 0.15f, 0.25f, 0.35f, 0.45f, 1.0f);
        Render::DrawCircle(centerX, centerY, radarRadius * 0.66f, 28, 0.15f, 0.25f, 0.35f, 0.45f, 1.0f);

        // 3. Polar Crosshair Axes
        Render::DrawLine(centerX - radarRadius + 4.0f, centerY, centerX + radarRadius - 4.0f, centerY, 0.15f, 0.25f, 0.35f, 0.45f, 1.0f);
        Render::DrawLine(centerX, centerY - radarRadius + 4.0f, centerX, centerY + radarRadius - 4.0f, 0.15f, 0.25f, 0.35f, 0.45f, 1.0f);

        // 4. Dynamic Rotating Radar Sweep Line
        if (menu.radarSweep) {
            DWORD tick = GetTickCount();
            float sweepAngle = (float)(tick % 2400) / 2400.0f * 6.2831853f;
            Render::DrawRadarSweepLine(centerX, centerY, radarRadius, sweepAngle, ar, ag, ab, 0.65f);
        }

        // 5. Cardinal Direction Labels (N, S, E, W relative to player yaw)
        float yawRad = Math::g_camAngles.y * Math::DEG2RAD;
        float cosYaw = cosf(yawRad);
        float sinYaw = sinf(yawRad);
        float radarScale = radarRadius / maxRangeUnits;

        // North is world +Y (yaw = 90) -> screen direction offset
        float northAngle = -yawRad - 1.5707963f;
        float nX = centerX + cosf(northAngle) * (radarRadius - 10.0f);
        float nY = centerY + sinf(northAngle) * (radarRadius - 10.0f);
        Render::DrawString(nX - 3.0f, nY - 4.0f, 1.0f, 0.2f, 0.2f, "N", fontScale * 0.78f);

        // 6. Header Badge (RADAR | 62m)
        char title[48];
        snprintf(title, sizeof(title), "RADAR | %.0fm", maxRangeUnits * 0.03125f);
        float titleW = (float)strlen(title) * 7.5f * fontScale + 12.0f;
        Render::DrawPillBadge(centerX - titleW * 0.5f, centerY - radarRadius - 18.0f, titleW, 16.0f, 0.02f, 0.03f, 0.05f, 0.88f, ar, ag, ab, 0.8f);
        Render::DrawString(centerX - titleW * 0.5f + 6.0f, centerY - radarRadius - 14.5f, ar, ag, ab, title, fontScale * 0.85f);

        // 7. Local Player Marker (Sharp glowing apex triangle)
        float triH = (screenH >= 1080) ? 8.0f : 6.5f;
        float triW = (screenH >= 1080) ? 6.0f : 5.0f;
        Render::DrawTriangle(centerX, centerY - triH, centerX - triW, centerY + triH * 0.7f, centerX + triW, centerY + triH * 0.7f,
                             0.0f, 1.0f, 0.4f, 1.0f, true);
        Render::DrawCircle(centerX, centerY, 2.5f, 8, 1.0f, 1.0f, 1.0f, 0.95f, 1.0f);

        int localIdx = Engine::GetLocalPlayerIndex();
        bool isSpectator = Math::g_camSpectator;
        int observerTeam = isSpectator ? 0 : Engine::GetLocalPlayerTeam();

        // 8. Player Blips Rendering via precomputed radar matrix
        for (int i = 0; i < 32; i++) {
            const PlayerData& p = ESP::g_cachedPlayers[i];
            if (!p.alive) continue;
            if (!isSpectator && (p.isLocal || (i + 1) == localIdx)) continue;

            bool filterTeammates = menu.enemyOnly || menu.aimEnemyOnly;
            if (filterTeammates && observerTeam != 0) {
                int targetTeam = p.team != 0 ? p.team : Engine::GetPlayerTeam(i + 1);
                if (targetTeam == observerTeam) {
                    continue;
                }
            }

            float dx = p.origin.x - Math::g_camPos.x;
            float dy = p.origin.y - Math::g_camPos.y;
            Vec2 radarPos = {0, 0};
            bool clamped = false;

            if (!Math::WorldToRadarFast(dx, dy, cosYaw, sinYaw, radarScale,
                                       centerX, centerY, radarRadius, radarPos, clamped)) {
                continue;
            }

            float cr, cg, cb;
            if (p.team == 1) { cr = 1.0f; cg = 0.25f; cb = 0.25f; } // Terrorist Red
            else if (p.team == 2) { cr = 0.20f; cg = 0.65f; cb = 1.0f; } // CT Azure
            else { cr = 1.0f; cg = 0.90f; cb = 0.20f; } // Unknown Gold

            // C4 carrier golden aura ring
            if (p.team == 1 && p.hasC4) {
                float c4Ring = (screenH >= 1080) ? 7.5f : 6.0f;
                Render::DrawCircle(radarPos.x, radarPos.y, c4Ring, 16, 1.0f, 0.8f, 0.0f, 1.0f, 1.8f);
            }

            // Blip drop shadow & sharp core
            Render::DrawFilledCircle(radarPos.x, radarPos.y, blipRadius + 1.0f, 14, 0.0f, 0.0f, 0.0f, 0.85f);
            Render::DrawFilledCircle(radarPos.x, radarPos.y, blipRadius, 14, cr, cg, cb, 0.95f);
            Render::DrawCircle(radarPos.x, radarPos.y, blipRadius, 14, 1.0f, 1.0f, 1.0f, 0.8f, 1.0f);

            // Elevation Indicator (▲ for above, ▼ for below)
            float localGroundZ = Math::g_camPos.z - 64.0f;
            float deltaGroundZ = p.feetPos.z - localGroundZ;
            if (deltaGroundZ > 48.0f) {
                Render::DrawString(radarPos.x - 2.5f, radarPos.y - (10.5f * fontScale), 1.0f, 1.0f, 1.0f, "^", 0.9f);
            } else if (deltaGroundZ < -48.0f) {
                Render::DrawString(radarPos.x - 2.5f, radarPos.y + (3.0f * fontScale), 1.0f, 1.0f, 1.0f, "v", 0.9f);
            }
        }

        // 9. C4 Bomb World Item on Radar
        if (menu.c4Tracker) {
            for (int i = 0; i < Engine::g_worldEntityCount; i++) {
                const WorldEntityData& wed = Engine::g_worldEntities[i];
                if (!wed.active || !wed.isC4) continue;

                float dx = wed.origin.x - Math::g_camPos.x;
                float dy = wed.origin.y - Math::g_camPos.y;
                Vec2 radarPos = {0, 0};
                bool clamped = false;

                if (Math::WorldToRadarFast(dx, dy, cosYaw, sinYaw, radarScale,
                                           centerX, centerY, radarRadius, radarPos, clamped)) {
                    float boxHalf = (screenH >= 1080) ? 4.5f : 3.5f;
                    Render::DrawFilledBox(radarPos.x - boxHalf, radarPos.y - boxHalf, boxHalf * 2.0f, boxHalf * 2.0f, 1.0f, 0.35f, 0.0f, 0.95f);
                    Render::DrawBox(radarPos.x - boxHalf, radarPos.y - boxHalf, boxHalf * 2.0f, boxHalf * 2.0f, 1.0f, 1.0f, 0.0f, 1.0f);
                    Render::DrawString(radarPos.x - (8.0f * fontScale), radarPos.y - (14.0f * fontScale), 1.0f, 0.85f, 0.0f, "C4", 0.85f);
                }
            }
        }
    }
}

