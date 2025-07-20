// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ScriptComponent.h"
#include "HexRune/HexRune.h"

namespace Havtorn
{
    void SScriptComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        SerializeData(AssetRegistryKey, toData, pointerPosition);
    }

    void SScriptComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        DeserializeData(AssetRegistryKey, fromData, pointerPosition);
    }

    U32 SScriptComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += GetDataSize(AssetRegistryKey);

        return size;
    }
}
