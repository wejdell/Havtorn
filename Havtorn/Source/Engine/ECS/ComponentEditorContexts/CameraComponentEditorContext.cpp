// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraComponentEditorContext.h"

#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

#include "Graphics/Debug/DebugDrawUtility.h"

#include <GUI.h>

namespace Havtorn
{
	SCameraComponentEditorContext SCameraComponentEditorContext::Context = {};

	SComponentViewResult SCameraComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("Camera"))
			return SComponentViewResult();

		SCameraComponent* cameraComp = scene->GetComponent<SCameraComponent>(entityOwner);
		if (!SComponent::IsValid(cameraComp))
			return SComponentViewResult();

		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transform))
			return SComponentViewResult();

		GUI::SliderEnum("Projection Type", cameraComp->ProjectionType, { "Perspective", "Orthographic" });

		if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Perspective)
		{
			GUI::DragFloat("FOV", cameraComp->FOV, GUI::SliderSpeed, 1.0f, 180.0f);
			GUI::DragFloat("Aspect Ratio", cameraComp->AspectRatio, GUI::SliderSpeed, 0.1f, 10.0f);
			GUI::SetOrthographic(false);
		}
		else if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Orthographic)
		{
			GUI::DragFloat("View Width", cameraComp->ViewWidth, GUI::SliderSpeed, 0.1f, 100.0f);
			GUI::DragFloat("View Height", cameraComp->ViewHeight, GUI::SliderSpeed, 0.1f, 100.0f);
			GUI::SetOrthographic(true);
		}

		GUI::DragFloat("Near Clip Plane", cameraComp->NearClip, GUI::SliderSpeed, 0.01f, cameraComp->FarClip - 1.0f);
		GUI::DragFloat("Far Clip Plane", cameraComp->FarClip, GUI::SliderSpeed, cameraComp->NearClip + 1.0f, 10000.0f);

		if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Perspective)
		{
			cameraComp->ProjectionMatrix = Havtorn::SMatrix::PerspectiveFovLH(Havtorn::UMath::DegToRad(cameraComp->FOV), cameraComp->AspectRatio, cameraComp->NearClip, cameraComp->FarClip);
		}
		else if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Orthographic)
		{
			cameraComp->ProjectionMatrix = Havtorn::SMatrix::OrthographicLH(cameraComp->ViewWidth, cameraComp->ViewHeight, cameraComp->NearClip, cameraComp->FarClip);
		}

		GUI::Checkbox("Is Starting Camera", cameraComp->IsStartingCamera);

		if (cameraComp->Owner != scene->MainCameraEntity)
		{
			GUI::Checkbox("Is Active", cameraComp->IsActive);

			SMatrix transformMatrix = transform->Transform.GetMatrix();
			GDebugDraw::AddCamera(transformMatrix.GetTranslation(), transformMatrix.GetEuler(), cameraComp->FOV, cameraComp->AspectRatio, cameraComp->FarClip, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum * 2.0f, false);

			if (cameraComp->IsActive)
			{
				SComponentViewResult result;
				result.ComponentViewed = cameraComp;
				result.Label = EComponentViewResultLabel::RenderPreview;
				return result;
			}
		}
		else
			GUI::TextDisabled("Main Camera");
		
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
