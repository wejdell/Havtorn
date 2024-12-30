// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "GhostyComponentEditorContext.h"

#include "GhostyComponent.h"

#include <Scene/Scene.h>
#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
	SGhostyComponentEditorContext SGhostyComponentEditorContext::Context = {};

	SComponentViewResult SGhostyComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!ImGui::UUtils::TryOpenComponentView("Ghosty"))
			return SComponentViewResult();

		SGhostyComponent* component = scene->GetComponent<SGhostyComponent>(entityOwner);

		F32 ghostyInput[3] = { component->State.Input.X, component->State.Input.Y, component->State.Input.Z };
		ImGui::DragFloat3("GhostyState", ghostyInput, 0.0f);

		ImGui::Checkbox("IsInWalkingAnimation", &component->State.IsInWalkingAnimationState);

		return SComponentViewResult();
	}

	bool SGhostyComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("Ghosty Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SGhostyComponent>(entity);
		scene->AddComponentEditorContext(entity, &SGhostyComponentEditorContext::Context);
		return true;
	}

	bool SGhostyComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("X"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SGhostyComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SGhostyComponentEditorContext::Context);
		return true;
	}
}
