// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "PrefabComponentEditorContext.h"

#include "Engine.h"
#include "Assets/AssetRegistry.h"
#include "ECS/ComponentAlgo.h"
#include "ECS/Components/PrefabComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/ComponentEditorContexts/MaterialComponentEditorContext.h"
#include "Scene/Scene.h"
#include "Assets/AssetReference.h"

#include "Graphics/Debug/DebugDrawUtility.h"

#include <GUI.h>

namespace Havtorn
{
	SPrefabComponentEditorContext SPrefabComponentEditorContext::Context = {};

    SComponentViewResult SPrefabComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("Prefab"))
			return SComponentViewResult();

		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transform))
			return SComponentViewResult();

		SPrefabComponent* prefab = scene->GetComponent<SPrefabComponent>(entityOwner);
		const SPrefabAsset* prefabAsset = GEngine::GetAssetRegistry()->RequestAssetData<SPrefabAsset>(prefab->AssetReference, entityOwner.GUID);
		if (prefabAsset == nullptr)
			return { EComponentViewResultLabel::InspectAssetComponent, prefab, SAssetReference::ConvertToPointers(prefab->AssetReference), EAssetType::Prefab };

		// TODO.NW: Handle spawning vs building mode

		// TODO.NW: Handle unpacking

		// TODO.NW: Handle Refreshing when asset changes

		return { EComponentViewResultLabel::InspectAssetComponent, prefab, SAssetReference::ConvertToPointers(prefab->AssetReference), EAssetType::Prefab };
    }

	bool SPrefabComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Prefab Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SPrefabComponent>(entity);
		scene->AddComponentEditorContext(entity, &SPrefabComponentEditorContext::Context);

		return true;
	}

	bool SPrefabComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##22"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SPrefabComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SPrefabComponentEditorContext::Context);
		return true;
	}

	U8 SPrefabComponentEditorContext::GetSortingPriority() const
	{
		return 4;
	}
}
