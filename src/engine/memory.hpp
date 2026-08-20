#ifndef ENGINE_MEMORY_HPP
#define ENGINE_MEMORY_HPP

#include <windows.h>
#include "../sdk/sdk.hpp"

namespace Engine {
    bool IsReadableFast(const void* addr, size_t size);
    bool SafeReadBytes(const void* src, void* dst, size_t size);
    bool SafeReadString(addr_t addr, char* out, size_t maxLen);

    template<typename T>
    inline bool SafeRead(addr_t addr, T& out) {
        return SafeReadBytes((const void*)(uintptr_t)addr, &out, sizeof(T));
    }
}

#endif // ENGINE_MEMORY_HPP
