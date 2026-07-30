// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AudioEmitterComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <Engine.h>
#include <Assets/AssetRegistry.h>
#include <ECS/Components/AudioEmitterComponent.h>

#include <GUI.h>

namespace Havtorn
{
    void SAudioEmitterComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
        SAudioEmitterComponent* emitterComponent = scene->GetComponent<SAudioEmitterComponent>(entityOwner);
        if (emitterComponent == nullptr)
            return;

        // TODO.NW: Add coming array GUI element for adding multiple sounds. The component is currently constructed with one in the array.

        CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
        inspector->InspectAssetComponent(emitterComponent, EAssetType::AudioClip, SAssetReference::ConvertToPointers(emitterComponent->AssetReferences));
    }
}
