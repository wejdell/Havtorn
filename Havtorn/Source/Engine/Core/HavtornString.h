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
            size_t size = name.length();
            HV_ASSERT(size != 0, "Trying to create HavtornString from std::string with size 0!");

            string = new char[size + 1];
            memcpy(string, &name[0], sizeof(char) * size);
            string[size] = '\0';
        }

        ~CHavtornString()
        {
            delete[] string;
            string = nullptr;
        }

        const std::string AsString() const { return std::string(string); }

        const char* c_str() const { return string; }

        const char* ConstChar() const { return string; }

    private:
        char* string = nullptr;
    };
}