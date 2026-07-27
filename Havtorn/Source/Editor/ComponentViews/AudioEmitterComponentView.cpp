// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AudioEmitterComponentView.h"
#include "Engine.h"
#include "Assets/AssetRegistry.h"

#include "ECS/Components/AudioEmitterComponent.h"

#include <GUI.h>

namespace Havtorn
{
    SComponentViewResult SAudioEmitterComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
        SAudioEmitterComponent* emitterComponent = scene->GetComponent<SAudioEmitterComponent>(entityOwner);
        if (emitterComponent == nullptr)
            return {};

        // TODO.NW: Add coming array GUI element for adding multiple sounds. The component is currently constructed with one in the array.

        return { EComponentViewResultLabel::InspectAssetComponent, emitterComponent, SAssetReference::ConvertToPointers(emitterComponent->AssetReferences), EAssetType::AudioClip };
    }
}
