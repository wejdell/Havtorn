// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "StaticMeshComponent.h"
#include "FileSystem\FileHeaderDeclarations.h"

namespace Havtorn
{
    U32 SStaticMeshComponent::Serialize(char* toData, U32& bufferPosition)
    {
        bufferPosition += SerializeSimple(static_cast<U32>(DrawCallData.size()), toData, bufferPosition);
        bufferPosition += SerializeVector(DrawCallData, toData, bufferPosition);
        bufferPosition += SerializeSimple(static_cast<U32>(Name.length()), toData, bufferPosition);
        bufferPosition += SerializeString(Name, toData, bufferPosition);
        bufferPosition += SerializeSimple(NumberOfMaterials, toData, bufferPosition);
        bufferPosition += SerializeSimple(VertexShaderIndex, toData, bufferPosition);
        bufferPosition += SerializeSimple(InputLayoutIndex, toData, bufferPosition);
        bufferPosition += SerializeSimple(PixelShaderIndex, toData, bufferPosition);
        bufferPosition += SerializeSimple(SamplerIndex, toData, bufferPosition);
        bufferPosition += SerializeSimple(TopologyIndex, toData, bufferPosition);
        return bufferPosition;
    }

    U32 SStaticMeshComponent::Deserialize(const char* fromData, U32& bufferPosition)
    {
        U32 meshNumber = 0;
        bufferPosition += DeserializeSimple(meshNumber, fromData, bufferPosition);
        bufferPosition += DeserializeVector(DrawCallData, fromData, meshNumber, bufferPosition);
        U32 meshNameLength = 0;
        bufferPosition += DeserializeSimple(meshNameLength, fromData, bufferPosition);
        bufferPosition += DeserializeString(Name, fromData, meshNameLength, bufferPosition);
        bufferPosition += DeserializeSimple(NumberOfMaterials, fromData, bufferPosition);
        bufferPosition += DeserializeSimple(VertexShaderIndex, fromData, bufferPosition);
        bufferPosition += DeserializeSimple(InputLayoutIndex, fromData, bufferPosition);
        bufferPosition += DeserializeSimple(PixelShaderIndex, fromData, bufferPosition);
        bufferPosition += DeserializeSimple(SamplerIndex, fromData, bufferPosition);
        bufferPosition += DeserializeSimple(TopologyIndex, fromData, bufferPosition);
        return bufferPosition;
    }

    U32 SStaticMeshComponent::GetSize() const
    {
        U32 size = 0;
        size += sizeof(U32);
        size += static_cast<U32>(DrawCallData.size()) * sizeof(SDrawCallData);
        size += sizeof(U32);
        size += static_cast<U32>(Name.length()) * sizeof(char);
        size += 6 * sizeof(U8);

        return size;
    }
}
