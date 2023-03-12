// Copyright 2022 Team Havtorn. All Rights Reserved.
#pragma once

#include "Core/Core.h"
#include <array>

namespace Havtorn
{
    class HAVTORN_API CHavtornString
    {
    public:
        CHavtornString(const std::string& name)
        {
            U64 size = name.length() + 1;
            HV_ASSERT(size != 0, "Trying to create HavtornString from std::string with size 0!");

            Characters = new char[size];
            memcpy(Characters, &name[0], sizeof(char) * size);
            Characters[size] = '\0';
            Size = static_cast<U32>(size);
        }

        ~CHavtornString()
        {
            SAFE_ARRAY_DELETE(Characters);
        }

        inline const U32 Length() const { return Size; }
        inline const std::string AsString() const { return std::string(Characters); }
        inline const char* c_str() const { return Characters; }
        inline const char* ConstChar() const { return Characters; }
        bool Contains(const char* str) const;

    private:
        [[nodiscard]] std::array<U16, 256> CharacterTableForComparison(const char* pattern) const;
        bool Same(const char* str1, const char* str2, const U16 count) const;

    private:
        char* Characters = nullptr;
        U32 Size = 0;
    };

    template<U8 maxSize>
    class CHavtornStaticString
    {
    public:
        CHavtornStaticString() = default;

        CHavtornStaticString(const std::string& name)
        {
            constexpr bool isNotOverflow = maxSize > 0;
            HV_ASSERT(isNotOverflow, "Trying to create a HavtornStaticString with size > 255!\nMax allowed size is 255, so that the entire size is 256 counting the Size property.");

            U64 size = name.length();
            HV_ASSERT(size != 0, "Trying to create a HavtornStaticString from an empty std::string!");
            HV_ASSERT(size <= maxSize, "Trying to create a HavtornStaticString from an std::string larger than the specified max size!");

            memcpy(&Characters[0], name.data(), sizeof(char) * size);

            Size = static_cast<U8>(size);
        }

        ~CHavtornStaticString()
        {
        }

        CHavtornStaticString(const CHavtornStaticString& other)
        {
            memcpy(&Characters[0], other.Characters.data(), sizeof(char) * other.Size);
            Size = static_cast<U8>(other.Size);
        }

        CHavtornStaticString(const CHavtornStaticString&& other)
        {
            Characters = std::move(other.Characters);
            Size = static_cast<U8>(other.Size);
        }

        CHavtornStaticString& operator=(const CHavtornStaticString& other)
        {
            memcpy(&Characters[0], other.Characters.data(), sizeof(char) * other.Size);
            Size = static_cast<U8>(other.Size);
            return *this;
        }

        CHavtornStaticString& operator=(const CHavtornStaticString&& other)
        {
            Characters = std::move(other.Characters);
            Size = static_cast<U8>(other.Size);
            return *this;
        }

        inline const U32 Length() const { return Size; }
        inline const std::string AsString() const { return std::string(std::begin(Characters), std::begin(Characters) + STATIC_U64(Size)); }
        char* Data() { return Characters.data(); }

        // NR: This should only be called when we know reliably what the new length should be. Used by HavtornInputTextResizeCallback
        void SetLength(U8 length) { HV_ASSERT(Size <= maxSize, "Trying to set size larger than max!"); Size = length; }

    private:
        std::array<char, maxSize> Characters = {};
        U8 Size = 0;
    };
}
