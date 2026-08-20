#include "engine.hpp"
#include "../core/logger.hpp"
#include "../core/math.hpp"
#include "../hooks/hooks.hpp"
#include <cstring>
#include <cctype>
#include <psapi.h>


namespace Engine {
    uintptr_t g_hwBase          = 0;
    DWORD     g_hwSize          = 0;
    uintptr_t g_clientBase      = 0;
    DWORD     g_clientSize      = 0;
    addr_t    g_pEngfuncsAddr   = 0;
    uintptr_t* g_pEngfuncsTable = nullptr;

    pfnClientCmd_t        g_fnClientCmd        = nullptr;
    pfnGetViewAngles_t   g_fnGetViewAngles    = nullptr;
    pfnSetViewAngles_t   g_fnSetViewAngles    = nullptr;
    pfnGetPlayerInfo_t    g_fnGetPlayerInfo    = nullptr;
    pfnTraceLine_t       g_fnTraceLine        = nullptr;
    pfnGetLocalPlayer_t   g_fnGetLocalPlayer   = nullptr;
    pfnGetEntityByIndex_t g_fnGetEntityByIndex = nullptr;
    pfnHUD_GetPlayerTeam_t g_fnHUD_GetPlayerTeam = nullptr;



    uint64_t g_lastActiveFrame[33] = {0};
    Vec3     g_activeOrigins[33]   = {{0,0,0}};
    bool     g_playerHasC4[33]     = {false};
    bool     g_addEntityHooked     = false;

    PlayerData g_players[32] = {};
    int        g_validPlayerCount = 0;
    uint64_t   g_lastCacheFrame = 0;

    WorldEntityData g_worldEntities[64] = {};
    int             g_worldEntityCount  = 0;

    // Fast O(1) Studio Header and Hitbox Cache to eliminate 128 IsBadReadPtr calls per frame
    struct CachedModelData {
        addr_t       modelAddr;
        studiohdr_t* pHdr;
        bool         hasHeadHb;
        int          headHbIndex;
        mstudiohitbox_t headHb;
    };
    static CachedModelData s_modelCache[64] = {};
    static int             s_modelCacheCount = 0;

    bool IsReadableFast(const void* addr, size_t size) {
        if (!addr || size == 0) return false;
        uintptr_t uAddr = (uintptr_t)addr;
        if (uAddr < 0x10000 || uAddr >= 0x7FFE0000) return false;
        return (IsBadReadPtr(addr, size) == 0);
    }

    bool SafeReadBytes(const void* src, void* dst, size_t size) {
        if (!IsReadableFast(src, size)) return false;
        memcpy(dst, src, size);
        return true;
    }

    static bool SafeReadString(addr_t addr, char* out, size_t maxLen) {
        if (!out || maxLen == 0) return false;
        out[0] = 0;
        if (!IsReadableFast((const void*)(uintptr_t)addr, 1)) return false;
        for (size_t i = 0; i < maxLen - 1; i++) {
            char c;
            if (!SafeReadBytes((const void*)(uintptr_t)(addr + i), &c, 1)) {
                out[i] = 0;
                return i > 0;
            }
            out[i] = c;
            if (c == 0) return true;
        }
        out[maxLen - 1] = 0;
        return true;
    }

    // Zero-allocation case-insensitive substring search
    static bool StrContainsCaseInsensitive(const char* src, const char* sub) {
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

    static void ExtractWeaponName(const char* modelStr, char* outName, size_t outSize) {
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


    bool ResolveFunctions() {
        HMODULE hClient = GetModuleHandleA("client.dll");
        if (!hClient) {
            Logger::Log("[-] client.dll not found in process space yet");
            return false;
        }
        g_clientBase = (uintptr_t)hClient;
        MODULEINFO mi = {0};
        if (GetModuleInformation(GetCurrentProcess(), hClient, &mi, sizeof(mi))) {
            g_clientSize = mi.SizeOfImage;
        }
        if (!g_clientSize) g_clientSize = 0x300000;

        // Resolve client.dll exported helper functions
        g_fnHUD_GetPlayerTeam = (pfnHUD_GetPlayerTeam_t)GetProcAddress(hClient, "HUD_GetPlayerTeam");


        // 1. Scan client.dll::Initialize for global gEngfuncs pointer / table
        BYTE* pInit = (BYTE*)GetProcAddress(hClient, "Initialize");
        if (pInit) {
            Logger::Log("[+] client.dll::Initialize at 0x%08X", (unsigned)(uintptr_t)pInit);

            for (int i = 0; i < 48; i++) {
                if (pInit[i] == 0xBF) { // mov edi, offset gEngfuncs
                    addr_t candidate = *(addr_t*)(pInit + i + 1);
                    if (candidate >= 0x10000 && candidate <= 0x7FFF0000 && IsReadableFast((const void*)(uintptr_t)candidate, 532)) {
                        g_pEngfuncsAddr = candidate;
                        g_pEngfuncsTable = (uintptr_t*)candidate;
                        Logger::Log("[+] Found gEngfuncs table via 'mov edi, 0x%08X'", (unsigned)candidate);
                        break;
                    }
                }
                if (pInit[i] == 0xA3) { // mov [offset gEngfuncs], eax
                    addr_t candidate = *(addr_t*)(pInit + i + 1);
                    if (candidate >= 0x10000 && candidate <= 0x7FFF0000 && IsReadableFast((const void*)(uintptr_t)candidate, 532)) {
                        g_pEngfuncsAddr = candidate;
                        g_pEngfuncsTable = (uintptr_t*)candidate;
                        Logger::Log("[+] Found gEngfuncs pointer via 'mov [0x%08X], eax'", (unsigned)candidate);
                        break;
                    }
                }
            }
        }

        // 2. Direct RVA Fallback for OyunYoneticisi / Standard CS 1.6 build
        if (!g_pEngfuncsTable) {
            addr_t knownRVA = 0x00121BA0;
            addr_t candidate = g_clientBase + knownRVA;
            if (IsReadableFast((const void*)(uintptr_t)candidate, 532)) {
                g_pEngfuncsAddr = candidate;
                g_pEngfuncsTable = (uintptr_t*)candidate;
                Logger::Log("[+] Resolved gEngfuncs table via validated RVA 0x%08X (VA: 0x%08X)", (unsigned)knownRVA, (unsigned)candidate);
            }
        }

        // 3. Fallback to hw.dll engine table at RVA 0x166A98
        if (!g_pEngfuncsTable && g_hwBase) {
            addr_t candidate = g_hwBase + 0x166A98;
            if (IsReadableFast((const void*)(uintptr_t)candidate, 532)) {
                g_pEngfuncsAddr = candidate;
                g_pEngfuncsTable = (uintptr_t*)candidate;
                Logger::Log("[+] Resolved gEngfuncs table via hw.dll source table (VA: 0x%08X)", (unsigned)candidate);
            }
        }

        if (g_pEngfuncsTable) {
            // GoldSrc official cl_enginefuncs_s indices:
            // Index 20 (0x50): pfnClientCmd
            // Index 21 (0x54): pfnGetPlayerInfo
            if (IsReadableFast((const void*)g_pEngfuncsTable[20], 16)) {
                g_fnClientCmd = (pfnClientCmd_t)g_pEngfuncsTable[20];
            }
            if (IsReadableFast((const void*)g_pEngfuncsTable[21], 16)) {
                g_fnGetPlayerInfo = (pfnGetPlayerInfo_t)g_pEngfuncsTable[21];
            }

            // Dynamic Runtime Probing for GetViewAngles & SetViewAngles (GoldSrc index 32/33 or 33/34)
            for (int idx = 31; idx <= 34; idx++) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[idx], 16)) {
                    pfnGetViewAngles_t pTestGet = (pfnGetViewAngles_t)g_pEngfuncsTable[idx];
                    float testAngles[3] = { -9999.0f, -9999.0f, -9999.0f };
                    pTestGet(testAngles);
                    if (testAngles[0] > -180.0f && testAngles[0] < 180.0f &&
                        testAngles[1] > -360.0f && testAngles[1] < 360.0f) {
                        g_fnGetViewAngles = pTestGet;
                        if (IsReadableFast((const void*)g_pEngfuncsTable[idx + 1], 16)) {
                            g_fnSetViewAngles = (pfnSetViewAngles_t)g_pEngfuncsTable[idx + 1];
                        }
                        break;
                    }
                }
            }
            if (!g_fnGetViewAngles) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[33], 16)) g_fnGetViewAngles = (pfnGetViewAngles_t)g_pEngfuncsTable[33];
                else if (IsReadableFast((const void*)g_pEngfuncsTable[32], 16)) g_fnGetViewAngles = (pfnGetViewAngles_t)g_pEngfuncsTable[32];
            }
            if (!g_fnSetViewAngles) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[34], 16)) g_fnSetViewAngles = (pfnSetViewAngles_t)g_pEngfuncsTable[34];
                else if (IsReadableFast((const void*)g_pEngfuncsTable[33], 16)) g_fnSetViewAngles = (pfnSetViewAngles_t)g_pEngfuncsTable[33];
            }

            // Dynamic Runtime Probing for GetLocalPlayer & GetEntityByIndex
            for (int idx = 49; idx <= 52; idx++) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[idx], 16)) {
                    pfnGetLocalPlayer_t pTestLocal = (pfnGetLocalPlayer_t)g_pEngfuncsTable[idx];
                    void* pLocal = pTestLocal();
                    if (pLocal && IsReadableFast(pLocal, 0x100)) {
                        int entIndex = *(int*)pLocal;
                        if (entIndex >= 1 && entIndex <= 32) {
                            g_fnGetLocalPlayer = pTestLocal;
                            break;
                        }
                    }
                }
            }
            if (!g_fnGetLocalPlayer) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[51], 16)) g_fnGetLocalPlayer = (pfnGetLocalPlayer_t)g_pEngfuncsTable[51];
                else if (IsReadableFast((const void*)g_pEngfuncsTable[50], 16)) g_fnGetLocalPlayer = (pfnGetLocalPlayer_t)g_pEngfuncsTable[50];
            }

            for (int idx = 51; idx <= 55; idx++) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[idx], 16)) {
                    pfnGetEntityByIndex_t pTestEnt = (pfnGetEntityByIndex_t)g_pEngfuncsTable[idx];
                    void* pEnt1 = pTestEnt(1);
                    if (pEnt1 && IsReadableFast(pEnt1, 0x100)) {
                        int entIndex1 = *(int*)pEnt1;
                        if (entIndex1 == 1) {
                            g_fnGetEntityByIndex = pTestEnt;
                            break;
                        }
                    }
                }
            }
            if (!g_fnGetEntityByIndex) {
                if (IsReadableFast((const void*)g_pEngfuncsTable[53], 16)) g_fnGetEntityByIndex = (pfnGetEntityByIndex_t)g_pEngfuncsTable[53];
                else if (IsReadableFast((const void*)g_pEngfuncsTable[52], 16)) g_fnGetEntityByIndex = (pfnGetEntityByIndex_t)g_pEngfuncsTable[52];
            }

            if (IsReadableFast((const void*)g_pEngfuncsTable[57], 16)) {
                g_fnTraceLine = (pfnTraceLine_t)g_pEngfuncsTable[57];
            } else if (IsReadableFast((const void*)g_pEngfuncsTable[56], 16)) {
                g_fnTraceLine = (pfnTraceLine_t)g_pEngfuncsTable[56];
            }

            Logger::Log("[+] Engine function bindings: ClientCmd=0x%08X | GetViewAngles=0x%08X | SetViewAngles=0x%08X | TraceLine=0x%08X | GetPlayerInfo=0x%08X | GetEntityByIndex=0x%08X | GetLocalPlayer=0x%08X",
                        (unsigned)(uintptr_t)g_fnClientCmd,
                        (unsigned)(uintptr_t)g_fnGetViewAngles,
                        (unsigned)(uintptr_t)g_fnSetViewAngles,
                        (unsigned)(uintptr_t)g_fnTraceLine,
                        (unsigned)(uintptr_t)g_fnGetPlayerInfo,
                        (unsigned)(uintptr_t)g_fnGetEntityByIndex,
                        (unsigned)(uintptr_t)g_fnGetLocalPlayer);

            return (g_fnGetPlayerInfo != nullptr && g_fnGetEntityByIndex != nullptr);
        }

        Logger::Log("[-] Failed to locate gEngfuncs table");
        return false;
    }

    bool IsTargetVisible(const Vec3& start, const Vec3& end, int ignoreEntIndex, int targetEntIndex) {
        if (!g_fnTraceLine) return true; // Fallback to true if traceLine not bound

        float vStart[3] = { start.x, start.y, start.z };
        float vEnd[3]   = { end.x,   end.y,   end.z };

        pmtrace_t tr = {0};
        // Trace flags: 1 = PM_STUDIO_IGNORE (ignores player models, collides with BSP map world geometry)
        // usehull: 0 = point ray in GoldSrc (zero-width line-of-sight ray)
        g_fnTraceLine(vStart, vEnd, 1, 0, ignoreEntIndex, &tr);

        // Ray reached target without hitting solid BSP walls
        if (tr.allsolid == 0 && tr.fraction >= 0.95f) return true;

        // Ray hit the target entity itself directly
        if (targetEntIndex > 0 && tr.ent == targetEntIndex) return true;

        return false;
    }


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
            return 1; // Terrorist
        }
        if (StrContainsCaseInsensitive(modelName, "ct") ||
            StrContainsCaseInsensitive(modelName, "gign") ||
            StrContainsCaseInsensitive(modelName, "gsg9") ||
            StrContainsCaseInsensitive(modelName, "sas") ||
            StrContainsCaseInsensitive(modelName, "urban") ||
            StrContainsCaseInsensitive(modelName, "vip") ||
            StrContainsCaseInsensitive(modelName, "spetsnaz") ||
            StrContainsCaseInsensitive(modelName, "seal")) {
            return 2; // Counter-Terrorist
        }
        return 0;
    }

    int GetPlayerTeam(int index) {
        if (index < 1 || index > 32) return 0;

        // 1. Exported HUD_GetPlayerTeam from client.dll
        if (g_fnHUD_GetPlayerTeam) {
            int t = g_fnHUD_GetPlayerTeam(index);
            if (t == 1 || t == 2) {
                s_cachedTeam[index] = t;
                return t;
            }
        }

        // 2. Direct read from client.dll g_PlayerExtraInfo table (+0x12B2F4)
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

        // 3. Fallback to hud_player_info_t model check
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

        // 4. Fallback to cl_entity_t model path check
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

        // 5. Return cached team if previously resolved
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

        // Fallback scan via thisplayer flag in hud_player_info_t
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

    studiohdr_t* GetStudioHeader(addr_t modelAddr) {
        if (!modelAddr || !IsReadableFast((const void*)(uintptr_t)modelAddr, 64)) return nullptr;

        // Check fast studio cache
        for (int i = 0; i < s_modelCacheCount; i++) {
            if (s_modelCache[i].modelAddr == modelAddr) {
                return s_modelCache[i].pHdr;
            }
        }

        studiohdr_t* pHdr = nullptr;
        int magic = 0, ver = 0;
        if (SafeRead(modelAddr, magic) && magic == STUDIO_MAGIC) {
            if (SafeRead(modelAddr + 4, ver) && ver == STUDIO_VERSION) {
                pHdr = (studiohdr_t*)(uintptr_t)modelAddr;
            }
        }
        if (!pHdr) {
            for (size_t off = 0; off < 0x200; off += 4) {
                addr_t candidate = 0;
                if (SafeRead(modelAddr + off, candidate) && candidate >= 0x10000 && candidate <= 0x7FFE0000) {
                    if (IsReadableFast((const void*)(uintptr_t)candidate, sizeof(studiohdr_t))) {
                        int cMagic = 0, cVer = 0;
                        if (SafeRead(candidate, cMagic) && cMagic == STUDIO_MAGIC) {
                            if (SafeRead(candidate + 4, cVer) && cVer == STUDIO_VERSION) {
                                pHdr = (studiohdr_t*)(uintptr_t)candidate;
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Cache resolved studio header & hitboxes
        if (pHdr && s_modelCacheCount < 64) {
            CachedModelData& entry = s_modelCache[s_modelCacheCount++];
            entry.modelAddr = modelAddr;
            entry.pHdr = pHdr;
            entry.hasHeadHb = false;
            entry.headHbIndex = -1;

            addr_t hdrAddr = (addr_t)(uintptr_t)pHdr;
            studiohdr_t hdr = {0};
            if (SafeRead(hdrAddr, hdr) && hdr.numhitboxes > 0 && hdr.numhitboxes <= 64 && hdr.hitboxindex > 0) {
                addr_t hitboxesBase = hdrAddr + hdr.hitboxindex;
                for (int k = 0; k < hdr.numhitboxes; k++) {
                    mstudiohitbox_t hb = {0};
                    if (SafeRead(hitboxesBase + k * sizeof(mstudiohitbox_t), hb)) {
                        if (hb.group == HITGROUP_HEAD) {
                            entry.hasHeadHb = true;
                            entry.headHbIndex = k;
                            entry.headHb = hb;
                            break;
                        }
                    }
                }
            }
        }

        return pHdr;
    }

    bool GetHitboxData(addr_t studioHdrAddr, int desiredGroup, mstudiohitbox_t& outHitbox, int& outHitboxIndex) {
        if (!studioHdrAddr) return false;

        // Check studio cache
        for (int i = 0; i < s_modelCacheCount; i++) {
            if ((addr_t)(uintptr_t)s_modelCache[i].pHdr == studioHdrAddr) {
                if (desiredGroup == HITGROUP_HEAD && s_modelCache[i].hasHeadHb) {
                    outHitbox = s_modelCache[i].headHb;
                    outHitboxIndex = s_modelCache[i].headHbIndex;
                    return true;
                }
            }
        }

        if (!IsReadableFast((const void*)(uintptr_t)studioHdrAddr, sizeof(studiohdr_t))) return false;
        studiohdr_t hdr = {0};
        if (!SafeRead(studioHdrAddr, hdr)) return false;
        if (hdr.id != STUDIO_MAGIC || hdr.version != STUDIO_VERSION) return false;
        if (hdr.numhitboxes <= 0 || hdr.numhitboxes > 64 || hdr.hitboxindex <= 0) return false;

        addr_t hitboxesBase = studioHdrAddr + hdr.hitboxindex;
        if (!IsReadableFast((const void*)(uintptr_t)hitboxesBase, hdr.numhitboxes * sizeof(mstudiohitbox_t))) return false;

        for (int k = 0; k < hdr.numhitboxes; k++) {
            mstudiohitbox_t hb = {0};
            if (SafeRead(hitboxesBase + k * sizeof(mstudiohitbox_t), hb)) {
                if (hb.group == desiredGroup) {
                    outHitbox = hb;
                    outHitboxIndex = k;
                    return true;
                }
            }
        }
        return false;
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


        // 1. Local Player Identification
        int localIdx = GetLocalPlayerIndex();
        if (index == localIdx) {
            out.isLocal = true;
            return false;
        }

        // 2. Active Engine Rendering Pipeline Check (Instant 0-frame death gate)
        if (g_addEntityHooked) {
            if (Hooks::g_frameCount > g_lastActiveFrame[index] + 1) {
                return false;
            }
        }

        // 3. Player Death Verification via client.dll Scoreboard/ExtraInfo
        if (g_clientBase) {
            uintptr_t pExtra = g_clientBase + 0x12B2F4 + (index * 104);
            if (IsReadableFast((const void*)pExtra, 104)) {
                char isDead = *(const char*)(pExtra + 0x44);
                if (isDead != 0) {
                    return false; // Confirmed dead by client HUD state
                }
            }
        }

        // 4. Player Info Validation (Connected & Active)
        if (g_fnGetPlayerInfo) {
            hud_player_info_t info = {0};
            if (g_fnGetPlayerInfo(index, &info) == 0) {
                return false;
            }
            if (!info.name || !IsReadableFast(info.name, 1) || info.name[0] == '\0') {
                return false;
            }
            if (info.spectator != 0) {
                return false;
            }
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

        // 5. Fetch and Validate cl_entity_t
        void* pEnt = g_fnGetEntityByIndex ? g_fnGetEntityByIndex(index) : nullptr;
        if (!pEnt || !IsReadableFast(pEnt, 0x380)) return false;
        out.entAddr = (addr_t)(uintptr_t)pEnt;

        // Model pointer check (If model is 0, player has no active 3D model in game world)
        addr_t pModel = 0;
        SafeRead(out.entAddr + 0x0284, pModel);
        if (!pModel) SafeRead(out.entAddr + 0x02A4, pModel);
        if (!pModel) SafeRead(out.entAddr + 0x02DC, pModel);
        if (!pModel) return false;
        out.modelAddr = pModel;

        // Check EF_NODRAW (0x80) in curstate effects
        int curEffects = 0;
        if (SafeRead(out.entAddr + 0x0274, curEffects) || SafeRead(out.entAddr + 0x02E4, curEffects)) {
            if (curEffects & 0x80) return false;
        }

        // Instant Death Check: Sequence >= 101 (Death animations in standard GoldSrc player models)
        int animSeq = 0;
        if (SafeRead(out.entAddr + 0x02DC, animSeq) || SafeRead(out.entAddr + 0x00A0, animSeq)) {
            if (animSeq >= 101 && animSeq <= 125) {
                return false; // Player is playing death animation (0ms instant elimination)
            }
            // Check for defusing sequence (around sequence 87..92 depending on model)
            if (animSeq >= 87 && animSeq <= 92) {
                out.isDefusing = true;
            }
        }

        // Instant Death Check: Solid flag (SOLID_NOT == 0 upon death)
        short solidFlag = 2;
        if (SafeRead(out.entAddr + 0x02EA, solidFlag)) {
            if (solidFlag == 0) {
                return false; // Non-solid corpse
            }
        }

        // 6. Resolve Live Real-time Render Origin (Synchronous Zero-Lag at offset 0x02C0)
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

        // Read Live Render Angles
        Vec3 liveAngles = {0, 0, 0};
        if (SafeRead(out.entAddr + 0x02CC, liveAngles) && liveAngles.IsValid()) {
            out.angles = liveAngles;
        } else if (SafeRead(out.entAddr + 0x00B8, liveAngles) && liveAngles.IsValid()) {
            out.angles = liveAngles;
        }

        // Calculate exact distance in meters (32 units = 1 meter in GoldSrc)
        if (Math::g_camValid) {
            out.distanceMeters = out.origin.Dist(Math::g_camPos) * 0.03125f;
        }

        // 7. Ducking / Crouching Hull Detection
        int useHull = 0;
        SafeRead(out.entAddr + 0x00A0, useHull);
        out.isDucking = (useHull == 1);

        // 8. Transform Studio Skeleton to Anatomical World-Space Target Positions
        float yawRad = out.angles.y * Math::DEG2RAD;
        float pitchRad = out.angles.x * Math::DEG2RAD;
        float cosYaw = cosf(yawRad), sinYaw = sinf(yawRad);
        float cosPitch = cosf(pitchRad), sinPitch = sinf(pitchRad);

        // Body Orientation Bases
        Vec3 bodyFwd = { cosYaw, sinYaw, 0.0f };
        Vec3 bodyRgt = { sinYaw, -cosYaw, 0.0f };
        Vec3 aimFwd  = { cosPitch * cosYaw, cosPitch * sinYaw, -sinPitch };
        Vec3 aimUp   = { sinPitch * cosYaw, sinPitch * sinYaw, cosPitch };

        // Read all 4 live model attachments from engine entity structure
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

        // Resolve Studio Header and Hitboxes
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

        // Anatomical Spine & Torso Construction
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

        // Head Apex and Cranium Center
        if (hasAttHead) {
            out.headPos = attHead;
        } else {
            float headZ = out.isDucking ? 6.5f : 8.5f;
            out.headPos = out.neckPos + Vec3(0, 0, headZ) + aimFwd * 1.8f;
        }

        // Clavicle Girdle & Upper Shoulder Joints
        float clavicleSpan = 5.0f;
        float shoulderSpan = 10.5f;
        out.lClaviclePos = out.upperSpinePos - bodyRgt * clavicleSpan + Vec3(0, 0, 0.5f);
        out.rClaviclePos = out.upperSpinePos + bodyRgt * clavicleSpan + Vec3(0, 0, 0.5f);
        out.lShoulderPos = out.upperSpinePos - bodyRgt * shoulderSpan + aimFwd * (sinPitch * 0.8f);
        out.rShoulderPos = out.upperSpinePos + bodyRgt * shoulderSpan + aimFwd * (sinPitch * 0.8f);

        // Hands & Weapon Grip Anchors
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

        // Natural 2-Link Arm Inverse Kinematics for Left & Right Elbows
        Vec3 midL = (out.lShoulderPos + out.lHandPos) * 0.5f;
        Vec3 bendL = -bodyRgt * 5.2f - aimFwd * 1.6f - Vec3(0, 0, out.isDucking ? 4.5f : 6.0f);
        out.lElbowPos = midL + bendL;

        Vec3 midR = (out.rShoulderPos + out.rHandPos) * 0.5f;
        Vec3 bendR = bodyRgt * 5.2f - aimFwd * 1.6f - Vec3(0, 0, out.isDucking ? 4.5f : 6.0f);
        out.rElbowPos = midR + bendR;

        // Pelvic Girdle & Leg Joints
        float hipSpan = 6.2f;
        out.lHipPos = out.pelvisPos - bodyRgt * hipSpan;
        out.rHipPos = out.pelvisPos + bodyRgt * hipSpan;

        float groundZ = out.origin.z - (out.isDucking ? 18.0f : 36.0f);

        // Gait and Leg Stride Stance Calculation
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

        // Ankle and Toe Terminals
        out.lAnklePos = out.lFootPos + Vec3(0, 0, 3.0f);
        out.rAnklePos = out.rFootPos + Vec3(0, 0, 3.0f);
        out.lToePos   = out.lFootPos + bodyFwd * 5.5f;
        out.rToePos   = out.rFootPos + bodyFwd * 5.5f;

        // Set true ground feet position
        out.feetPos = out.origin;
        out.feetPos.z = groundZ;

        // Set true head top apex position (hull crown)
        out.topPos = out.origin;
        out.topPos.z += (out.isDucking ? 18.0f : 36.0f);

        // Set eye position
        out.eyePos = out.origin;
        out.eyePos.z += (out.isDucking ? 12.0f : 28.0f);


        // Sanity check: Living player height must be at least 15 units
        if (out.topPos.z - out.feetPos.z < 15.0f) {
            return false;
        }

        out.alive = true;

        // 8. Multi-Layered Rock-Solid Team Resolution
        out.team = GetPlayerTeam(index);
        if (out.team == 0 && out.modelName[0]) {
            out.team = GetTeamFromModelName(out.modelName);
        }

        // 9. C4 Carrier Detection (STRICT: ONLY Terrorists (team == 1) can carry C4)
        if (out.team == 1) {
            out.hasC4 = g_playerHasC4[index];
        } else {
            out.hasC4 = false; // CTs NEVER carry C4!
        }

        // 10. Inspect Active Weapon Model via weapon entity slot or attachment
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

        // Check if entity already registered in world list
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

        // Add new entity
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

