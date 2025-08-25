#include "request.h"
#include "base.h"
#include "hooking.h"
#include "url.h"
#include "redirection.h"
#include "opts.h"
#include <array>
#include <android/log.h>
#include <cstdlib>

#define LOG_TAG "Starfall"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

extern bool InternalCheckBytes(void* func, int offset, uint8_t* pattern, int len, bool b);
extern struct pf_patch_t pf_construct_patch_sig(const char* sig, void* cb);
extern struct pf_patchset_t pf_construct_patchset(struct pf_patch_t* patches, int n);
extern void pf_patchset_emit(void* buf, size_t size, struct pf_patchset_t patchset);
extern void* tbuf;
extern size_t tsize;
extern bool FixMemLeak;
namespace Hooking { extern void* Ret0Callback; extern void VTHook(void** vt, void* newfn, void** oldfn); }
extern bool UseBackendParam;
extern FString Backend;
extern void __URL_SetHost(void* url, const FString& host);

namespace Unreal {
    struct FCurlHttpRequest {
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
        FCurlHttpRequest()
            : VTable(nullptr), CurlHandle(nullptr), VerbIdx(-1), Response(nullptr), Status(EHttpRequestStatus::NotStarted), ElapsedTime(0.0f) {}
    };

    int FCurlHttpRequest::SetURLIdx = 0;
    void** FCurlHttpRequest::ProcessRequestVT = nullptr;

    inline bool FCurlHttpRequest::ProcessRequest() {
        if (VTable && FCurlHttpRequest::ProcessRequestVT) {
            using ProcFn = bool(*)(FCurlHttpRequest*);
            int idx = FCurlHttpRequest::SetURLIdx > 0 ? FCurlHttpRequest::SetURLIdx : 0;
            ProcFn fn = nullptr;
            if (FCurlHttpRequest::ProcessRequestVT[idx]) fn = (ProcFn)FCurlHttpRequest::ProcessRequestVT[idx];
            if (fn) return fn(this);
        }
        return false;
    }

    inline FString FCurlHttpRequest::GetURL() const { return URL; }
    inline FString FCurlHttpRequest::GetURLParameter(const FString& ParameterName) const {
        FString result = TEXT("");
        int32 pos = URL.Find(TEXT("?"));
        if (pos == INDEX_NONE) return result;
        FString query = URL.Mid(pos + 1);
        TArray<FString> pairs;
        query.ParseIntoArray(pairs, TEXT("&"), true);
        for (const auto& p : pairs) {
            FString key, val;
            if (p.Split(TEXT("="), &key, &val)) {
                if (key == ParameterName) return val;
            }
        }
        return result;
    }
    inline FString FCurlHttpRequest::GetHeader(const FString& HeaderName) const {
        const FString* found = Headers.Find(HeaderName);
        return found ? *found : FString();
    }
    inline TArray<FString> FCurlHttpRequest::GetAllHeaders() const {
        TArray<FString> out;
        for (const auto& Pair : Headers) {
            out.Add(Pair.Key + TEXT(": ") + Pair.Value);
        }
        return out;
    }
    inline FString FCurlHttpRequest::GetContentType() const {
        const FString* found = Headers.Find(TEXT("Content-Type"));
        return found ? *found : FString();
    }
    inline int32 FCurlHttpRequest::GetContentLength() const {
        return Payload.Num();
    }
    inline const TArray<uint8>& FCurlHttpRequest::GetContent() const { return Payload; }
    inline FString FCurlHttpRequest::GetVerb() const { return Verb; }
    inline void FCurlHttpRequest::SetVerb(const FString& InVerb) { Verb = InVerb; }
    inline void FCurlHttpRequest::SetURL(const FString& InURL) { URL = InURL; }
    inline void FCurlHttpRequest::SetContent(const TArray<uint8>& ContentPayload) { Payload = ContentPayload; }
    inline void FCurlHttpRequest::SetContentAsString(const FString& ContentString) {
        Payload.Empty();
        const ANSICHAR* s = TCHAR_TO_ANSI(*ContentString);
        int32 len = ContentString.Len();
        Payload.Append((const uint8*)s, len);
    }
    inline void FCurlHttpRequest::SetHeader(const FString& HeaderName, const FString& HeaderValue) {
        Headers.Add(HeaderName, HeaderValue);
    }
    inline void FCurlHttpRequest::AppendToHeader(const FString& HeaderName, const FString& AdditionalHeaderValue) {
        FString* cur = Headers.Find(HeaderName);
        if (cur) {
            *cur = *cur + TEXT(", ") + AdditionalHeaderValue;
        } else {
            Headers.Add(HeaderName, AdditionalHeaderValue);
        }
    }
    inline void FCurlHttpRequest::CancelRequest() {
        Status = EHttpRequestStatus::Failed;
    }
    inline EHttpRequestStatus::Type FCurlHttpRequest::GetStatus() const { return Status; }
    inline const FHttpResponsePtr FCurlHttpRequest::GetResponse() const { return Response; }
    inline void FCurlHttpRequest::Tick(float DeltaSeconds) { ElapsedTime += DeltaSeconds; }
    inline float FCurlHttpRequest::GetElapsedTime() const { return ElapsedTime; }
    inline void FCurlHttpRequest::OnRequestComplete(FHttpRequestCompleteDelegate Delegate) { OnRequestCompleteDelegate = Delegate; }
}

namespace Starfall {
    bool setupMemLeak = false;
    FString backend;
    namespace Hooks {
        bool (*ProcessRequestOG)(Unreal::FCurlHttpRequest* Request) = nullptr;
        bool (*EOSProcessRequestOG)(Unreal::FCurlHttpRequest* Request) = nullptr;

        bool InternalProcessRequest(Unreal::FCurlHttpRequest* Request, decltype(ProcessRequestOG) OG) {
            if (Unreal::FCurlHttpRequest::SetURLIdx == 0) {
                void* GetFunc = *Request->VTable;
                uint32_t URLOffset = 0;
                for (int i = 0; i < 100; i++) {
                    if (InternalCheckBytes(GetFunc, i, (uint8_t[]){0x48, 0x8D, 0x91}, 3, false)) {
                        URLOffset = *(uint32_t*)((uintptr_t)GetFunc + i + 3);
                        break;
                    }
                }
                if (URLOffset != 0) {
                    for (int64_t i = 0; i < ((uintptr_t)Unreal::FCurlHttpRequest::ProcessRequestVT - (uintptr_t)Request->VTable) / 8; i++) {
                        auto func = Request->VTable[i];
                        for (int j = 0; j < 100; j++) {
                            if (InternalCheckBytes(func, j, (uint8_t[]){0x48, 0x81, 0xC1}, 3, false)) {
                                if (*(uint32_t*)((uintptr_t)func + j + 3) == URLOffset) {
                                    Unreal::FCurlHttpRequest::SetURLIdx = i;
                                    break;
                                }
                            }
                        }
                        if (Unreal::FCurlHttpRequest::SetURLIdx != 0) break;
                    }
                }
                if (Unreal::FCurlHttpRequest::SetURLIdx == 0) Unreal::FCurlHttpRequest::SetURLIdx = 10;
            }

            if (!setupMemLeak && FixMemLeak) {
                static struct pf_patch_t ml_patch = pf_construct_patch_sig("48 8B 01 4C 8D 41 08 48 FF 60 20", Hooking::Ret0Callback);
                static struct pf_patch_t ml_patch2 = pf_construct_patch_sig("48 89 5C 24 ?? 57 48 83 EC ?? 48 8B 01 4C 8B C2 48 8D 54 24", Hooking::Ret0Callback);
                static struct pf_patch_t patches[] = { ml_patch, ml_patch2 };
                static struct pf_patchset_t patchset = pf_construct_patchset(patches, 2);
                pf_patchset_emit(tbuf, tsize, patchset);
                setupMemLeak = true;
            }

            auto urlS = Request->GetURL();
            auto url = (URL*)malloc(sizeof(URL));
            if (!url) return false;
            url->Construct(urlS);

            if (shouldRedirect(url)) {
                if (UseBackendParam) {
                    url->SetHost(backend);
                } else {
                    __URL_SetHost(url, Backend);
                }

                FString str = *url;
                int idx = (OG == EOSProcessRequestOG) ? 10 : Unreal::FCurlHttpRequest::SetURLIdx;
                ((void (*)(Unreal::FCurlHttpRequest*, FString))Request->VTable[idx])(Request, str);
                free(str.String);

                if (UseBackendParam) url->Dealloc(); else url->DeallocPathQuery();
            } else {
                url->Dealloc();
            }

            return OG ? OG(Request) : false;
        }

        bool ProcessRequestHook(Unreal::FCurlHttpRequest* Request) { return InternalProcessRequest(Request, ProcessRequestOG); }
        bool EOSProcessRequestHook(Unreal::FCurlHttpRequest* Request) { return InternalProcessRequest(Request, EOSProcessRequestOG); }
    }

    namespace Callbacks {
        bool PtrCallback(struct pf_patch_t* patch, void* stream) {
            Unreal::FCurlHttpRequest::ProcessRequestVT = (void**)stream;
            Hooking::VTHook((void**)stream, (void*)Hooks::ProcessRequestHook, (void**)&Hooks::ProcessRequestOG);
            return true;
        }

        bool EOSPtrCallback(struct pf_patch_t* patch, void* stream) {
            Hooking::VTHook((void**)stream, (void*)Hooks::EOSProcessRequestHook, (void**)&Hooks::EOSProcessRequestOG);
            return true;
        }
    }
}

int64_t curl_global_init(int64_t arg1) {
    int32_t w1 = 1;
    return curl_global_init_mem((int)arg1, w1);
}

void curl_global_cleanup() {
    int64_t base = 0x0968b000;
    int32_t &counter = *(int32_t *)(base + 0x700);
    if (counter == 0) return;
    counter--;
    if (counter != 0) return;
    Curl_global_host_cache_dtor();
    int64_t x19 = base + 0x700;
    int32_t w8 = *(int8_t *)(x19 + 8);
    if ((w8 & 1) != 0) {
        Curl_ssl_cleanup();
    }
    Curl_resolver_global_cleanup();
    *(int64_t *)(x19 + 8) = 0;
}

int64_t curl_easy_init() {
    int64_t flagptr = *(int64_t *)(0x08b71000 + 0xbe0);
    int64_t x8 = 0x0968b000 + 0x700;
    int32_t w8 = *(int32_t *)x8;
    if (w8 == 0) {
        int32_t w0 = curl_global_init_mem(3, 1);
        if (w0 != 0) return 0;
    }
    int64_t x0 = Curl_open();
    if (x0 == 0) return 0;
    return x0;
}

int64_t curl_easy_setopt(int64_t arg1, int64_t arg2, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t) {
    if (arg1 == 0) return 0x2b;
    return Curl_setopt(arg1, arg2);
}

int64_t curl_easy_perform(int64_t arg1) {
    if (arg1 == 0) return 1;
    int64_t x8 = *(int64_t *)arg1;
    if (x8 != 0xab1eb0000) return 1;
    return Curl_easy_perform(arg1);
}

int64_t curl_easy_getinfo(int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t) {
    return 0;
}

void curl_easy_cleanup(int64_t arg1) {
    if (arg1) {
        Curl_easy_cleanup_internal(arg1);
    }
}

int64_t curl_slist_append(int64_t arg1, int64_t arg2) {
    int64_t alloc_fn_ptr_loc = 0x08b75000 + 0xff0;
    auto AllocFn = *(int64_t*)alloc_fn_ptr_loc;
    if (!AllocFn) return 0;
    int64_t copied = ((int64_t(*)(int64_t))AllocFn)(arg2);
    if (!copied) return 0;
    int64_t newnode = ((int64_t(*)(size_t))AllocFn)(0x10);
    if (!newnode) {
        ((void(*)(int64_t))AllocFn)(copied);
        return 0;
    }
    *(int64_t *)newnode = copied;
    *(int64_t *)(newnode + 8) = 0;
    if (arg1 == 0) return newnode;
    int64_t cur = arg1;
    while (*(int64_t *)(cur + 8) != 0) cur = *(int64_t *)(cur + 8);
    *(int64_t *)(cur + 8) = newnode;
    return arg1;
}

void curl_slist_free_all(int64_t arg1) {
    int64_t cur = arg1;
    if (!cur) return;
    int64_t alloc_fn_ptr_loc = 0x08b75000 + 0xff0;
    auto FreeFn = *(int64_t*)alloc_fn_ptr_loc;
    while (cur) {
        int64_t data = *(int64_t *)cur;
        int64_t next = *(int64_t *)(cur + 8);
        if (data && FreeFn) ((void(*)(int64_t))FreeFn)(data);
        if (FreeFn) ((void(*)(int64_t))FreeFn)(cur);
        cur = next;
    }
}

int64_t curl_version_info(int64_t) { return 0; }

int64_t curl_easy_strerror(int64_t arg1) {
    int64_t x8 = 0x071e8000 + 0xabe;
    int64_t x9 = 0x071fd000 + 0x3c0;
    return arg1 == 0 ? x9 : x8;
}

int64_t curl_multi_init() { return 0; }
int64_t curl_multi_add_handle(int64_t, int64_t) { return 0; }
int64_t curl_multi_perform(int64_t, int64_t) { return 0; }

int64_t curl_multi_info_read(int64_t arg1, int64_t arg2) {
    int64_t out_count_ptr = arg2;
    int64_t multi = arg1;
    *(int32_t *)(out_count_ptr) = 0;
    if (!multi) return 0;
    int64_t magic = *(int64_t *)(multi);
    if (magic != 0xab1eb0000) return 0;
    int64_t list = *(int64_t *)(multi + 0x20);
    size_t count = Curl_llist_count(list);
    if (count == 0) return 0;
    int64_t first_node = *(int64_t *)(list);
    int64_t result = *(int64_t *)(first_node);
    Curl_llist_remove(list, first_node, 0);
    count = Curl_llist_count(list);
    int32_t w0 = curlx_uztosi(count);
    *(int32_t *)(out_count_ptr) = w0;
    return result;
}

int64_t curl_multi_cleanup(int64_t) { return 0; }
int64_t curl_multi_remove_handle(int64_t, int64_t) { return 0; }

int64_t curl_multi_fdset(int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5) {
    if (arg1 == 0) return 1;
    int64_t header = *(int64_t *)arg1;
    if (header != 0xab1eb0000) return 1;
    int64_t x22 = *(int64_t *)(arg1 + 8);
    if (x22 == 0) {
        *(int32_t *)arg5 = -1;
        return 0;
    }
    int w24 = -1;
    int64_t cur = x22;
    while (cur) {
        int32_t w0 = Curl_multi_fdset_internal(cur, (int64_t)(arg2));
        cur = *(int64_t *)cur;
    }
    *(int32_t *)arg5 = w24;
    return 0;
}

int64_t curl_multi_socket_all(int64_t arg1, int64_t arg2) {
    int32_t rc = Curl_multi_socket_action_internal(arg1, 1, -1, 0, arg2);
    if (rc > 0) return rc;
    Curl_multi_update_timer(arg1);
    return rc;
}

int64_t curl_multi_socket_action(int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4) {
    int rc = Curl_multi_socket_action_internal(arg1, 0, (int)arg2, (int)arg3, arg4);
    if (rc > 0) return rc;
    Curl_multi_update_timer(arg1);
    return rc;
}

extern "C" bool shouldRedirect(void* url) {
    if (!url) return false;
    FString hostGuess = FString();
    bool found = false;
    try {
        FString maybe = *(FString*)url;
        if (maybe.Len() > 0) {
            if (maybe.Find(TEXT("ol.epicgames.com")) != INDEX_NONE) {
                return true;
            }
        }
    } catch (...) {}
    return false;
}

__attribute__((constructor)) static void __initialize_request_redirection() {
    UseBackendParam = true;
    Backend = FString(TEXT("127.0.0.1:3551"));
    FixMemLeak = true;
    Starfall::backend = FString(TEXT("127.0.0.1:3551"));
    if (Unreal::FCurlHttpRequest::ProcessRequestVT) {
        Hooking::VTHook(Unreal::FCurlHttpRequest::ProcessRequestVT, (void*)Starfall::Hooks::ProcessRequestHook, (void**)&Starfall::Hooks::ProcessRequestOG);
    }
}
