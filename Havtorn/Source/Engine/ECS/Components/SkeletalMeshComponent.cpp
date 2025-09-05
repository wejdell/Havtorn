// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SkeletalMeshComponent.h"
#include "Assets/FileHeaderDeclarations.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
    void SSkeletalMeshComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        AssetReference.Serialize(toData, pointerPosition);
    }

    void SSkeletalMeshComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        AssetReference.Deserialize(fromData, pointerPosition);
    }

    U32 SSkeletalMeshComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += AssetReference.GetSize();

        return size;
    }

    void SSkeletalMeshComponent::IsDeleted(CScene* /*fromScene*/)
    {
        GEngine::GetAssetRegistry()->UnrequestAsset(AssetReference, Owner.GUID);
    }
}
