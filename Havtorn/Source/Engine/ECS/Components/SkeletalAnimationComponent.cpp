// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "SkeletalAnimationComponent.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
    void SSkeletalAnimationComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        U32 numberOfRefs = STATIC_U32(AssetReferences.size());
        SerializeData(numberOfRefs, toData, pointerPosition);
        for (const SAssetReference& assetRef : AssetReferences)
            assetRef.Serialize(toData, pointerPosition);
    }

    void SSkeletalAnimationComponent::Deserialize(const char* fromData, U64& pointerPosition)
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

    U32 SSkeletalAnimationComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += GetDataSize(STATIC_U32(AssetReferences.size()));
        for (const SAssetReference& assetRef : AssetReferences)
            size += assetRef.GetSize();
        return size;
    }

    void SSkeletalAnimationComponent::IsDeleted(CScene* /*fromScene*/)
    {
        for (const SAssetReference& assetRef : AssetReferences)
            GEngine::GetAssetRegistry()->UnrequestAsset(assetRef, Owner.GUID);
    }
}
