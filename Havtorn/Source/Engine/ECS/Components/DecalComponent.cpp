// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponent.h"

namespace Havtorn
{
    void SDecalComponent::Serialize(char* toData, U32& pointerPosition) const
    {
        SerializeSimple(static_cast<U32>(TextureReferences.size()), toData, pointerPosition);
        SerializeVector(TextureReferences, toData, pointerPosition);
        SerializeSimple(ShouldRenderAlbedo, toData, pointerPosition);
        SerializeSimple(ShouldRenderMaterial, toData, pointerPosition);
        SerializeSimple(ShouldRenderNormal, toData, pointerPosition);
    }

    void SDecalComponent::Deserialize(const char* fromData, U32& pointerPosition)
    {
        U32 textureNumber = 0;
        DeserializeSimple(textureNumber, fromData, pointerPosition);
        DeserializeVector(TextureReferences, fromData, textureNumber, pointerPosition);
        DeserializeSimple(ShouldRenderAlbedo, fromData, pointerPosition);
        DeserializeSimple(ShouldRenderMaterial, fromData, pointerPosition);
        DeserializeSimple(ShouldRenderNormal, fromData, pointerPosition);
    }

    U32 SDecalComponent::GetSize() const
    {
        U32 size = 0;
        
        size += sizeof(U32);
        size += static_cast<U32>(TextureReferences.size()) * sizeof(U16);
        size += 3 * sizeof(bool);

        return size;
    }
}
