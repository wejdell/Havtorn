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
        SerializeData(Owner, toData, pointerPosition);
        SerializeData(AssetRegistryKeys, toData, pointerPosition);
    }

    void SDecalComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        // NR: Texture info is saved and loaded using AssetRegistry
        DeserializeData(ShouldRenderAlbedo, fromData, pointerPosition);
        DeserializeData(ShouldRenderMaterial, fromData, pointerPosition);
        DeserializeData(ShouldRenderNormal, fromData, pointerPosition);
        DeserializeData(Owner, fromData, pointerPosition);
        DeserializeData(AssetRegistryKeys, fromData, pointerPosition);
    }

    U32 SDecalComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(ShouldRenderAlbedo);
        size += GetDataSize(ShouldRenderMaterial);
        size += GetDataSize(ShouldRenderNormal);
        size += GetDataSize(Owner);
        size += GetDataSize(AssetRegistryKeys);
        return size;
    }
}
