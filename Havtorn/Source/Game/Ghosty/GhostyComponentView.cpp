// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "GhostyComponentView.h"

#include "GhostyComponent.h"

#include <Scene/Scene.h>
#include <Core/imgui.h>

namespace Havtorn
{
	SComponentViewResult SGhostyComponentView::View(const SEntity& entityOwner, CScene* scene)
	{
		SGhostyComponent* component = scene->GetComponent<SGhostyComponent>(entityOwner);

		F32 ghostyInput[3] = { component->State.Input.X, component->State.Input.Y, component->State.Input.Z };
		ImGui::DragFloat3("GhostyState", ghostyInput, 0.0f);

		ImGui::Checkbox("IsInWalkingAnimation", &component->State.IsInWalkingAnimationState);

		return SComponentViewResult();
	}
}