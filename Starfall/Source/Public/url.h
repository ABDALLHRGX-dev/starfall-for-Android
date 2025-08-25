#pragma once
#include "ue.h"
#include <wchar.h>

namespace Starfall {
    struct URL {
        FString Scheme;
        FString Domain;
        FString Port;
        FString Path;
        FString Query;

        void Construct(FString url);
        void SetHost(FString host);
        void Dealloc();
        void DeallocPathQuery();

        operator FString();
    };

    void __URL_SetHost(URL* url, FString host);
}
