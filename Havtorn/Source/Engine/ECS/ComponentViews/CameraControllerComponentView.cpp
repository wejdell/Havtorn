// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraControllerComponentView.h"

#include "ECS/Components/CameraControllerComponent.h"
#include "Scene/Scene.h"

#include <imgui.h>
#include "Core/Utilities.h"

namespace Havtorn
{
	SComponentViewResult Havtorn::SCameraControllerComponentView::View(const SEntity& entityOwner, CScene* scene)
	{
		if (!ImGui::UUtils::TryOpenComponentView("CameraController"))
			return SComponentViewResult();

		SCameraControllerComponent* cameraControllerComp = scene->GetComponent<SCameraControllerComponent>(entityOwner);
		ImGui::DragFloat("Max Move Speed", &cameraControllerComp->MaxMoveSpeed, ImGui::UUtils::SliderSpeed, 0.1f, 10.0f);
		ImGui::DragFloat("Rotation Speed", &cameraControllerComp->RotationSpeed, ImGui::UUtils::SliderSpeed, 0.1f, 5.0f);
		ImGui::DragFloat("Acceleration Duration", &cameraControllerComp->AccelerationDuration, ImGui::UUtils::SliderSpeed * 0.1f, 0.1f, 5.0f);

		return SComponentViewResult();
	}
}
