// pch.h
#pragma once
#include <cstdint>
#include <array>
#include <string_view>
#include <algorithm>

struct pf_patch_t {
    const char* sig;
    bool (*callback)(struct pf_patch_t* patch, void* stream);
};

struct pf_patchset_t {
    pf_patch_t* patches;
    size_t count;
};

pf_patch_t pf_construct_patch_sig(const char* sig, bool (*callback)(pf_patch_t*, void*));
pf_patchset_t pf_construct_patchset(pf_patch_t* patches, size_t count);
void pf_patchset_emit(void* buf, size_t size, pf_patchset_t patchset);

template <size_t _Sz>
struct ConstexprString {
    char _St[_Sz];

public:
    consteval ConstexprString(const char (&_Ps)[_Sz]) {
        std::copy_n(_Ps, _Sz, _St);
    }

    operator const char* () const {
        return _St;
    }

    constexpr std::string_view StringView() const {
        return _St;
    }

    constexpr int PatternCount() const {
        int c = 0;
        for (int i = 0; i < _Sz; i++) {
            if (_St[i] == ' ') c++;
        }
        return c + 1;
    }
};

template <typename _Ft>
struct ConstexprFunc {
    _Ft _Fn;

public:
    consteval ConstexprFunc(_Ft _Pf) : _Fn(_Pf) {}

    constexpr _Ft Get() const {
        return _Fn;
    }
};

template <size_t _Sz>
struct ConstexprArray {
    uint8_t _Ar[_Sz];
public:
    consteval ConstexprArray(std::array<uint8_t, _Sz> _Pa) {
        std::copy_n(_Pa.data(), _Sz, _Ar);
    }

    constexpr void* Get() const {
        return (void*)_Ar;
    }
};
