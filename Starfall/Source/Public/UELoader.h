#pragma once
#include <string>

namespace Starfall {
    namespace UELoader {
        bool LoadEngineLibrary(const std::string& path);
        void* GetSymbol(const char* symbol);
        std::string GetLoadedEngineName();
    }
}
