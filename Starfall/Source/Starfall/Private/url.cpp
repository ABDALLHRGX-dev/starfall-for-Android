#include "url.h"

namespace Starfall {
    void URL::Construct(FString url) {
        size_t schemeEnd = url.find(L"://");
        if (schemeEnd != FString::npos) {
            Scheme = url.substr(0, schemeEnd);
            url = url.substr(schemeEnd + 3);
        }

        size_t domainEnd = url.find(L"/");
        if (domainEnd == FString::npos) domainEnd = url.Length;
        Domain = url.substr(0, domainEnd);
        url = url.substr(domainEnd);

        size_t portStart = Domain.find(L":");
        if (portStart != FString::npos) {
            Port = Domain.substr(portStart + 1);
            Domain = Domain.substr(0, portStart);
        }

        size_t queryStart = url.find(L"?");
        if (queryStart != FString::npos) {
            Query = url.substr(queryStart + 1);
            Path = url.substr(0, queryStart);
        } else {
            Path = url;
        }
    }

    void URL::SetHost(FString host) {
        Domain = host;
    }

    void URL::Dealloc() {
        Scheme.Dealloc();
        Domain.Dealloc();
        Port.Dealloc();
        Path.Dealloc();
        Query.Dealloc();
    }

    void URL::DeallocPathQuery() {
        Path.Dealloc();
        Query.Dealloc();
    }

    URL::operator FString() {
        FString result = Scheme + FString(L"://") + Domain;
        if (Port.Length > 0) result += FString(L":") + Port;
        result += Path;
        if (Query.Length > 0) result += FString(L"?") + Query;
        return result;
    }

    void __URL_SetHost(URL* url, FString host) {
        url->Domain = host;
    }
}
