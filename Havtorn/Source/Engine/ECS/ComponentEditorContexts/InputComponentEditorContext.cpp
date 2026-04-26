// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "InputComponentEditorContext.h"
#include "ECS/Components/InputComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	SInputComponentEditorContext SInputComponentEditorContext::Context = { };

	SComponentViewResult SInputComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		SInputComponent* component = scene->GetComponent<SInputComponent>(entityOwner);
		return SComponentViewResult{ .Label = EComponentViewResultLabel::InspectAssetComponent, .ComponentViewed = component, .AssetReferences = SAssetReference::ConvertToPointers(component->AssetReference), .AssetType = EAssetType::InputAsset };
	}

	bool SInputComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SInputComponent>(entity);
		scene->AddComponentEditorContext(entity, &SInputComponentEditorContext::Context);
		return true;
	}

	bool SInputComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SInputComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SInputComponentEditorContext::Context);
		return true;
	}
}
