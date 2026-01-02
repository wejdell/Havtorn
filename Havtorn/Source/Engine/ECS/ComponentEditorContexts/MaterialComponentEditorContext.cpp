// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponentEditorContext.h"
#include "ECS/Components/MaterialComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>


namespace Havtorn
{
	SMaterialComponentEditorContext SMaterialComponentEditorContext::Context = {};

    SComponentViewResult Havtorn::SMaterialComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("Material"))
			return SComponentViewResult();

		SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(entityOwner);

		return { EComponentViewResultLabel::InspectAssetComponent, materialComp, SAssetReference::ConvertToPointers(materialComp->AssetReferences), EAssetType::Material };
    }
	
	bool SMaterialComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Material Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SMaterialComponent>(entity);
		scene->AddComponentEditorContext(entity, &SMaterialComponentEditorContext::Context);
		return true;
	}

	bool SMaterialComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##6"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SMaterialComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SMaterialComponentEditorContext::Context);
		return true;
	}

	U8 SMaterialComponentEditorContext::GetSortingPriority() const
	{
		return 3;
	}
}
