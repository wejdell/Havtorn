// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "PrefabComponent.h"
#include "Engine.h"
#include "Assets/FileHeaderDeclarations.h"
#include "Assets/AssetRegistry.h"
#include "TransformComponent.h"

namespace Havtorn
{
    void SPrefabComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        AssetReference.Serialize(toData, pointerPosition);
    }

    void SPrefabComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
        AssetReference.Deserialize(fromData, pointerPosition);
    }

    U32 SPrefabComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += AssetReference.GetSize();

        return size;
    }

    void SPrefabComponent::IsDeleted(CScene* fromScene)
    {
        GEngine::GetAssetRegistry()->UnrequestAsset(AssetReference, Owner.GUID);

        STransformComponent* transformComponent = fromScene->GetComponent<STransformComponent>(Owner);
        if (!SComponent::IsValid(transformComponent))
            return;

        if (transformComponent->AttachedEntities.empty())
            return;

        std::vector<SEntity> childrenToRemove;
        fromScene->GetAttachedEntities(Owner, childrenToRemove);

        // NW: Pop out the included Owner that we get from GetAttachedEntities
        childrenToRemove.pop_back();

        for (const SEntity& child : childrenToRemove)
            fromScene->RemoveEntity(child);
    }
}
