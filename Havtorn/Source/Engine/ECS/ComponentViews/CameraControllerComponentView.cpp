// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraControllerComponentView.h"
#include "ECS/Components/CameraControllerComponent.h"

#include <Core/imgui.h>

void Havtorn::SCameraControllerComponentView::View(const SEntity& entityOwner, CScene* scene)
{
	F32 slideSpeed = 0.1f;

	SCameraControllerComponent* cameraControllerComp = scene->GetComponent<SCameraControllerComponent>(entityOwner);
	ImGui::DragFloat("Max Move Speed", &cameraControllerComp->MaxMoveSpeed, slideSpeed, 0.1f, 10.0f);
	ImGui::DragFloat("Rotation Speed", &cameraControllerComp->RotationSpeed, slideSpeed, 0.1f, 5.0f);
	ImGui::DragFloat("Acceleration Duration", &cameraControllerComp->AccelerationDuration, slideSpeed * 0.1f, 0.1f, 5.0f);
}
