// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "StaticMeshComponent.h"
#include "FileSystem\FileHeaderDeclarations.h"
#include "Scene/AssetRegistry.h"

namespace Havtorn
{
    void SStaticMeshComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        AssetReference.Serialize(toData, pointerPosition);
    }

    void SStaticMeshComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        AssetReference.Deserialize(fromData, pointerPosition);
    }

    U32 SStaticMeshComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += AssetReference.GetSize();

        return size;
    }

    void SStaticMeshComponent::IsDeleted(CScene* /*fromScene*/)
    {
        GEngine::GetAssetRegistry()->UnrequestAsset(AssetReference, Owner.GUID);
    }
}
