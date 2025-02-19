// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "GhostyComponentView.h"

#include "GhostyComponent.h"

#include <Scene/Scene.h>
//#include <imgui/imgui.h>
//#include <Core/Utilities.h>

//#include <Editor/GUI.h>
#include <GUIProcess.h>
#include <Core/Utilities.h>

#include <GUI.h>

namespace Havtorn
{
	SComponentViewResult SGhostyComponentView::View(const SEntity& entityOwner, CScene* scene)
	{
		entityOwner;
		scene;

		//GUI::GUI::Begin();

		//float a;

		//a = GUI::GUIProcess::DragFloat("HelloFloat", &a, 0.1f, 20.0f);

		if (!ImGui::UUtils::TryOpenComponentView("Ghosty"))
			return SComponentViewResult();

		SGhostyComponent* component = scene->GetComponent<SGhostyComponent>(entityOwner);

		F32 ghostyInput[3] = { component->State.Input.X, component->State.Input.Y, component->State.Input.Z };
		GUI::DragFloat3("GhostyState", ghostyInput, 0.0f, 1.0f);
		GUI::CheckBox("Is In Walking Animation", &component->State.IsInWalkingAnimationState);
		
		//GUI::GUIProcess::DragFloat3("GhostyState", ghostyInput, 0.0f, 1.0f);
		//GUI::GUIProcess::CheckBox("IsInWalkingAnimation", &component->State.IsInWalkingAnimationState);
		//ImGui::DragFloat3("GhostyState", ghostyInput, 0.0f);
		//ImGui::Checkbox("IsInWalkingAnimation", &component->State.IsInWalkingAnimationState);


		return SComponentViewResult();
	}
}
