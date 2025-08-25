#include "pch.h"
#include <cstring>

pf_patch_t pf_construct_patch_sig(const char* sig, bool (*callback)(pf_patch_t*, void*)) {
    return { sig, callback };
}

pf_patchset_t pf_construct_patchset(pf_patch_t* patches, size_t count) {
    return { patches, count };
}

void pf_patchset_emit(void* buf, size_t size, pf_patchset_t patchset) {
    (void)buf;
    (void)size;
    (void)patchset;
}
