// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponent.h"

namespace Havtorn
{
    void SMaterialComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        SerializeData(AssetRegistryKeys, toData, pointerPosition);
    }

    void SMaterialComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        DeserializeData(AssetRegistryKeys, fromData, pointerPosition);
    }

    U32 SMaterialComponent::GetSize() const
    {
        U32 size = 0;
        
        size += GetDataSize(Owner);
        size += GetDataSize(AssetRegistryKeys);

        return size;
    }
}
