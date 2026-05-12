// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AudioEmitterComponent.h"
#include "Engine.h"
#include "Assets/AssetRegistry.h"
#include "AudioListenerComponent.h"

namespace Havtorn
{
    SAudioListenerComponent::SAudioListenerComponent(const SEntity& entityOwner)
        : SComponent(entityOwner)
    {
        AudioObjectID = GEngine::GetWorld()->RegisterAudioObject(true, {});
    }
    
    void SAudioListenerComponent::IsDeleted(CScene* /*fromScene*/)
    {
        GEngine::GetWorld()->UnregisterAudioObject(AudioObjectID, {});
    }
}
