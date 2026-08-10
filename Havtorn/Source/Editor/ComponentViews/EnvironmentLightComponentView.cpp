// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EnvironmentLightComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/EnvironmentLightComponent.h>
#include <Scene/Scene.h>
#include <Engine.h>
#include <Assets/AssetReference.h>

#include <GUI.h>

namespace Havtorn
{
    void SEnvironmentLightComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(entityOwner);

		GUI::Checkbox("Is Active", environmentLightComp->IsActive);
		GUI::Text("Ambient Static Cubemap");

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->InspectAssetComponent(environmentLightComp, EAssetType::TextureCube, SAssetReference::ConvertToPointers(environmentLightComp->AssetReference));
    }
}
