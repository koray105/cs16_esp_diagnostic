#include "studio.hpp"
#include "memory.hpp"

namespace Engine {
    struct CachedModelData {
        addr_t       modelAddr;
        studiohdr_t* pHdr;
        bool         hasHeadHb;
        int          headHbIndex;
        mstudiohitbox_t headHb;
    };
    static CachedModelData s_modelCache[64] = {};
    static int             s_modelCacheCount = 0;

    studiohdr_t* GetStudioHeader(addr_t modelAddr) {
        if (!modelAddr || !IsReadableFast((const void*)(uintptr_t)modelAddr, 64)) return nullptr;

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
}
