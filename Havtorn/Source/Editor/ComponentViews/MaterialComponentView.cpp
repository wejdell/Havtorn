// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponentView.h"
#include "ECS/Components/MaterialComponent.h"
#include "Scene/Scene.h"
#include "Assets/AssetReference.h"

#include <GUI.h>

namespace Havtorn
{
    SComponentViewResult Havtorn::SMaterialComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(entityOwner);

		return { EComponentViewResultLabel::InspectAssetComponent, materialComp, SAssetReference::ConvertToPointers(materialComp->AssetReferences), EAssetType::Material };
    }

	U8 SMaterialComponentView::GetSortingPriority() const
	{
		return 3;
	}
}
