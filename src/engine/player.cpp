#include "player.hpp"
#include "memory.hpp"
#include "resolver.hpp"
#include "studio.hpp"
#include "entity.hpp"
#include "../core/math.hpp"
#include "../hooks/hooks.hpp"
#include <cstring>
#include <cmath>

namespace Engine {
    uint64_t g_lastActiveFrame[33] = {0};
    Vec3     g_activeOrigins[33]   = {{0,0,0}};
    bool     g_addEntityHooked     = false;

    PlayerData g_players[32] = {};
    int        g_validPlayerCount = 0;
    uint64_t   g_lastCacheFrame = 0;

    static int s_cachedTeam[33] = {0};
    static int s_cachedLocalIdx = -1;
    static int s_cachedLocalTeam = 0;

    int GetTeamFromModelName(const char* modelName) {
        if (!modelName || modelName[0] == 0) return 0;

        if (StrContainsCaseInsensitive(modelName, "terror") ||
            StrContainsCaseInsensitive(modelName, "leet") ||
            StrContainsCaseInsensitive(modelName, "arctic") ||
            StrContainsCaseInsensitive(modelName, "guerilla") ||
            StrContainsCaseInsensitive(modelName, "militia") ||
            StrContainsCaseInsensitive(modelName, "phoenix") ||
            StrContainsCaseInsensitive(modelName, "kurd") ||
            StrContainsCaseInsensitive(modelName, "arab")) {
            return 1;
        }
        if (StrContainsCaseInsensitive(modelName, "ct") ||
            StrContainsCaseInsensitive(modelName, "gign") ||
            StrContainsCaseInsensitive(modelName, "gsg9") ||
            StrContainsCaseInsensitive(modelName, "sas") ||
            StrContainsCaseInsensitive(modelName, "urban") ||
            StrContainsCaseInsensitive(modelName, "vip") ||
            StrContainsCaseInsensitive(modelName, "spetsnaz") ||
            StrContainsCaseInsensitive(modelName, "seal")) {
            return 2;
        }
        return 0;
    }

    int GetPlayerTeam(int index) {
        if (index < 1 || index > 32) return 0;

        if (g_fnHUD_GetPlayerTeam) {
            int t = g_fnHUD_GetPlayerTeam(index);
            if (t == 1 || t == 2) {
                s_cachedTeam[index] = t;
                return t;
            }
        }

        if (g_clientBase) {
            uintptr_t pExtra = g_clientBase + 0x12B2F4 + (index * 104);
            if (IsReadableFast((const void*)pExtra, 104)) {
                short teamNum = *(const short*)(pExtra + 0x06);
                if (teamNum == 1 || teamNum == 2) {
                    s_cachedTeam[index] = teamNum;
                    return teamNum;
                }
                const char* pTeamStr = (const char*)(pExtra + 0x08);
                if (IsReadableFast(pTeamStr, 4)) {
                    if (_strnicmp(pTeamStr, "TERROR", 6) == 0) {
                        s_cachedTeam[index] = 1;
                        return 1;
                    } else if (_strnicmp(pTeamStr, "CT", 2) == 0 || _strnicmp(pTeamStr, "COUNTER", 7) == 0) {
                        s_cachedTeam[index] = 2;
                        return 2;
                    }
                }
            }
        }

        if (g_fnGetPlayerInfo) {
            hud_player_info_t info = {0};
            if (g_fnGetPlayerInfo(index, &info) != 0 && info.model && IsReadableFast(info.model, 1)) {
                int modelTeam = GetTeamFromModelName(info.model);
                if (modelTeam != 0) {
                    s_cachedTeam[index] = modelTeam;
                    return modelTeam;
                }
            }
        }

        if (g_fnGetEntityByIndex) {
            void* pEnt = g_fnGetEntityByIndex(index);
            if (pEnt && IsReadableFast(pEnt, 0x300)) {
                addr_t pMdl = 0;
                SafeRead((addr_t)(uintptr_t)pEnt + 0x0284, pMdl);
                if (!pMdl) SafeRead((addr_t)(uintptr_t)pEnt + 0x01E0, pMdl);
                if (pMdl) {
                    char mdlStr[64] = {0};
                    if (SafeReadString(pMdl, mdlStr, sizeof(mdlStr))) {
                        int modelTeam = GetTeamFromModelName(mdlStr);
                        if (modelTeam != 0) {
                            s_cachedTeam[index] = modelTeam;
                            return modelTeam;
                        }
                    }
                }
            }
        }

        if (s_cachedTeam[index] == 1 || s_cachedTeam[index] == 2) {
            return s_cachedTeam[index];
        }

        return 0;
    }

    int GetLocalPlayerIndex() {
        if (g_fnGetLocalPlayer) {
            void* pLocal = g_fnGetLocalPlayer();
            if (pLocal && IsReadableFast(pLocal, 16)) {
                int idx = *(int*)pLocal;
                if (idx >= 1 && idx <= 32) {
                    s_cachedLocalIdx = idx;
                    return idx;
                }
            }
        }

        if (g_fnGetPlayerInfo) {
            for (int i = 1; i <= 32; i++) {
                hud_player_info_t info = {0};
                if (g_fnGetPlayerInfo(i, &info) != 0 && info.thisplayer != 0) {
                    s_cachedLocalIdx = i;
                    return i;
                }
            }
        }

        return (s_cachedLocalIdx >= 1 && s_cachedLocalIdx <= 32) ? s_cachedLocalIdx : -1;
    }

    int GetLocalPlayerTeam() {
        int idx = GetLocalPlayerIndex();
        if (idx >= 1 && idx <= 32) {
            int t = GetPlayerTeam(idx);
            if (t == 1 || t == 2) {
                s_cachedLocalTeam = t;
                return t;
            }
        }

        if (s_cachedLocalTeam == 1 || s_cachedLocalTeam == 2) {
            return s_cachedLocalTeam;
        }

        return 0;
    }

    bool GetHitboxWorldPosition(const PlayerData& p, int hitgroup, Vec3& outPos) {
        if (!p.alive || !p.origin.IsValid()) return false;

        if (hitgroup == HITGROUP_HEAD) {
            outPos = p.headPos;
            return outPos.IsValid();
        } else if (hitgroup == HITGROUP_CHEST) {
            outPos = p.chestPos;
            return outPos.IsValid();
        } else if (hitgroup == HITGROUP_STOMACH) {
            outPos = p.stomachPos;
            return outPos.IsValid();
        }
        outPos = p.headPos;
        return outPos.IsValid();
    }

    bool GetPlayerHeadPosition(const PlayerData& p, Vec3& outHeadPos) {
        return GetHitboxWorldPosition(p, HITGROUP_HEAD, outHeadPos);
    }

    bool ReadPlayer(int index, PlayerData& out) {
        out.index = index;
        out.entAddr = 0;
        out.modelAddr = 0;
        out.studioHdrAddr = 0;
        out.alive = false;
        out.isLocal = false;
        out.isDucking = false;
        out.isDefusing = false;
        out.hasC4 = false;
        out.hasStudioHitbox = false;
        out.headHitboxIndex = -1;
        out.headHitboxMin = {0, 0, 0};
        out.headHitboxMax = {0, 0, 0};
        out.distanceMeters = 0.0f;
        out.originOffset = 0;
        out.health = 100;
        out.team = 0;
        out.name[0] = 0;
        out.modelName[0] = 0;
        out.weaponName[0] = 0;
        out.origin = {0, 0, 0};
        out.angles = {0, 0, 0};
        out.headPos = {0, 0, 0};
        out.neckPos = {0, 0, 0};
        out.upperSpinePos = {0, 0, 0};
        out.chestPos = {0, 0, 0};
        out.stomachPos = {0, 0, 0};
        out.pelvisPos = {0, 0, 0};
        out.feetPos = {0, 0, 0};
        out.topPos = {0, 0, 0};
        out.eyePos = {0, 0, 0};

        out.lClaviclePos = {0, 0, 0}; out.rClaviclePos = {0, 0, 0};
        out.lShoulderPos = {0, 0, 0}; out.rShoulderPos = {0, 0, 0};
        out.lElbowPos = {0, 0, 0};    out.rElbowPos = {0, 0, 0};
        out.lHandPos = {0, 0, 0};     out.rHandPos = {0, 0, 0};
        out.lHipPos = {0, 0, 0};      out.rHipPos = {0, 0, 0};
        out.lKneePos = {0, 0, 0};     out.rKneePos = {0, 0, 0};
        out.lAnklePos = {0, 0, 0};    out.rAnklePos = {0, 0, 0};
        out.lFootPos = {0, 0, 0};     out.rFootPos = {0, 0, 0};
        out.lToePos = {0, 0, 0};      out.rToePos = {0, 0, 0};

        int localIdx = GetLocalPlayerIndex();
        if (index == localIdx) {
            out.isLocal = true;
            return false;
        }

        if (g_addEntityHooked) {
            if (Hooks::g_frameCount > g_lastActiveFrame[index] + 1) {
                return false;
            }
        }

        if (g_clientBase) {
            uintptr_t pExtra = g_clientBase + 0x12B2F4 + (index * 104);
            if (IsReadableFast((const void*)pExtra, 104)) {
                char isDead = *(const char*)(pExtra + 0x44);
                if (isDead != 0) {
                    return false;
                }
            }
        }

        if (g_fnGetPlayerInfo) {
            hud_player_info_t info = {0};
            if (g_fnGetPlayerInfo(index, &info) == 0) return false;
            if (!info.name || !IsReadableFast(info.name, 1) || info.name[0] == '\0') return false;
            if (info.spectator != 0) return false;
            if (info.thisplayer != 0) {
                out.isLocal = true;
                return false;
            }
            strncpy(out.name, info.name, sizeof(out.name) - 1);
            out.name[sizeof(out.name) - 1] = 0;
            if (info.model && IsReadableFast(info.model, 1)) {
                strncpy(out.modelName, info.model, sizeof(out.modelName) - 1);
                out.modelName[sizeof(out.modelName) - 1] = 0;
            }
        } else {
            return false;
        }

        void* pEnt = g_fnGetEntityByIndex ? g_fnGetEntityByIndex(index) : nullptr;
        if (!pEnt || !IsReadableFast(pEnt, 0x380)) return false;
        out.entAddr = (addr_t)(uintptr_t)pEnt;

        addr_t pModel = 0;
        SafeRead(out.entAddr + 0x0284, pModel);
        if (!pModel) SafeRead(out.entAddr + 0x02A4, pModel);
        if (!pModel) SafeRead(out.entAddr + 0x02DC, pModel);
        if (!pModel) return false;
        out.modelAddr = pModel;

        int curEffects = 0;
        if (SafeRead(out.entAddr + 0x0274, curEffects) || SafeRead(out.entAddr + 0x02E4, curEffects)) {
            if (curEffects & 0x80) return false;
        }

        int animSeq = 0;
        if (SafeRead(out.entAddr + 0x02DC, animSeq) || SafeRead(out.entAddr + 0x00A0, animSeq)) {
            if (animSeq >= 101 && animSeq <= 125) return false;
            if (animSeq >= 87 && animSeq <= 92) out.isDefusing = true;
        }

        short solidFlag = 2;
        if (SafeRead(out.entAddr + 0x02EA, solidFlag)) {
            if (solidFlag == 0) return false;
        }

        Vec3 liveOrigin = {0, 0, 0};
        if (SafeRead(out.entAddr + 0x02C0, liveOrigin) && liveOrigin.IsValid() && !liveOrigin.IsZero()) {
            out.origin = liveOrigin;
            out.originOffset = 0x02C0;
        } else if (SafeRead(out.entAddr + 0x00AC, liveOrigin) && liveOrigin.IsValid() && !liveOrigin.IsZero()) {
            out.origin = liveOrigin;
            out.originOffset = 0x00AC;
        } else if (SafeRead(out.entAddr + 0x00BC, liveOrigin) && liveOrigin.IsValid() && !liveOrigin.IsZero()) {
            out.origin = liveOrigin;
            out.originOffset = 0x00BC;
        } else {
            return false;
        }

        Vec3 liveAngles = {0, 0, 0};
        if (SafeRead(out.entAddr + 0x02CC, liveAngles) && liveAngles.IsValid()) {
            out.angles = liveAngles;
        } else if (SafeRead(out.entAddr + 0x00B8, liveAngles) && liveAngles.IsValid()) {
            out.angles = liveAngles;
        }

        if (Math::g_camValid) {
            out.distanceMeters = out.origin.Dist(Math::g_camPos) * 0.03125f;
        }

        int useHull = 0;
        SafeRead(out.entAddr + 0x00A0, useHull);
        out.isDucking = (useHull == 1);

        float yawRad = out.angles.y * Math::DEG2RAD;
        float pitchRad = out.angles.x * Math::DEG2RAD;
        float cosYaw = cosf(yawRad), sinYaw = sinf(yawRad);
        float cosPitch = cosf(pitchRad), sinPitch = sinf(pitchRad);

        Vec3 bodyFwd = { cosYaw, sinYaw, 0.0f };
        Vec3 bodyRgt = { sinYaw, -cosYaw, 0.0f };
        Vec3 aimFwd  = { cosPitch * cosYaw, cosPitch * sinYaw, -sinPitch };
        Vec3 aimUp   = { sinPitch * cosYaw, sinPitch * sinYaw, cosPitch };

        Vec3 attHead = {0, 0, 0};
        Vec3 attRightHand = {0, 0, 0};
        Vec3 attLeftHand = {0, 0, 0};
        Vec3 attBackSpine = {0, 0, 0};
        bool hasAttHead = false, hasAttRightHand = false, hasAttLeftHand = false, hasAttBack = false;

        Vec3 tempAtt;
        if (SafeRead(out.entAddr + 0x02D8, tempAtt) && tempAtt.IsValid() && !tempAtt.IsZero()) {
            if (out.origin.Dist2D(tempAtt) < 28.0f) { attHead = tempAtt; hasAttHead = true; }
        }
        if (SafeRead(out.entAddr + 0x02E4, tempAtt) && tempAtt.IsValid() && !tempAtt.IsZero()) {
            if (out.origin.Dist(tempAtt) < 48.0f) { attRightHand = tempAtt; hasAttRightHand = true; }
        }
        if (SafeRead(out.entAddr + 0x02F0, tempAtt) && tempAtt.IsValid() && !tempAtt.IsZero()) {
            if (out.origin.Dist(tempAtt) < 48.0f) { attLeftHand = tempAtt; hasAttLeftHand = true; }
        }
        if (SafeRead(out.entAddr + 0x02FC, tempAtt) && tempAtt.IsValid() && !tempAtt.IsZero()) {
            if (out.origin.Dist(tempAtt) < 36.0f) { attBackSpine = tempAtt; hasAttBack = true; }
        }

        studiohdr_t* pStudioHdr = GetStudioHeader(pModel);
        if (pStudioHdr) {
            out.studioHdrAddr = (addr_t)(uintptr_t)pStudioHdr;
            mstudiohitbox_t headHb = {0};
            int headHbIdx = -1;
            if (GetHitboxData(out.studioHdrAddr, HITGROUP_HEAD, headHb, headHbIdx)) {
                out.hasStudioHitbox = true;
                out.headHitboxIndex = headHbIdx;
                out.headHitboxMin = { headHb.bbmin[0], headHb.bbmin[1], headHb.bbmin[2] };
                out.headHitboxMax = { headHb.bbmax[0], headHb.bbmax[1], headHb.bbmax[2] };
            }
        }

        if (out.isDucking) {
            out.pelvisPos     = out.origin + Vec3(0, 0, -4.5f);
            out.stomachPos    = out.pelvisPos + Vec3(0, 0, 5.5f) + bodyFwd * 3.0f;
            out.chestPos      = out.stomachPos + Vec3(0, 0, 6.5f) + aimFwd * 2.5f;
            out.upperSpinePos = out.chestPos + Vec3(0, 0, 4.5f) + aimFwd * 1.5f;
            out.neckPos       = out.upperSpinePos + Vec3(0, 0, 3.5f) + aimFwd * 1.0f;
        } else {
            out.pelvisPos     = out.origin + Vec3(0, 0, -1.5f);
            out.stomachPos    = out.pelvisPos + Vec3(0, 0, 8.5f) + bodyFwd * 0.8f;
            out.chestPos      = out.stomachPos + Vec3(0, 0, 9.0f) + aimFwd * 2.0f;
            out.upperSpinePos = out.chestPos + Vec3(0, 0, 6.0f) + aimFwd * 1.5f;
            out.neckPos       = out.upperSpinePos + Vec3(0, 0, 4.0f) + aimFwd * 1.0f;
        }

        if (hasAttHead) {
            out.headPos = attHead;
        } else {
            float headZ = out.isDucking ? 6.5f : 8.5f;
            out.headPos = out.neckPos + Vec3(0, 0, headZ) + aimFwd * 1.8f;
        }

        float clavicleSpan = 5.0f;
        float shoulderSpan = 10.5f;
        out.lClaviclePos = out.upperSpinePos - bodyRgt * clavicleSpan + Vec3(0, 0, 0.5f);
        out.rClaviclePos = out.upperSpinePos + bodyRgt * clavicleSpan + Vec3(0, 0, 0.5f);
        out.lShoulderPos = out.upperSpinePos - bodyRgt * shoulderSpan + aimFwd * (sinPitch * 0.8f);
        out.rShoulderPos = out.upperSpinePos + bodyRgt * shoulderSpan + aimFwd * (sinPitch * 0.8f);

        if (hasAttRightHand) {
            out.rHandPos = attRightHand;
        } else {
            out.rHandPos = out.rShoulderPos + aimFwd * 14.5f - bodyRgt * 2.0f - Vec3(0, 0, out.isDucking ? 2.5f : 4.0f);
        }

        if (hasAttLeftHand) {
            out.lHandPos = attLeftHand;
        } else {
            out.lHandPos = out.lShoulderPos + aimFwd * 13.0f + bodyRgt * 3.5f - Vec3(0, 0, out.isDucking ? 3.0f : 5.0f);
        }

        Vec3 midL = (out.lShoulderPos + out.lHandPos) * 0.5f;
        Vec3 bendL = -bodyRgt * 5.2f - aimFwd * 1.6f - Vec3(0, 0, out.isDucking ? 4.5f : 6.0f);
        out.lElbowPos = midL + bendL;

        Vec3 midR = (out.rShoulderPos + out.rHandPos) * 0.5f;
        Vec3 bendR = bodyRgt * 5.2f - aimFwd * 1.6f - Vec3(0, 0, out.isDucking ? 4.5f : 6.0f);
        out.rElbowPos = midR + bendR;

        float hipSpan = 6.2f;
        out.lHipPos = out.pelvisPos - bodyRgt * hipSpan;
        out.rHipPos = out.pelvisPos + bodyRgt * hipSpan;

        float groundZ = out.origin.z - (out.isDucking ? 18.0f : 36.0f);

        if (out.isDucking) {
            out.lFootPos = out.lHipPos - bodyRgt * 2.5f + bodyFwd * 6.0f;
            out.lFootPos.z = groundZ;
            out.rFootPos = out.rHipPos + bodyRgt * 2.5f - bodyFwd * 5.0f;
            out.rFootPos.z = groundZ;

            out.lKneePos = (out.lHipPos + out.lFootPos) * 0.5f + bodyFwd * 9.0f + Vec3(0, 0, 3.5f);
            out.rKneePos = (out.rHipPos + out.rFootPos) * 0.5f + bodyFwd * 7.5f + Vec3(0, 0, 3.5f);
        } else {
            float stride = sinf(out.origin.x * 0.04f + out.origin.y * 0.04f) * 6.5f;
            out.lFootPos = out.lHipPos - bodyRgt * 1.2f + bodyFwd * stride;
            out.lFootPos.z = groundZ;
            out.rFootPos = out.rHipPos + bodyRgt * 1.2f - bodyFwd * stride;
            out.rFootPos.z = groundZ;

            out.lKneePos = (out.lHipPos + out.lFootPos) * 0.5f + bodyFwd * 3.5f;
            out.rKneePos = (out.rHipPos + out.rFootPos) * 0.5f + bodyFwd * 3.5f;
        }

        out.lAnklePos = out.lFootPos + Vec3(0, 0, 3.0f);
        out.rAnklePos = out.rFootPos + Vec3(0, 0, 3.0f);
        out.lToePos   = out.lFootPos + bodyFwd * 5.5f;
        out.rToePos   = out.rFootPos + bodyFwd * 5.5f;

        out.feetPos = out.origin;
        out.feetPos.z = groundZ;

        out.topPos = out.origin;
        out.topPos.z += (out.isDucking ? 18.0f : 36.0f);

        out.eyePos = out.origin;
        out.eyePos.z += (out.isDucking ? 12.0f : 28.0f);

        if (out.topPos.z - out.feetPos.z < 15.0f) {
            return false;
        }

        out.alive = true;

        out.team = GetPlayerTeam(index);
        if (out.team == 0 && out.modelName[0]) {
            out.team = GetTeamFromModelName(out.modelName);
        }

        if (out.team == 1) {
            out.hasC4 = g_playerHasC4[index];
        } else {
            out.hasC4 = false;
        }

        addr_t pWeaponModel = 0;
        if (SafeRead(out.entAddr + 0x01E0, pWeaponModel) && pWeaponModel) {
            char mdlPath[64] = {0};
            if (SafeReadString(pWeaponModel, mdlPath, sizeof(mdlPath)) || SafeReadString(pWeaponModel + 4, mdlPath, sizeof(mdlPath))) {
                ExtractWeaponName(mdlPath, out.weaponName, sizeof(out.weaponName));
            }
        }

        return true;
    }

    void UpdateAllPlayers(uint64_t frameCount) {
        if (g_lastCacheFrame == frameCount && frameCount != 0) return;
        g_lastCacheFrame = frameCount;
        g_validPlayerCount = 0;

        for (int i = 1; i <= 32; i++) {
            PlayerData p;
            bool valid = ReadPlayer(i, p);
            g_players[i - 1] = p;
            if (valid && p.alive) {
                g_validPlayerCount++;
            }
        }
    }
}
