// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponent.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
    void SDecalComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        SerializeData(ShouldRenderAlbedo, toData, pointerPosition);
        SerializeData(ShouldRenderMaterial, toData, pointerPosition);
        SerializeData(ShouldRenderNormal, toData, pointerPosition);
        U32 numberOfRefs = STATIC_U32(AssetReferences.size());
        SerializeData(numberOfRefs, toData, pointerPosition);
        for (const SAssetReference& assetRef : AssetReferences)
            assetRef.Serialize(toData, pointerPosition);
    }

    void SDecalComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        DeserializeData(ShouldRenderAlbedo, fromData, pointerPosition);
        DeserializeData(ShouldRenderMaterial, fromData, pointerPosition);
        DeserializeData(ShouldRenderNormal, fromData, pointerPosition); 
        U32 numberOfRefs = 0;
        DeserializeData(numberOfRefs, fromData, pointerPosition);
        for (U32 i = 0; i < numberOfRefs; i++)
        {
            SAssetReference& newRef = AssetReferences.emplace_back();
            newRef.Deserialize(fromData, pointerPosition);
        }
    }

    U32 SDecalComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += GetDataSize(ShouldRenderAlbedo);
        size += GetDataSize(ShouldRenderMaterial);
        size += GetDataSize(ShouldRenderNormal);
        size += GetDataSize(STATIC_U32(AssetReferences.size()));
        for (const SAssetReference& assetRef : AssetReferences)
            size += assetRef.GetSize();
        return size;
    }

    void SDecalComponent::IsDeleted(CScene* /*fromScene*/)
    {
        for (const SAssetReference& assetRef : AssetReferences)
            GEngine::GetAssetRegistry()->UnrequestAsset(assetRef, Owner.GUID);
    }
}
