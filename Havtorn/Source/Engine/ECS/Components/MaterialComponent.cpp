// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponent.h"

namespace Havtorn
{
    void SMaterialComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(EntityOwner, toData, pointerPosition);
        SerializeData(static_cast<U32>(AssetRegistryKeys.size()), toData, pointerPosition);
        SerializeData(AssetRegistryKeys, toData, pointerPosition);
    }

    void SMaterialComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(EntityOwner, fromData, pointerPosition);
        U32 numberOfAssetRegistryKeys = 0;
        DeserializeData(numberOfAssetRegistryKeys, fromData, pointerPosition);
        DeserializeData(AssetRegistryKeys, fromData, numberOfAssetRegistryKeys, pointerPosition);
    }

    U32 SMaterialComponent::GetSize() const
    {
        U32 size = 0;
        
        size += sizeof(SEntity);
        size += sizeof(U32);
        size += sizeof(U64) * static_cast<U32>(AssetRegistryKeys.size());

        return size;
    }
}
