// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/MaterialComponent.h>
#include <Scene/Scene.h>
#include <Assets/AssetReference.h>

#include <GUI.h>

namespace Havtorn
{
    void Havtorn::SMaterialComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(entityOwner);
		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->InspectAssetComponent(materialComp, EAssetType::Material, SAssetReference::ConvertToPointers(materialComp->AssetReferences));
    }

	U8 SMaterialComponentView::GetSortingPriority() const
	{
		return 3;
	}
}
