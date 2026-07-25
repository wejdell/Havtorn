// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraControllerComponentView.h"

#include "ECS/Components/CameraControllerComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>


namespace Havtorn
{
	SCameraControllerComponentView SCameraControllerComponentView::Context = {};

	SComponentViewResult Havtorn::SCameraControllerComponentView::View(const SEntity& entityOwner, CScene* scene) const
	{
		SCameraControllerComponent* cameraControllerComp = scene->GetComponent<SCameraControllerComponent>(entityOwner);
		GUI::DragFloat("Max Move Speed", cameraControllerComp->MaxMoveSpeed, GUI::SliderSpeed, 0.1f, 10.0f);
		GUI::DragFloat("Rotation Speed", cameraControllerComp->RotationSpeed, GUI::SliderSpeed, 0.1f, 5.0f);
		GUI::DragFloat("Acceleration Duration", cameraControllerComp->AccelerationDuration, GUI::SliderSpeed * 0.1f, 0.1f, 5.0f);
		GUI::ComboEnum("Controller Type", cameraControllerComp->ControllerType);

		return SComponentViewResult();
	}
}
