#pragma once
#include "opts.h"
#include "pch.h"
#include <android/log.h>

namespace Starfall {
    namespace Globals {
        extern void* buf;
        extern void* EOSBuf;

        extern void* tbuf;
        extern size_t tsize;

        extern void* rbuf;
        extern size_t rsize;

        extern void* EOSTextBuf;
        extern size_t EOSTextSize;

        extern void* EOSRDataBuf;
        extern size_t EOSRDataSize;
    }
    using namespace Globals;

    inline bool initialized = false;
    inline bool IsInitialized() { return initialized; }
    inline void Init() { initialized = true; }
    inline void Shutdown() { initialized = false; }

    inline void HandleTouch(int x, int y) {
        __android_log_print(ANDROID_LOG_INFO, "Starfall", "HandleTouch called at (%d,%d)", x, y);
    }

    inline void OnPause() {
        __android_log_print(ANDROID_LOG_INFO, "Starfall", "OnPause called");
    }

    inline void OnResume() {
        __android_log_print(ANDROID_LOG_INFO, "Starfall", "OnResume called");
    }
}
using namespace Starfall;
