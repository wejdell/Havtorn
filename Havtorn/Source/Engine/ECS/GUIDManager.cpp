// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GUIDManager.h"

namespace Havtorn
{
    U64 Havtorn::UGUIDManager::Generate()
    {
        GUID newID;
        ENGINE_HR_MESSAGE(CoCreateGuid(&newID), "Could not create new GUID using CoCreateGuid!");

        const unsigned char bytes[8] =
        {
            (newID.Data1 >> 24) & 0xFF,
            (newID.Data1 >> 16) & 0xFF,
            (newID.Data1 >> 8) & 0xFF,
            (newID.Data1) & 0xFF,

            static_cast<unsigned char>((newID.Data2 >> 8) & 0xFF),
            static_cast<unsigned char>((newID.Data2) & 0xFF),

            static_cast<unsigned char>((newID.Data3 >> 8) & 0xFF),
            static_cast<unsigned char>((newID.Data3) & 0xFF)
        };

        U64 guid = 0;
        memcpy(&guid, &bytes[0], sizeof(U64));
        return guid;
    }
}
