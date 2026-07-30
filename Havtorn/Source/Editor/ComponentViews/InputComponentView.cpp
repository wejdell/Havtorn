// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "InputComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/InputComponent.h>
#include <Scene/Scene.h>

#include <GUI.h>

namespace Havtorn
{
	void SInputComponentView::View(const SEntity& entityOwner, CScene* scene) const
	{
		SInputComponent* component = scene->GetComponent<SInputComponent>(entityOwner);
		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->InspectAssetComponent(component, EAssetType::InputAsset, SAssetReference::ConvertToPointers(component->AssetReference));
	}
}
