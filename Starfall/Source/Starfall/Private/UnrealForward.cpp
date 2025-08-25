#include "UnrealForward.h"
#include <android/log.h>

#define LOG_TAG "Starfall"

namespace Starfall {
    namespace UnrealForward {
        static int CurrentEngineVersion = 5;

        std::string GetEngineSoPath() {
            if (CurrentEngineVersion == 4)
                return "/lib/arm64-v8a/libUE4.so";
            else
                return "/lib/arm64-v8a/libUnreal.so";
        }

        bool InitializeEngine(int engineVersion) {
            CurrentEngineVersion = engineVersion;
            return UELoader::LoadEngineLibrary(GetEngineSoPath());
        }

        void* GetFCurlHttpRequestSymbol() {
            if (!UELoader::GetSymbol("FCurlHttpRequest")) {
                __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "FCurlHttpRequest symbol not found!");
            }
            return UELoader::GetSymbol("FCurlHttpRequest");
        }
    }
}
