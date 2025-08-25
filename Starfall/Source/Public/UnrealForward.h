#pragma once
#include "UELoader.h"
#include <string>

namespace Starfall {
    namespace UnrealForward {
        bool InitializeEngine(int engineVersion); // 4 or 5
        void* GetFCurlHttpRequestSymbol();
        std::string GetEngineSoPath();
    }
}
