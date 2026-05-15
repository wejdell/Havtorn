// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AudioEmitterComponent.h"
#include "Engine.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
    SAudioEmitterComponent::SAudioEmitterComponent(const SEntity& entityOwner)
        : SComponent(entityOwner)
    {
        Init();
    }
    SAudioEmitterComponent::SAudioEmitterComponent(const SEntity& entityOwner, const std::vector<std::string>& assetPaths)
        : SComponent(entityOwner)
        , AssetReferences(SAssetReference::MakeVectorFromPaths(assetPaths))
    {
        Init();
    }

    void SAudioEmitterComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
        U32 numberOfRefs = STATIC_U32(AssetReferences.size());
        SerializeData(numberOfRefs, toData, pointerPosition);
        for (const SAssetReference& assetRef : AssetReferences)
            assetRef.Serialize(toData, pointerPosition);
    }

    void SAudioEmitterComponent::Deserialize(const char* fromData, U64& pointerPosition)
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

    U32 SAudioEmitterComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);
        size += GetDataSize(STATIC_U32(AssetReferences.size()));
        for (const SAssetReference& assetRef : AssetReferences)
            size += assetRef.GetSize();
        return size;
    }

    void SAudioEmitterComponent::IsDeleted(CScene* /*fromScene*/)
    {
        GEngine::GetWorld()->UnregisterAudioObject(AudioObjectID, AssetReferences);
    }

    void SAudioEmitterComponent::Init()
    {
        // NW: AUDIO_ASSET_LOADING: By trying to load referenced assets on begin play (when the audio system is constructed), as well
        // as when components are constructed, we should handle automatically loading all referenced assets. As long as we don't change
        // the lists of AssetReferences on a particular component at play time. This setup will probably show up again so we should try 
        // to keep it in mind, and consider an asset streaming/multithreaded loading solution

        AudioObjectID = GEngine::GetWorld()->RegisterAudioObject(false, AssetReferences);
    }
}
