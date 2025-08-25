#include "ue.h"
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

namespace Unreal {
    FString::FString() : String(nullptr), Length(0), MaxSize(0) {}

    FString::FString(const char* Other) {
        Length = strlen(Other);
        MaxSize = Length + 1;
        AllocString();
        mbstowcs(String, Other, Length);
        String[Length] = 0;
    }

    FString::FString(const wchar_t* Other) {
        Length = wcslen(Other);
        MaxSize = Length + 1;
        AllocString();
        wcscpy(String, Other);
    }

    FString::FString(wchar_t* Other) {
        Length = wcslen(Other);
        MaxSize = Length + 1;
        AllocString();
        wcscpy(String, Other);
    }

    FString::FString(uint32_t len) : Length(len), MaxSize(len + 1) {
        AllocString();
        String[len] = 0;
    }

    FString FString::operator+(FString other) {
        FString result(Length + other.Length);
        wcscpy(result.String, String);
        wcscat(result.String, other.String);
        return result;
    }

    void FString::operator+=(FString other) {
        if (Length + other.Length + 1 > MaxSize) {
            MaxSize = Length + other.Length + 1;
            String = (wchar_t*)realloc(String, MaxSize * sizeof(wchar_t));
        }
        wcscat(String, other.String);
        Length += other.Length;
    }

    FString FString::substr(size_t off, size_t count) {
        if (off == npos) return FString();
        if (count == npos) count = Length - off;
        FString result(count);
        wcsncpy(result.String, String + off, count);
        result.String[count] = 0;
        return result;
    }

    size_t FString::find(wchar_t c) {
        for (uint32_t i = 0; i < Length; i++) {
            if (String[i] == c) return i;
        }
        return npos;
    }

    size_t FString::find(char c) {
        return find((wchar_t)c);
    }

    size_t FString::find(const wchar_t* c) {
        wchar_t* pos = wcsstr(String, c);
        if (pos) return pos - String;
        return npos;
    }

    bool FString::contains(wchar_t c) {
        return find(c) != npos;
    }

    bool FString::contains(const wchar_t* c) {
        return find(c) != npos;
    }

    bool FString::starts_with(const wchar_t* c) {
        return wcsncmp(String, c, wcslen(c)) == 0;
    }

    bool FString::ends_with(const wchar_t* c) {
        size_t len = wcslen(c);
        if (len > Length) return false;
        return wcsncmp(String + Length - len, c, len) == 0;
    }

    size_t FString::find_first_of(char c) {
        return find(c);
    }

    size_t FString::find_first_of(wchar_t c) {
        return find(c);
    }

    wchar_t* FString::c_str() {
        return String;
    }

    FString::operator wchar_t* () {
        return String;
    }

    void FString::Dealloc() {
        if (String) free(String);
        String = nullptr;
        Length = 0;
        MaxSize = 0;
    }

    void FString::AllocString() {
        String = (wchar_t*)malloc(MaxSize * sizeof(wchar_t));
    }
}
