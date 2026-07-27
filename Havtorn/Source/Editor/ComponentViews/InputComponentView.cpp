// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "InputComponentView.h"
#include "ECS/Components/InputComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	SComponentViewResult SInputComponentView::View(const SEntity& entityOwner, CScene* scene) const
	{
		SInputComponent* component = scene->GetComponent<SInputComponent>(entityOwner);
		return SComponentViewResult{ .Label = EComponentViewResultLabel::InspectAssetComponent, .ComponentViewed = component, .AssetReferences = SAssetReference::ConvertToPointers(component->AssetReference), .AssetType = EAssetType::InputAsset };
	}
}
