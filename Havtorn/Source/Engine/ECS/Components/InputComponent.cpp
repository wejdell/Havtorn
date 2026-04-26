// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "InputComponent.h"

#include "Engine.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
    void SInputComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        AssetReference.Serialize(toData, pointerPosition);
    }

    void SInputComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        AssetReference.Deserialize(fromData, pointerPosition);
    }

    U32 SInputComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += AssetReference.GetSize();

        return size;
    }

    void SInputComponent::IsDeleted(CScene* /*fromScene*/)
    {
        GEngine::GetAssetRegistry()->UnrequestAsset(AssetReference, Owner.GUID);
    }

}
