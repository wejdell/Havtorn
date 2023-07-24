// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "StaticMeshComponent.h"
#include "FileSystem\FileHeaderDeclarations.h"

namespace Havtorn
{
    void SStaticMeshComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeSimple(static_cast<U32>(DrawCallData.size()), toData, pointerPosition);
        SerializeVector(DrawCallData, toData, pointerPosition);
        // SerializeSimple(static_cast<U32>(Name.length()), toData, bufferPosition);
        // SerializeString(Name, toData, bufferPosition);
        SerializeSimple(Name, toData, pointerPosition);
        SerializeSimple(NumberOfMaterials, toData, pointerPosition);
        SerializeSimple(VertexShaderIndex, toData, pointerPosition);
        SerializeSimple(InputLayoutIndex, toData, pointerPosition);
        SerializeSimple(PixelShaderIndex, toData, pointerPosition);
        SerializeSimple(SamplerIndex, toData, pointerPosition);
        SerializeSimple(TopologyIndex, toData, pointerPosition);
    }

    void SStaticMeshComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        U32 meshNumber = 0;
        DeserializeSimple(meshNumber, fromData, pointerPosition);
        DeserializeVector(DrawCallData, fromData, meshNumber, pointerPosition);
        //U32 meshNameLength = 0;
        //bufferPosition += DeserializeSimple(meshNameLength, fromData, bufferPosition);
        //bufferPosition += DeserializeString(Name, fromData, meshNameLength, bufferPosition);
        DeserializeSimple(Name, fromData, pointerPosition);
        DeserializeSimple(NumberOfMaterials, fromData, pointerPosition);
        DeserializeSimple(VertexShaderIndex, fromData, pointerPosition);
        DeserializeSimple(InputLayoutIndex, fromData, pointerPosition);
        DeserializeSimple(PixelShaderIndex, fromData, pointerPosition);
        DeserializeSimple(SamplerIndex, fromData, pointerPosition);
        DeserializeSimple(TopologyIndex, fromData, pointerPosition);
    }

    U32 SStaticMeshComponent::GetSize() const
    {
        U32 size = 0;
        size += sizeof(U32);
        size += static_cast<U32>(DrawCallData.size()) * sizeof(SDrawCallData);
        //size += sizeof(U32);
        //size += static_cast<U32>(Name.length()) * sizeof(char);
        size += sizeof(Name);
        size += 6 * sizeof(U8);

        return size;
    }
}
