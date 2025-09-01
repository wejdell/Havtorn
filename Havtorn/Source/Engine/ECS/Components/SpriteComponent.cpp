// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "SpriteComponent.h"
#include "Scene/AssetRegistry.h"

namespace Havtorn
{
    void SSpriteComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        SerializeData(Color, toData, pointerPosition);
        SerializeData(UVRect, toData, pointerPosition);
        AssetReference.Serialize(toData, pointerPosition);
    }

    void SSpriteComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        DeserializeData(Color, fromData, pointerPosition);
        DeserializeData(UVRect, fromData, pointerPosition);
        AssetReference.Deserialize(fromData, pointerPosition);
    }

    U32 SSpriteComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += GetDataSize(Color);
        size += GetDataSize(UVRect);
        size += AssetReference.GetSize();

        return size;
    }

    void SSpriteComponent::IsDeleted(CScene* /*fromScene*/)
    {
        GEngine::GetAssetRegistry()->UnrequestAsset(AssetReference, Owner.GUID);
    }
}
