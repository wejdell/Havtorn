// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraControllerComponentEditorContext.h"

#include "ECS/Components/CameraControllerComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>


namespace Havtorn
{
	SCameraControllerComponentEditorContext SCameraControllerComponentEditorContext::Context = {};

	SComponentViewResult Havtorn::SCameraControllerComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		SCameraControllerComponent* cameraControllerComp = scene->GetComponent<SCameraControllerComponent>(entityOwner);
		GUI::DragFloat("Max Move Speed", cameraControllerComp->MaxMoveSpeed, GUI::SliderSpeed, 0.1f, 10.0f);
		GUI::DragFloat("Rotation Speed", cameraControllerComp->RotationSpeed, GUI::SliderSpeed, 0.1f, 5.0f);
		GUI::DragFloat("Acceleration Duration", cameraControllerComp->AccelerationDuration, GUI::SliderSpeed * 0.1f, 0.1f, 5.0f);

		return SComponentViewResult();
	}

	bool SCameraControllerComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SCameraControllerComponent>(entity);
		scene->AddComponentEditorContext(entity, &SCameraControllerComponentEditorContext::Context);
		return true;
	}

	bool SCameraControllerComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SCameraControllerComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SCameraControllerComponentEditorContext::Context);
		return true;
	}
}
