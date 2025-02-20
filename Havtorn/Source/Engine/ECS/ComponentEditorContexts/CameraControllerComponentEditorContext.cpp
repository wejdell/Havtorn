// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraControllerComponentEditorContext.h"

#include "ECS/Components/CameraControllerComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
	SCameraControllerComponentEditorContext SCameraControllerComponentEditorContext::Context = {};

	SComponentViewResult Havtorn::SCameraControllerComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!ImGui::UUtils::TryOpenComponentView("CameraController"))
			return SComponentViewResult();

		SCameraControllerComponent* cameraControllerComp = scene->GetComponent<SCameraControllerComponent>(entityOwner);
		ImGui::DragFloat("Max Move Speed", &cameraControllerComp->MaxMoveSpeed, ImGui::UUtils::SliderSpeed, 0.1f, 10.0f);
		ImGui::DragFloat("Rotation Speed", &cameraControllerComp->RotationSpeed, ImGui::UUtils::SliderSpeed, 0.1f, 5.0f);
		ImGui::DragFloat("Acceleration Duration", &cameraControllerComp->AccelerationDuration, ImGui::UUtils::SliderSpeed * 0.1f, 0.1f, 5.0f);

		return SComponentViewResult();
	}

	bool SCameraControllerComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("Camera Controller Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SCameraControllerComponent>(entity);
		scene->AddComponentEditorContext(entity, &SCameraControllerComponentEditorContext::Context);
		return true;
	}

	bool SCameraControllerComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("X##2"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SCameraControllerComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SCameraControllerComponentEditorContext::Context);
		return true;
	}
}
