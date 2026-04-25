// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponentEditorContext.h"
#include "ECS/Components/MaterialComponent.h"
#include "Scene/Scene.h"
#include "Assets/AssetReference.h"

#include <GUI.h>


namespace Havtorn
{
	SMaterialComponentEditorContext SMaterialComponentEditorContext::Context = {};

    SComponentViewResult Havtorn::SMaterialComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(entityOwner);

		return { EComponentViewResultLabel::InspectAssetComponent, materialComp, SAssetReference::ConvertToPointers(materialComp->AssetReferences), EAssetType::Material };
    }
	
	bool SMaterialComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SMaterialComponent>(entity);
		scene->AddComponentEditorContext(entity, &SMaterialComponentEditorContext::Context);
		return true;
	}

	bool SMaterialComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SMaterialComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SMaterialComponentEditorContext::Context);
		return true;
	}

	U8 SMaterialComponentEditorContext::GetSortingPriority() const
	{
		return 3;
	}
}
