// Copyright 2022 Team Havtorn. All Rights Reserved.
#pragma once

#include "Core/Core.h"

namespace Havtorn
{
    class HAVTORN_API CHavtornString
    {
    public:
        CHavtornString(const std::string& name)
        {
            U32 size = static_cast<U32>(name.length());
            HV_ASSERT(size != 0, "Trying to create HavtornString from std::string with size 0!");

            Characters = new char[size + 1];
            memcpy(Characters, &name[0], sizeof(char) * size);
            Characters[size] = '\0';
            Size = size;
        }

        inline const U32 Length() const { return Size; }
        inline const std::string AsString() const { return std::string(Characters); }
        inline const char* c_str() const { return Characters; }
        inline const char* ConstChar() const { return Characters; }
        bool Contains(const char* str) const;

        ~CHavtornString()
        {
            delete[] Characters;
            Characters = nullptr;
        }

    private:
        [[nodiscard]] std::array<U16, 256> CharacterTableForComparison(const char* pattern) const;
        bool Same(const char* str1, const char* str2, const U16 count) const;

    private:
        char* Characters = nullptr;
        U32 Size = 0;
    };
}
