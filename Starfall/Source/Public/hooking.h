#pragma once
#include "pch.h"
#include <sys/mman.h>
#include <cstdint>

namespace Starfall {
    namespace Hooking {
        inline void AsmHook(void* ptr, void* detour);

        bool Ret0Callback(struct pf_patch_t* patch, void* stream);

        void VTHook(void** addr, void* detour, void** orig);
    }

    __attribute__((noinline)) bool InternalCheckBytes(void* base, int ind, const uint8_t* bytes, size_t sz, bool upwards);
}
