#pragma once
#include "pch.h"

namespace Unreal {
    class FString {
    public:
        wchar_t* String;
        uint32_t Length;
        uint32_t MaxSize;
        inline static const size_t npos = -1;

        FString();
        FString(const char* Other);
        FString(const wchar_t* Other);
        FString(wchar_t* Other);
        explicit FString(uint32_t len);

        FString operator+(FString other);
        void operator+=(FString other);

        FString substr(size_t off, size_t count = -1);

        size_t find(wchar_t c);
        size_t find(char c);
        size_t find(const wchar_t* c);

        bool contains(wchar_t c);
        bool contains(const wchar_t* c);

        bool starts_with(const wchar_t* c);
        bool ends_with(const wchar_t* c);

        size_t find_first_of(char c);
        size_t find_first_of(wchar_t c);

        wchar_t* c_str();

        operator wchar_t* ();

        void Dealloc();
    private:
        void AllocString();
    };
}
using namespace Unreal;
