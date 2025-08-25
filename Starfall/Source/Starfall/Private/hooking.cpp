#include "hooking.h"
#include <dlfcn.h>
#include <pthread.h>
#include <android/log.h>
#include <unistd.h>

#define LOG_TAG "Starfall"

namespace Starfall {
    namespace Hooking {

        void AsmHook(void* ptr, void* detour) {
            size_t page_size = sysconf(_SC_PAGESIZE);
            void* page_start = (void*)((uintptr_t)ptr & ~(page_size - 1));
            mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
            (void)detour; // للحفاظ على المعامل بدون استخدام تحذير
        }

        bool Ret0Callback(struct pf_patch_t* patch, void* stream) {
            (void)patch;
            (void)stream;
            return true;
        }

        void VTHook(void** addr, void* detour, void** orig) {
            size_t page_size = sysconf(_SC_PAGESIZE);
            void* page_start = (void*)((uintptr_t)addr & ~(page_size - 1));
            if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
                __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "mprotect failed");
                return;
            }
            *orig = *addr;
            *addr = detour;
        }

    } // namespace Hooking

    __attribute__((noinline)) bool InternalCheckBytes(void* base, int ind, const uint8_t* bytes, size_t sz, bool upwards) {
        auto offBase = (uint8_t*)(upwards ? (uintptr_t)base - ind : (uintptr_t)base + ind);
        for (int i = 0; i < sz; i++) {
            if (*(offBase + i) != bytes[i]) return false;
        }
        return true;
    }

} // namespace Starfall
