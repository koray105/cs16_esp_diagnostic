#include "memory.hpp"
#include <cstring>

namespace Engine {
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

    bool SafeReadString(addr_t addr, char* out, size_t maxLen) {
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
}
