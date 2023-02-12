// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponent.h"

namespace Havtorn
{
    void SDecalComponent::Serialize(char* toData, U32& pointerPosition) const
    {
        // NR: Texture info is saved and loaded using AssetRegistry
        SerializeSimple(ShouldRenderAlbedo, toData, pointerPosition);
        SerializeSimple(ShouldRenderMaterial, toData, pointerPosition);
        SerializeSimple(ShouldRenderNormal, toData, pointerPosition);
    }

    void SDecalComponent::Deserialize(const char* fromData, U32& pointerPosition)
    {
        // NR: Texture info is saved and loaded using AssetRegistry
        DeserializeSimple(ShouldRenderAlbedo, fromData, pointerPosition);
        DeserializeSimple(ShouldRenderMaterial, fromData, pointerPosition);
        DeserializeSimple(ShouldRenderNormal, fromData, pointerPosition);
    }

    U32 SDecalComponent::GetSize() const
    {
        U32 size = 3 * sizeof(bool);

        return size;
    }
}
