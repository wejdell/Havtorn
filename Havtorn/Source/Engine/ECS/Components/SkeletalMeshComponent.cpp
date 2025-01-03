// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SkeletalMeshComponent.h"
#include "FileSystem\FileHeaderDeclarations.h"

namespace Havtorn
{
    void SSkeletalMeshComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        SerializeData(AssetRegistryKey, toData, pointerPosition);
    }

    void SSkeletalMeshComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        DeserializeData(AssetRegistryKey, fromData, pointerPosition);
    }

    U32 SSkeletalMeshComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += GetDataSize(AssetRegistryKey);

        return size;
    }
}
