// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ScriptComponentEditorContext.h"

#include "ECS/Components/ScriptComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	SScriptComponentEditorContext SScriptComponentEditorContext::Context = {};

	SComponentViewResult SScriptComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("Script"))
			return SComponentViewResult();

		SScriptComponent* component = scene->GetComponent<SScriptComponent>(entityOwner);

		return { EComponentViewResultLabel::InspectAssetComponent, component, 0 };
	}

	bool SScriptComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Script Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SScriptComponent>(entity);
		scene->AddComponentEditorContext(entity, &SScriptComponentEditorContext::Context);
		return true;
	}

	bool SScriptComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		// TODO.NW: Push IDs here instead of having to this labeling
		if (!GUI::Button("X##11"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SScriptComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SScriptComponentEditorContext::Context);
		return true;
	}

	U8 SScriptComponentEditorContext::GetSortingPriority() const
	{
		return 1;
	}
}
