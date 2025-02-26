// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraComponentEditorContext.h"

#include "ECS/Components/CameraComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	SCameraComponentEditorContext SCameraComponentEditorContext::Context = {};

	SComponentViewResult SCameraComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("Camera"))
			return SComponentViewResult();

		SCameraComponent* cameraComp = scene->GetComponent<SCameraComponent>(entityOwner);

		int projectionIndex = static_cast<int>(cameraComp->ProjectionType);
		const char* projectionNames[2] = { "Perspective", "Orthographic" };
		const char* projectionName = (projectionIndex >= 0 && projectionIndex < 2) ? projectionNames[projectionIndex] : "Unknown";
		GUI::SliderInt("Projection Type", &projectionIndex, 0, 1, projectionName);
		cameraComp->ProjectionType = static_cast<Havtorn::ECameraProjectionType>(projectionIndex);

		if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Perspective)
		{
			GUI::DragFloat("FOV", &cameraComp->FOV, GUI::SliderSpeed, 1.0f, 180.0f);
			GUI::DragFloat("Aspect Ratio", &cameraComp->AspectRatio, GUI::SliderSpeed, 0.1f, 10.0f);
			GUI::SetOrthographic(false);
		}
		else if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Orthographic)
		{
			GUI::DragFloat("View Width", &cameraComp->ViewWidth, GUI::SliderSpeed, 0.1f, 100.0f);
			GUI::DragFloat("View Height", &cameraComp->ViewHeight, GUI::SliderSpeed, 0.1f, 100.0f);
			GUI::SetOrthographic(true);
		}

		GUI::DragFloat("Near Clip Plane", &cameraComp->NearClip, GUI::SliderSpeed, 0.01f, cameraComp->FarClip - 1.0f);
		GUI::DragFloat("Far Clip Plane", &cameraComp->FarClip, GUI::SliderSpeed, cameraComp->NearClip + 1.0f, 10000.0f);

		if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Perspective)
		{
			cameraComp->ProjectionMatrix = Havtorn::SMatrix::PerspectiveFovLH(Havtorn::UMath::DegToRad(cameraComp->FOV), cameraComp->AspectRatio, cameraComp->NearClip, cameraComp->FarClip);
		}
		else if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Orthographic)
		{
			cameraComp->ProjectionMatrix = Havtorn::SMatrix::OrthographicLH(cameraComp->ViewWidth, cameraComp->ViewHeight, cameraComp->NearClip, cameraComp->FarClip);
		}

		return SComponentViewResult();
	}

	bool SCameraComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Camera Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SCameraComponent>(entity);
		scene->AddComponentEditorContext(entity, &SCameraComponentEditorContext::Context);
		return true;
	}

	bool SCameraComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##1"))
		{
			//GUI::PopID();
			return false;
		}

		if (scene == nullptr || !entity.IsValid())
		{
			//GUI::PopID();
			return false;
		}

		scene->RemoveComponent<SCameraComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SCameraComponentEditorContext::Context);
		//GUI::PopID();
		return true;
	}
}
