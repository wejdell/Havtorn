// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponent.h"

namespace Havtorn
{
    U32 SDecalComponent::Serialize(char* toData, U32& bufferPosition)
    {
        bufferPosition += SerializeSimple(static_cast<U32>(TextureReferences.size()), toData, bufferPosition);
        bufferPosition += SerializeVector(TextureReferences, toData, bufferPosition);
        bufferPosition += SerializeSimple(ShouldRenderAlbedo, toData, bufferPosition);
        bufferPosition += SerializeSimple(ShouldRenderMaterial, toData, bufferPosition);
        bufferPosition += SerializeSimple(ShouldRenderNormal, toData, bufferPosition);

        return bufferPosition;
    }

    U32 SDecalComponent::Deserialize(const char* fromData, U32& bufferPosition)
    {
        U32 textureNumber = 0;
        bufferPosition += DeserializeSimple(textureNumber, fromData, bufferPosition);
        bufferPosition += DeserializeVector(TextureReferences, fromData, textureNumber, bufferPosition);
        bufferPosition += DeserializeSimple(ShouldRenderAlbedo, fromData, bufferPosition);
        bufferPosition += DeserializeSimple(ShouldRenderMaterial, fromData, bufferPosition);
        bufferPosition += DeserializeSimple(ShouldRenderNormal, fromData, bufferPosition);

        return bufferPosition;
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
