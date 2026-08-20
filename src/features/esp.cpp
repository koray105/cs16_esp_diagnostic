#include "esp.hpp"
#include "../core/math.hpp"
#include "../engine/engine.hpp"
#include "../render/renderer.hpp"
#include "../hooks/hooks.hpp"
#include <cstdio>
#include <cmath>


namespace ESP {
    PlayerData g_cachedPlayers[32];
    int        g_cachedValidCount = 0;
    int        g_cachedOnScreenCount = 0;

    static inline void DrawBoneSegment(const Vec2& p1, const Vec2& p2, float r, float g, float b, float alpha, float lineWidth) {
        // High-contrast occlusion drop-shadow line
        Render::DrawLine(p1.x, p1.y, p2.x, p2.y, 0.0f, 0.0f, 0.0f, alpha * 0.95f, lineWidth + 1.8f);
        // Bright foreground glowing bone
        Render::DrawLine(p1.x, p1.y, p2.x, p2.y, r, g, b, alpha, lineWidth);
    }


    static inline void DrawJointHub(const Vec2& pt, float radius, float r, float g, float b, float alpha) {
        Render::DrawFilledCircle(pt.x, pt.y, radius + 1.0f, 8, 0.0f, 0.0f, 0.0f, alpha * 0.95f);
        Render::DrawFilledCircle(pt.x, pt.y, radius, 8, r, g, b, alpha);
        Render::DrawFilledCircle(pt.x, pt.y, radius * 0.45f, 6, 1.0f, 1.0f, 1.0f, alpha);
    }

    static void DrawPlayerSkeleton(const PlayerData& p, int w, int h, float r, float g, float b, float a, float lineWidth) {
        Vec2 sHead = {0,0}, sNeck = {0,0}, sUpperSpine = {0,0}, sChest = {0,0}, sStomach = {0,0}, sPelvis = {0,0};
        Vec2 sLClavicle = {0,0}, sRClavicle = {0,0};
        Vec2 sLShoulder = {0,0}, sLElbow = {0,0}, sLHand = {0,0};
        Vec2 sRShoulder = {0,0}, sRElbow = {0,0}, sRHand = {0,0};
        Vec2 sLHip = {0,0}, sLKnee = {0,0}, sLAnkle = {0,0}, sLToe = {0,0};
        Vec2 sRHip = {0,0}, sRKnee = {0,0}, sRAnkle = {0,0}, sRToe = {0,0};

        bool okHead       = Math::WorldToScreen(p.headPos, sHead, w, h);
        bool okNeck       = Math::WorldToScreen(p.neckPos, sNeck, w, h);
        bool okUpperSpine = Math::WorldToScreen(p.upperSpinePos, sUpperSpine, w, h);
        bool okChest      = Math::WorldToScreen(p.chestPos, sChest, w, h);
        bool okStomach    = Math::WorldToScreen(p.stomachPos, sStomach, w, h);
        bool okPelvis     = Math::WorldToScreen(p.pelvisPos, sPelvis, w, h);

        if (!okHead && !okNeck && !okChest) return;

        // 1. Central Vertebral Column (Cervical, Thoracic & Lumbar Spine)
        if (okHead && okNeck)             DrawBoneSegment(sHead, sNeck, r, g, b, a, lineWidth);
        if (okNeck && okUpperSpine)       DrawBoneSegment(sNeck, sUpperSpine, r, g, b, a, lineWidth);
        if (okUpperSpine && okChest)      DrawBoneSegment(sUpperSpine, sChest, r, g, b, a, lineWidth);
        if (okChest && okStomach)         DrawBoneSegment(sChest, sStomach, r, g, b, a, lineWidth);
        if (okStomach && okPelvis)        DrawBoneSegment(sStomach, sPelvis, r, g, b, a, lineWidth);

        // 2. Clavicle & Shoulder Girdle
        bool okLClav = Math::WorldToScreen(p.lClaviclePos, sLClavicle, w, h);
        bool okRClav = Math::WorldToScreen(p.rClaviclePos, sRClavicle, w, h);
        bool okLSh   = Math::WorldToScreen(p.lShoulderPos, sLShoulder, w, h);
        bool okRSh   = Math::WorldToScreen(p.rShoulderPos, sRShoulder, w, h);

        if (okUpperSpine && okLClav) DrawBoneSegment(sUpperSpine, sLClavicle, r, g, b, a, lineWidth);
        if (okLClav && okLSh)        DrawBoneSegment(sLClavicle, sLShoulder, r, g, b, a, lineWidth);
        if (okUpperSpine && okRClav) DrawBoneSegment(sUpperSpine, sRClavicle, r, g, b, a, lineWidth);
        if (okRClav && okRSh)        DrawBoneSegment(sRClavicle, sRShoulder, r, g, b, a, lineWidth);

        // 3. Left Arm Chain (Shoulder -> Elbow -> Wrist/Hand)
        bool okLElb  = Math::WorldToScreen(p.lElbowPos, sLElbow, w, h);
        bool okLHnd  = Math::WorldToScreen(p.lHandPos, sLHand, w, h);
        if (okLSh && okLElb)  DrawBoneSegment(sLShoulder, sLElbow, r, g, b, a, lineWidth);
        if (okLElb && okLHnd) DrawBoneSegment(sLElbow, sLHand, r, g, b, a, lineWidth);

        // 4. Right Arm Chain (Shoulder -> Elbow -> Wrist/Weapon Grip)
        bool okRElb  = Math::WorldToScreen(p.rElbowPos, sRElbow, w, h);
        bool okRHnd  = Math::WorldToScreen(p.rHandPos, sRHand, w, h);
        if (okRSh && okRElb)  DrawBoneSegment(sRShoulder, sRElbow, r, g, b, a, lineWidth);
        if (okRElb && okRHnd) DrawBoneSegment(sRElbow, sRHand, r, g, b, a, lineWidth);

        // 5. Pelvic Girdle & Hip Joint Anchors
        bool okLHip = Math::WorldToScreen(p.lHipPos, sLHip, w, h);
        bool okRHip = Math::WorldToScreen(p.rHipPos, sRHip, w, h);
        if (okPelvis && okLHip) DrawBoneSegment(sPelvis, sLHip, r, g, b, a, lineWidth);
        if (okPelvis && okRHip) DrawBoneSegment(sPelvis, sRHip, r, g, b, a, lineWidth);

        // 6. Left Leg Chain (Hip -> Knee -> Ankle -> Toe)
        bool okLKnee  = Math::WorldToScreen(p.lKneePos, sLKnee, w, h);
        bool okLAnkle = Math::WorldToScreen(p.lAnklePos, sLAnkle, w, h);
        bool okLToe   = Math::WorldToScreen(p.lToePos, sLToe, w, h);
        if (okLHip && okLKnee)   DrawBoneSegment(sLHip, sLKnee, r, g, b, a, lineWidth);
        if (okLKnee && okLAnkle) DrawBoneSegment(sLKnee, sLAnkle, r, g, b, a, lineWidth);
        if (okLAnkle && okLToe)  DrawBoneSegment(sLAnkle, sLToe, r, g, b, a, lineWidth);

        // 7. Right Leg Chain (Hip -> Knee -> Ankle -> Toe)
        bool okRKnee  = Math::WorldToScreen(p.rKneePos, sRKnee, w, h);
        bool okRAnkle = Math::WorldToScreen(p.rAnklePos, sRAnkle, w, h);
        bool okRToe   = Math::WorldToScreen(p.rToePos, sRToe, w, h);
        if (okRHip && okRKnee)   DrawBoneSegment(sRHip, sRKnee, r, g, b, a, lineWidth);
        if (okRKnee && okRAnkle) DrawBoneSegment(sRKnee, sRAnkle, r, g, b, a, lineWidth);
        if (okRAnkle && okRToe)  DrawBoneSegment(sRAnkle, sRToe, r, g, b, a, lineWidth);

        // 8. Anatomical Joint Glow Hubs
        float hubRadius = (h >= 1080) ? 2.6f : 2.0f;
        if (okHead)       DrawJointHub(sHead, hubRadius + 1.2f, 1.0f, 1.0f, 1.0f, a);
        if (okNeck)       DrawJointHub(sNeck, hubRadius * 0.85f, r, g, b, a);
        if (okChest)      DrawJointHub(sChest, hubRadius, r, g, b, a);
        if (okStomach)    DrawJointHub(sStomach, hubRadius * 0.9f, r, g, b, a);
        if (okPelvis)     DrawJointHub(sPelvis, hubRadius, r, g, b, a);
        if (okLSh)        DrawJointHub(sLShoulder, hubRadius, r, g, b, a);
        if (okRSh)        DrawJointHub(sRShoulder, hubRadius, r, g, b, a);
        if (okLElb)       DrawJointHub(sLElbow, hubRadius * 0.85f, r, g, b, a);
        if (okRElb)       DrawJointHub(sRElbow, hubRadius * 0.85f, r, g, b, a);
        if (okLHnd)       DrawJointHub(sLHand, hubRadius * 0.95f, 1.0f, 1.0f, 1.0f, a);
        if (okRHnd)       DrawJointHub(sRHand, hubRadius * 0.95f, 1.0f, 1.0f, 1.0f, a);
        if (okLHip)       DrawJointHub(sLHip, hubRadius, r, g, b, a);
        if (okRHip)       DrawJointHub(sRHip, hubRadius, r, g, b, a);
        if (okLKnee)      DrawJointHub(sLKnee, hubRadius * 0.9f, r, g, b, a);
        if (okRKnee)      DrawJointHub(sRKnee, hubRadius * 0.9f, r, g, b, a);
        if (okLAnkle)     DrawJointHub(sLAnkle, hubRadius * 0.85f, r, g, b, a);
        if (okRAnkle)     DrawJointHub(sRAnkle, hubRadius * 0.85f, r, g, b, a);
        if (okLToe)       DrawJointHub(sLToe, hubRadius * 0.75f, 1.0f, 1.0f, 1.0f, a);
        if (okRToe)       DrawJointHub(sRToe, hubRadius * 0.75f, 1.0f, 1.0f, 1.0f, a);
    }


    static void DrawOffscreenIndicator(const Vec3& targetPos, int w, int h, float cr, float cg, float cb, float distMeters) {
        if (!Math::g_camValid) return;

        Vec3 delta = targetPos - Math::g_camPos;
        float fwdDot = delta.x * Math::g_camForward.x + delta.y * Math::g_camForward.y + delta.z * Math::g_camForward.z;
        float rgtDot = delta.x * Math::g_camRight.x   + delta.y * Math::g_camRight.y   + delta.z * Math::g_camRight.z;

        float angleRad = atan2f(-fwdDot, rgtDot) + 1.5707963f; // Screen coordinate angle

        float cx = (float)w * 0.5f;
        float cy = (float)h * 0.5f;
        float radius = (cy < cx ? cy : cx) * 0.78f;
        float arrowSize = (h >= 1080) ? 14.0f : 11.0f;

        Render::DrawOffscreenArrow(cx, cy, angleRad, radius, arrowSize, cr, cg, cb, 0.90f);

        // Distance Tag badge near arrow
        float tagX = cx + cosf(angleRad) * (radius - 24.0f);
        float tagY = cy + sinf(angleRad) * (radius - 24.0f);
        char dStr[16];
        snprintf(dStr, sizeof(dStr), "%.0fm", distMeters);
        float tagW = (float)strlen(dStr) * 8.0f + 8.0f;
        Render::DrawPillBadge(tagX - tagW * 0.5f, tagY - 7.0f, tagW, 14.0f, 0.03f, 0.04f, 0.06f, 0.85f, cr, cg, cb, 0.8f);
        Render::DrawString(tagX - tagW * 0.5f + 4.0f, tagY - 4.0f, 1.0f, 1.0f, 1.0f, dStr, 0.82f);
    }

    void Render(int w, int h, const MenuState& menu) {
        float fontScale = (h >= 1080) ? 1.05f : 0.92f;
        float boxLineWidth = (h >= 1080) ? 2.0f : 1.6f;
        float headRadius = (h >= 1080) ? 5.5f : 4.5f;
        float snaplineWidth = (h >= 1080) ? 1.5f : 1.2f;
        float barWidth = (h >= 1080) ? 4.5f : 3.5f;
        float barOffset = barWidth + 3.5f;

        float cx = (float)w * 0.5f, cy = (float)h * 0.5f;

        // 1. Custom / Sniper Crosshair
        if (menu.crosshair || menu.sniperCrosshair) {
            float chLen = (h >= 1080) ? 11.0f : 8.5f;
            float chGap = (h >= 1080) ? 4.0f : 3.0f;
            float chWidth = (h >= 1080) ? 2.0f : 1.5f;

            // Outer Shadow
            Render::DrawLine(cx - chLen - chGap, cy, cx - chGap, cy, 0.0f, 0.0f, 0.0f, 0.9f, chWidth + 1.2f);
            Render::DrawLine(cx + chGap, cy, cx + chLen + chGap, cy, 0.0f, 0.0f, 0.0f, 0.9f, chWidth + 1.2f);
            Render::DrawLine(cx, cy - chLen - chGap, cx, cy - chGap, 0.0f, 0.0f, 0.0f, 0.9f, chWidth + 1.2f);
            Render::DrawLine(cx, cy + chGap, cx, cy + chLen + chGap, 0.0f, 0.0f, 0.0f, 0.9f, chWidth + 1.2f);

            // Inner Accent Line
            Render::DrawLine(cx - chLen - chGap, cy, cx - chGap, cy, 0.0f, 1.0f, 0.4f, 0.95f, chWidth);
            Render::DrawLine(cx + chGap, cy, cx + chLen + chGap, cy, 0.0f, 1.0f, 0.4f, 0.95f, chWidth);
            Render::DrawLine(cx, cy - chLen - chGap, cx, cy - chGap, 0.0f, 1.0f, 0.4f, 0.95f, chWidth);
            Render::DrawLine(cx, cy + chGap, cx, cy + chLen + chGap, 0.0f, 1.0f, 0.4f, 0.95f, chWidth);

            // Center Precision Dot
            Render::DrawFilledCircle(cx, cy, 1.8f, 8, 1.0f, 0.2f, 0.2f, 0.95f);
        }

        if (!Engine::g_fnGetEntityByIndex) {
            Engine::ResolveFunctions();
        }

        g_cachedValidCount = 0;
        g_cachedOnScreenCount = 0;

        int localIdx = Engine::GetLocalPlayerIndex();
        bool isSpectator = Math::g_camSpectator;

        // Ensure players are updated once per frame
        Engine::UpdateAllPlayers(Hooks::g_frameCount);
        memcpy(g_cachedPlayers, Engine::g_players, sizeof(g_cachedPlayers));

        int inEyePlayerIndex = 0;
        if (isSpectator) {
            for (int k = 1; k <= 32; k++) {
                const PlayerData& cand = g_cachedPlayers[k - 1];
                if (cand.alive && cand.origin.Dist2D(Math::g_camPos) < 10.0f && fabsf(cand.origin.z - Math::g_camPos.z) < 45.0f) {
                    inEyePlayerIndex = k;
                    break;
                }
            }
        }

        int observerTeam = isSpectator ? (inEyePlayerIndex > 0 ? Engine::GetPlayerTeam(inEyePlayerIndex) : 0) : Engine::GetLocalPlayerTeam();
        float offMargin = (float)w * 0.35f;

        for (int i = 1; i <= 32; i++) {
            const PlayerData& p = g_cachedPlayers[i - 1];

            if (!p.alive) continue;
            if (!isSpectator && (p.isLocal || i == localIdx)) continue;
            if (isSpectator && inEyePlayerIndex == i) continue;

            bool filterTeammates = menu.enemyOnly || menu.aimEnemyOnly;
            if (filterTeammates && observerTeam != 0) {
                int targetTeam = p.team != 0 ? p.team : Engine::GetPlayerTeam(i);
                if (targetTeam == observerTeam) {
                    continue;
                }
            }

            g_cachedValidCount++;


            float cr, cg, cb;
            if (p.team == 1) { cr = 1.0f; cg = 0.25f; cb = 0.25f; } // Terrorist Modern Red
            else if (p.team == 2) { cr = 0.20f; cg = 0.65f; cb = 1.0f; } // CT Modern Azure
            else { cr = 1.0f; cg = 0.90f; cb = 0.20f; } // Unknown Gold

            Vec3 feetWorld = p.feetPos;
            Vec3 topWorld  = p.topPos;
            topWorld.z += (p.isDucking ? 1.0f : 1.5f);

            Vec2 sTop = {0, 0}, sFeet = {0, 0}, sOrigin = {0, 0};
            float zDist = 0.0f;
            const char* failReason = nullptr;

            bool w2sFeet = Math::WorldToScreen(feetWorld, sFeet, w, h, &zDist, &failReason);
            bool w2sTop  = Math::WorldToScreen(topWorld, sTop, w, h, nullptr, &failReason);

            // Offscreen Indicator execution for targets outside camera view frustum
            if (!w2sFeet || !w2sTop || zDist < 20.0f) {
                if (menu.offscreenEsp) {
                    DrawOffscreenIndicator(p.origin, w, h, cr, cg, cb, p.distanceMeters);
                }
                continue;
            }

            if (sFeet.x < -offMargin || sFeet.x > (float)w + offMargin ||
                sFeet.y < -offMargin || sFeet.y > (float)h + offMargin) {
                if (menu.offscreenEsp) {
                    DrawOffscreenIndicator(p.origin, w, h, cr, cg, cb, p.distanceMeters);
                }
                continue;
            }

            float topY    = sTop.y < sFeet.y ? sTop.y : sFeet.y;
            float bottomY = sTop.y > sFeet.y ? sTop.y : sFeet.y;
            float boxH    = bottomY - topY;

            if (boxH < 4.0f || boxH > (float)h * 1.5f) continue;

            float boxW = boxH * (p.isDucking ? 0.75f : 0.46f);
            if (boxW < 2.0f || boxW > (float)w * 1.5f) continue;

            bool hasOriginScreen = Math::WorldToScreen(p.origin, sOrigin, w, h);
            float midX = hasOriginScreen ? sOrigin.x : ((sFeet.x + sTop.x) * 0.5f);
            float bx   = midX - boxW * 0.5f;
            float by   = topY;

            g_cachedOnScreenCount++;

            // 1. Snaplines (Sleek gradient trace from bottom center)
            if (menu.snaplines) {
                Render::DrawGlowLine((float)w * 0.5f, (float)h, midX, bottomY, cr, cg, cb, 0.55f, snaplineWidth);
            }

            // 2. Box Chams / Alpha Glow Fill
            if (menu.boxChams) {
                float chamsA = (menu.chamsAlpha >= 0.05f && menu.chamsAlpha <= 0.90f) ? menu.chamsAlpha : 0.22f;
                Render::DrawFilledBox(bx, by, boxW, boxH, cr, cg, cb, chamsA);
            }

            // 3. Skeleton ESP (Full anatomical joint lines)
            if (menu.skeletonEsp) {
                DrawPlayerSkeleton(p, w, h, 1.0f, 1.0f, 1.0f, 0.88f, 1.4f);
            }

            // 4. ESP Bounding Box (2D Box vs Modern Corner Box)
            if (menu.espBox == BOX_2D || menu.espBox == 1) {
                // Drop shadow
                Render::DrawBox(bx - 1.0f, by - 1.0f, boxW + 2.0f, boxH + 2.0f, 0.0f, 0.0f, 0.0f, 0.9f, boxLineWidth + 1.2f);
                // Sharp colored box
                Render::DrawBox(bx, by, boxW, boxH, cr, cg, cb, 1.0f, boxLineWidth);
            } else if (menu.espBox == BOX_CORNER || menu.espBox == 2) {
                Render::DrawModernCornerBox(bx, by, boxW, boxH, cr, cg, cb, 1.0f, boxLineWidth);
            }

            // 5. Head Apex / Target Marker
            if (menu.headMarker) {
                Vec3 headWorld = {0, 0, 0};
                if (Engine::GetPlayerHeadPosition(p, headWorld)) {
                    Vec2 sHeadBone = {0, 0};
                    if (Math::WorldToScreen(headWorld, sHeadBone, w, h)) {
                        Render::DrawCircle(sHeadBone.x, sHeadBone.y, headRadius, 16, 1.0f, 1.0f, 1.0f, 0.95f, 1.2f);
                        Render::DrawFilledCircle(sHeadBone.x, sHeadBone.y, headRadius * 0.45f, 8, cr, cg, cb, 0.95f);
                    }
                }
            }

            // 6. Modern Health Bar (Solid / Gradient + Dynamic Numeric Pill)
            if (menu.espHealth != HP_OFF && menu.espHealth != 0) {
                float hpFrac = (float)p.health / 100.0f;
                if (hpFrac > 1.0f) hpFrac = 1.0f;
                if (hpFrac < 0.0f) hpFrac = 0.0f;
                float barH = boxH * hpFrac;
                float hpR = 1.0f - hpFrac;
                float hpG = hpFrac;

                // Dark Track Background
                Render::DrawFilledBox(bx - barOffset - 1.0f, by - 1.0f, barWidth + 2.0f, boxH + 2.0f, 0.02f, 0.03f, 0.05f, 0.88f);
                Render::DrawBox(bx - barOffset - 1.0f, by - 1.0f, barWidth + 2.0f, boxH + 2.0f, 0.0f, 0.0f, 0.0f, 0.95f, 1.0f);

                // Filled Bar
                if (menu.espHealth == HP_GRADIENT || menu.espHealth == 2) {
                    Render::DrawGradientBox(bx - barOffset, by + boxH - barH, barWidth, barH, 0.0f, 1.0f, 0.3f, 0.95f, hpR, hpG, 0.0f, 0.95f, true);
                } else {
                    Render::DrawFilledBox(bx - barOffset, by + boxH - barH, barWidth, barH, hpR, hpG, 0.0f, 0.95f);
                }

                // Numeric Health Badge Pill
                if (menu.hpText || p.health < 100) {
                    char hpStr[8];
                    snprintf(hpStr, sizeof(hpStr), "%d", p.health);
                    float hpBadgeW = (float)strlen(hpStr) * 7.0f * fontScale + 6.0f;
                    float hpBadgeY = by + boxH - barH - 7.0f;
                    Render::DrawPillBadge(bx - barOffset - hpBadgeW - 3.0f, hpBadgeY, hpBadgeW, 14.0f, 0.02f, 0.03f, 0.05f, 0.90f, hpR, hpG, 0.0f, 0.9f);
                    Render::DrawString(bx - barOffset - hpBadgeW - 1.0f, hpBadgeY + 3.0f, 1.0f, 1.0f, 1.0f, hpStr, fontScale * 0.80f);
                }
            }

            // 7. Modern High-Tech Info Pill Badges (Name, Weapon, Distance, Status Tags)
            if (menu.espInfo) {
                const char* pName = p.name[0] ? p.name : (p.modelName[0] ? p.modelName : "Player");

                // Top: Player Name Pill Badge
                size_t nameLen = strlen(pName);
                float nameBadgeW = (float)nameLen * 8.0f * fontScale + 12.0f;
                float nameBadgeH = 17.0f * fontScale;
                float nameX = midX - nameBadgeW * 0.5f;
                float nameY = by - nameBadgeH - 4.0f;

                Render::DrawPillBadge(nameX, nameY, nameBadgeW, nameBadgeH, 0.03f, 0.04f, 0.07f, 0.88f, cr, cg, cb, 0.85f);
                Render::DrawString(nameX + 6.0f, nameY + 4.0f, 1.0f, 1.0f, 1.0f, pName, fontScale * 0.95f);

                // Bottom: Weapon & Distance Badges
                float botY = by + boxH + 4.0f;
                if (p.weaponName[0]) {
                    char wepDistStr[64];
                    snprintf(wepDistStr, sizeof(wepDistStr), "%s | %.0fm", p.weaponName, p.distanceMeters);
                    float wepW = (float)strlen(wepDistStr) * 8.0f * fontScale + 12.0f;
                    float wepH = 16.0f * fontScale;
                    float wepX = midX - wepW * 0.5f;

                    Render::DrawPillBadge(wepX, botY, wepW, wepH, 0.03f, 0.04f, 0.07f, 0.85f, 0.3f, 0.6f, 0.9f, 0.75f);
                    Render::DrawString(wepX + 6.0f, botY + 3.5f, 0.85f, 0.92f, 1.0f, wepDistStr, fontScale * 0.88f);
                    botY += wepH + 3.0f;
                } else {
                    char distStr[32];
                    snprintf(distStr, sizeof(distStr), "%.0fm", p.distanceMeters);
                    float distW = (float)strlen(distStr) * 8.0f * fontScale + 10.0f;
                    float distH = 15.0f * fontScale;
                    float distX = midX - distW * 0.5f;

                    Render::DrawPillBadge(distX, botY, distW, distH, 0.03f, 0.04f, 0.07f, 0.85f, 0.5f, 0.5f, 0.5f, 0.6f);
                    Render::DrawString(distX + 5.0f, botY + 3.0f, 0.9f, 0.9f, 0.9f, distStr, fontScale * 0.85f);
                    botY += distH + 3.0f;
                }

                // Status Indicator Badges (Defusing / C4 Carrier)
                if (p.isDefusing) {
                    const char* defStr = "[ DEFUSING ]";
                    float defW = (float)strlen(defStr) * 8.0f * fontScale + 10.0f;
                    float defX = midX - defW * 0.5f;
                    float defY = nameY - 18.0f;
                    Render::DrawPillBadge(defX, defY, defW, 16.0f, 0.0f, 0.1f, 0.15f, 0.92f, 0.0f, 1.0f, 0.9f, 1.0f);
                    Render::DrawString(defX + 5.0f, defY + 3.5f, 0.0f, 1.0f, 0.9f, defStr, fontScale * 0.88f);
                    nameY = defY;
                }

                if (p.team == 1 && p.hasC4) {
                    const char* c4Str = "[ C4 CARRIER ]";
                    float c4W = (float)strlen(c4Str) * 8.0f * fontScale + 10.0f;
                    float c4X = midX - c4W * 0.5f;
                    float c4Y = nameY - 18.0f;
                    Render::DrawPillBadge(c4X, c4Y, c4W, 16.0f, 0.15f, 0.1f, 0.0f, 0.92f, 1.0f, 0.8f, 0.0f, 1.0f);
                    Render::DrawString(c4X + 5.0f, c4Y + 3.5f, 1.0f, 0.85f, 0.0f, c4Str, fontScale * 0.88f);
                }
            }
        }
    }
}
