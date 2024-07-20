// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponent.h"

namespace Havtorn
{
    void SDecalComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        // NR: Texture info is saved and loaded using AssetRegistry
        SerializeData(ShouldRenderAlbedo, toData, pointerPosition);
        SerializeData(ShouldRenderMaterial, toData, pointerPosition);
        SerializeData(ShouldRenderNormal, toData, pointerPosition);
        SerializeData(EntityOwner, toData, pointerPosition);
        SerializeData(static_cast<U32>(AssetRegistryKeys.size()), toData, pointerPosition);
        SerializeData(AssetRegistryKeys, toData, pointerPosition);
    }

    void SDecalComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        // NR: Texture info is saved and loaded using AssetRegistry
        DeserializeData(ShouldRenderAlbedo, fromData, pointerPosition);
        DeserializeData(ShouldRenderMaterial, fromData, pointerPosition);
        DeserializeData(ShouldRenderNormal, fromData, pointerPosition);
        DeserializeData(EntityOwner, fromData, pointerPosition);
        U32 numberOfAssetRegistryKeys = 0;
        DeserializeData(numberOfAssetRegistryKeys, fromData, pointerPosition);
        DeserializeData(AssetRegistryKeys, fromData, numberOfAssetRegistryKeys, pointerPosition);
    }

    U32 SDecalComponent::GetSize() const
    {
        U32 size = 3 * sizeof(bool);
        size += sizeof(SEntity);
        size += sizeof(U32);
        size += sizeof(U64) * static_cast<U32>(AssetRegistryKeys.size());
        return size;
    }
}
