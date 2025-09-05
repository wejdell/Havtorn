// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponent.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
    void SMaterialComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        U32 numberOfRefs = STATIC_U32(AssetReferences.size());
        SerializeData(numberOfRefs, toData, pointerPosition);
        for (const SAssetReference& assetRef : AssetReferences)
            assetRef.Serialize(toData, pointerPosition);
    }

    void SMaterialComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        U32 numberOfRefs = 0;
        DeserializeData(numberOfRefs, fromData, pointerPosition);
        for (U32 i = 0; i < numberOfRefs; i++)
        {
            SAssetReference& newRef = AssetReferences.emplace_back();
            newRef.Deserialize(fromData, pointerPosition);
        }
    }

    U32 SMaterialComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += GetDataSize(STATIC_U32(AssetReferences.size()));
        for (const SAssetReference& assetRef : AssetReferences)
            size += assetRef.GetSize();
        return size;
    }

    void SMaterialComponent::IsDeleted(CScene* /*fromScene*/)
    {
        for (const SAssetReference& assetRef : AssetReferences)
            GEngine::GetAssetRegistry()->UnrequestAsset(assetRef, Owner.GUID);
    }
}
