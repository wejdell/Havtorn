// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponent.h"
#include "Engine.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
    void SDecalComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        AssetReference.Serialize(toData, pointerPosition);
    }

    void SDecalComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        AssetReference.Deserialize(fromData, pointerPosition);
    }

    U32 SDecalComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += AssetReference.GetSize();
        return size;
    }

    void SDecalComponent::IsDeleted(CScene* /*fromScene*/)
    {
        GEngine::GetAssetRegistry()->UnrequestAsset(AssetReference, Owner.GUID);
    }
}
