#pragma once
#include "pch.h"
#include "base.h"
#include "url.h"
#include "redirection.h"
#include "opts.h"

#define CallVirt(T, vt, offset, ...) ((T) vt[offset])(__VA_ARGS__)

namespace Unreal {

    class FCurlHttpRequest {
    public:
        void** VTable;
        static int SetURLIdx;
        static void** ProcessRequestVT;

        void* CurlHandle;
        FString URL;
        FString Verb;
        int VerbIdx;
        TMap<FString, FString> Headers;
        TArray<uint8> Payload;
        FHttpResponsePtr Response;
        EHttpRequestStatus::Type Status;
        FHttpRequestCompleteDelegate OnRequestCompleteDelegate;
        float ElapsedTime;

        FCurlHttpRequest();

        bool ProcessRequest();
        FString GetURL() const;
        FString GetURLParameter(const FString& ParameterName) const;
        FString GetHeader(const FString& HeaderName) const;
        TArray<FString> GetAllHeaders() const;
        FString GetContentType() const;
        int32 GetContentLength() const;
        const TArray<uint8>& GetContent() const;
        FString GetVerb() const;
        void SetVerb(const FString& InVerb);
        void SetURL(const FString& InURL);
        void SetContent(const TArray<uint8>& ContentPayload);
        void SetContentAsString(const FString& ContentString);
        void SetHeader(const FString& HeaderName, const FString& HeaderValue);
        void AppendToHeader(const FString& HeaderName, const FString& AdditionalHeaderValue);
        void CancelRequest();
        EHttpRequestStatus::Type GetStatus() const;
        const FHttpResponsePtr GetResponse() const;
        void Tick(float DeltaSeconds);
        float GetElapsedTime() const;
        void OnRequestComplete(FHttpRequestCompleteDelegate Delegate);
    };

} // namespace Unreal

namespace Starfall {
    extern FString backend;
    void SetupRequest(Unreal::FCurlHttpRequest* Request);

    namespace Hooks {
        bool ProcessRequestHook(Unreal::FCurlHttpRequest* Request);
        bool EOSProcessRequestHook(Unreal::FCurlHttpRequest* Request);
    }

    namespace Callbacks {
        bool PtrCallback(struct pf_patch_t* patch, void* stream);
        bool EOSPtrCallback(struct pf_patch_t* patch, void* stream);
    }

    namespace Finders {
        void FindProcessRequest();
    }
}
