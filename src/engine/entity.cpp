#include "entity.hpp"
#include "memory.hpp"
#include "../core/math.hpp"
#include <cstring>
#include <cctype>

namespace Engine {
    WorldEntityData g_worldEntities[64] = {};
    int             g_worldEntityCount  = 0;
    bool            g_playerHasC4[33]   = {false};

    bool StrContainsCaseInsensitive(const char* src, const char* sub) {
        if (!src || !sub) return false;
        while (*src) {
            const char* h = src;
            const char* n = sub;
            while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
                h++;
                n++;
            }
            if (!*n) return true;
            src++;
        }
        return false;
    }

    void ExtractWeaponName(const char* modelStr, char* outName, size_t outSize) {
        if (!modelStr || !outName || outSize == 0) return;
        outName[0] = 0;

        if (StrContainsCaseInsensitive(modelStr, "ak47")) strncpy(outName, "AK-47", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "m4a1")) strncpy(outName, "M4A1", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "awp")) strncpy(outName, "AWP", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "deagle")) strncpy(outName, "Deagle", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "usp")) strncpy(outName, "USP", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "glock")) strncpy(outName, "Glock", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "mp5")) strncpy(outName, "MP5", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "scout")) strncpy(outName, "Scout", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "aug")) strncpy(outName, "AUG", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "sg552")) strncpy(outName, "SG552", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "sg550")) strncpy(outName, "SG550", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "g3sg1")) strncpy(outName, "G3SG1", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "famas")) strncpy(outName, "FAMAS", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "galil")) strncpy(outName, "Galil", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "ump45")) strncpy(outName, "UMP-45", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "p90")) strncpy(outName, "P90", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "mac10")) strncpy(outName, "MAC-10", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "tmp")) strncpy(outName, "TMP", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "xm1014")) strncpy(outName, "XM1014", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "m3")) strncpy(outName, "M3 Shotgun", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "m249")) strncpy(outName, "M249", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "p228")) strncpy(outName, "P228", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "fiveseven")) strncpy(outName, "Five-Seven", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "elite")) strncpy(outName, "Dual Elites", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "knife")) strncpy(outName, "Knife", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "c4") || StrContainsCaseInsensitive(modelStr, "backpack")) strncpy(outName, "C4 Bomb", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "hegrenade")) strncpy(outName, "HE Grenade", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "flashbang")) strncpy(outName, "Flashbang", outSize);
        else if (StrContainsCaseInsensitive(modelStr, "smokegrenade")) strncpy(outName, "Smoke", outSize);
        outName[outSize - 1] = 0;
    }

    void RegisterWorldEntity(int index, void* ent, const char* modelname, uint64_t currentFrame) {
        if (!ent || !modelname || modelname[0] == 0) return;
        if (!IsReadableFast(ent, 0x2D0)) return;

        bool isC4 = (StrContainsCaseInsensitive(modelname, "w_c4") || StrContainsCaseInsensitive(modelname, "w_backpack"));
        bool isGrenade = (StrContainsCaseInsensitive(modelname, "w_hegrenade") ||
                          StrContainsCaseInsensitive(modelname, "w_flashbang") ||
                          StrContainsCaseInsensitive(modelname, "w_smokegrenade"));
        bool isWeapon = StrContainsCaseInsensitive(modelname, "models/w_");

        if (!isC4 && !isGrenade && !isWeapon) return;

        Vec3 entOrigin = {0, 0, 0};
        if (!SafeRead((addr_t)(uintptr_t)ent + 0x02C0, entOrigin) || !entOrigin.IsValid() || entOrigin.IsZero()) {
            if (!SafeRead((addr_t)(uintptr_t)ent + 0x00AC, entOrigin) || !entOrigin.IsValid() || entOrigin.IsZero()) {
                return;
            }
        }

        for (int i = 0; i < g_worldEntityCount; i++) {
            if (g_worldEntities[i].index == index || g_worldEntities[i].entAddr == (addr_t)(uintptr_t)ent) {
                g_worldEntities[i].origin = entOrigin;
                g_worldEntities[i].lastSeenFrame = currentFrame;
                g_worldEntities[i].active = true;
                if (Math::g_camValid) {
                    g_worldEntities[i].distanceMeters = entOrigin.Dist(Math::g_camPos) * 0.03125f;
                }
                return;
            }
        }

        if (g_worldEntityCount < 64) {
            WorldEntityData& wed = g_worldEntities[g_worldEntityCount];
            wed.index = index;
            wed.entAddr = (addr_t)(uintptr_t)ent;
            wed.origin = entOrigin;
            wed.isC4 = isC4;
            wed.isPlantedC4 = (isC4 && StrContainsCaseInsensitive(modelname, "w_c4"));
            wed.isGrenade = isGrenade;
            wed.active = true;
            wed.lastSeenFrame = currentFrame;
            strncpy(wed.modelName, modelname, sizeof(wed.modelName) - 1);

            ExtractWeaponName(modelname, wed.displayName, sizeof(wed.displayName));
            if (wed.displayName[0] == 0) {
                if (isC4) strncpy(wed.displayName, "C4 Bomb", sizeof(wed.displayName) - 1);
                else strncpy(wed.displayName, "Item", sizeof(wed.displayName) - 1);
            }

            if (Math::g_camValid) {
                wed.distanceMeters = entOrigin.Dist(Math::g_camPos) * 0.03125f;
            } else {
                wed.distanceMeters = 0.0f;
            }

            g_worldEntityCount++;
        }
    }

    void PruneWorldEntities(uint64_t currentFrame) {
        int writeIdx = 0;
        for (int i = 0; i < g_worldEntityCount; i++) {
            if (currentFrame >= g_worldEntities[i].lastSeenFrame &&
                (currentFrame - g_worldEntities[i].lastSeenFrame) <= 3) {
                if (writeIdx != i) {
                    g_worldEntities[writeIdx] = g_worldEntities[i];
                }
                writeIdx++;
            }
        }
        g_worldEntityCount = writeIdx;
    }
}
