// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "StaticMeshComponent.h"
#include "FileSystem\FileHeaderDeclarations.h"

namespace Havtorn
{
    void SStaticMeshComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        //SerializeData(static_cast<U32>(DrawCallData.size()), toData, pointerPosition);
        //SerializeData(DrawCallData, toData, pointerPosition);
        //// SerializeData(static_cast<U32>(Name.length()), toData, bufferPosition);
        //// SerializeData(Name, toData, bufferPosition);
        //SerializeData(Name, toData, pointerPosition);
        //SerializeData(NumberOfMaterials, toData, pointerPosition);
        //SerializeData(VertexShaderIndex, toData, pointerPosition);
        //SerializeData(InputLayoutIndex, toData, pointerPosition);
        //SerializeData(PixelShaderIndex, toData, pointerPosition);
        //SerializeData(SamplerIndex, toData, pointerPosition);
        //SerializeData(TopologyIndex, toData, pointerPosition);
        SerializeData(EntityOwner, toData, pointerPosition);
        SerializeData(AssetRegistryKey, toData, pointerPosition);

    }

    void SStaticMeshComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        //U32 meshNumber = 0;
        //DeserializeData(meshNumber, fromData, pointerPosition);
        //DeserializeData(DrawCallData, fromData, meshNumber, pointerPosition);
        ////U32 meshNameLength = 0;
        ////bufferPosition += DeserializeData(meshNameLength, fromData, bufferPosition);
        ////bufferPosition += DeserializeData(Name, fromData, meshNameLength, bufferPosition);
        //DeserializeData(Name, fromData, pointerPosition);
        //DeserializeData(NumberOfMaterials, fromData, pointerPosition);
        //DeserializeData(VertexShaderIndex, fromData, pointerPosition);
        //DeserializeData(InputLayoutIndex, fromData, pointerPosition);
        //DeserializeData(PixelShaderIndex, fromData, pointerPosition);
        //DeserializeData(SamplerIndex, fromData, pointerPosition);
        //DeserializeData(TopologyIndex, fromData, pointerPosition);
        DeserializeData(EntityOwner, fromData, pointerPosition);
        DeserializeData(AssetRegistryKey, fromData, pointerPosition);
    }

    U32 SStaticMeshComponent::GetSize() const
    {
        U32 size = 0;
        //size += sizeof(U32);
        //size += static_cast<U32>(DrawCallData.size()) * sizeof(SDrawCallData);
        ////size += sizeof(U32);
        ////size += static_cast<U32>(Name.length()) * sizeof(char);
        //size += sizeof(Name);
        //size += 6 * sizeof(U8);
        size += sizeof(SEntity);
        size += sizeof(U64);

        return size;
    }
}
