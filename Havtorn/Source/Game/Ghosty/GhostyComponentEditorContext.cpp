// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "GhostyComponentEditorContext.h"

#include "GhostyComponent.h"

#include <Scene/Scene.h>
#include <GUI.h>

namespace Havtorn
{
	SGhostyComponentEditorContext SGhostyComponentEditorContext::Context = {};

	SComponentViewResult SGhostyComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("Ghosty"))
			return SComponentViewResult();

		SGhostyComponent* component = scene->GetComponent<SGhostyComponent>(entityOwner);

		GUI::DragFloat3("GhostyState", component->State.Input, 0.0f);

		GUI::Checkbox("IsInWalkingAnimation", &component->State.IsInWalkingAnimationState);

		return SComponentViewResult();
	}

	bool SGhostyComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Ghosty Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SGhostyComponent>(entity);
		scene->AddComponentEditorContext(entity, &SGhostyComponentEditorContext::Context);
		return true;
	}

	bool SGhostyComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SGhostyComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SGhostyComponentEditorContext::Context);
		return true;
	}
}
