#include "base.h"
#include <sys/mman.h>
#include <dlfcn.h>
#include <android/log.h>

namespace Starfall {
    namespace Globals {
        void* buf = nullptr;
        void* EOSBuf = nullptr;

        void* tbuf = nullptr;
        size_t tsize = 0;

        void* rbuf = nullptr;
        size_t rsize = 0;

        void* EOSTextBuf = nullptr;
        size_t EOSTextSize = 0;

        void* EOSRDataBuf = nullptr;
        size_t EOSRDataSize = 0;
    }
}
