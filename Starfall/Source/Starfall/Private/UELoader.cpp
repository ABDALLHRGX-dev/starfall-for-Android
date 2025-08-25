#include "UELoader.h"
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "Starfall"

namespace Starfall {
    namespace UELoader {
        static void* EngineHandle = nullptr;
        static std::string EngineName;

        bool LoadEngineLibrary(const std::string& path) {
            if (EngineHandle) return true;
            EngineHandle = dlopen(path.c_str(), RTLD_NOW);
            if (!EngineHandle) {
                __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Failed to load %s: %s", path.c_str(), dlerror());
                return false;
            }
            EngineName = path;
            __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Successfully loaded engine library: %s", path.c_str());
            return true;
        }

        void* GetSymbol(const char* symbol) {
            if (!EngineHandle) return nullptr;
            return dlsym(EngineHandle, symbol);
        }

        std::string GetLoadedEngineName() {
            return EngineName;
        }
    }
}
