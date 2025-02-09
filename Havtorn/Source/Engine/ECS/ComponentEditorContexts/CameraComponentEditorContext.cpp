// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraComponentEditorContext.h"

#include "ECS/Components/CameraComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Core/ImGuizmo/ImGuizmo.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
	SCameraComponentEditorContext SCameraComponentEditorContext::Context = {};

	SComponentViewResult SCameraComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!ImGui::UUtils::TryOpenComponentView("Camera"))
			return SComponentViewResult();

		SCameraComponent* cameraComp = scene->GetComponent<SCameraComponent>(entityOwner);

		int projectionIndex = static_cast<int>(cameraComp->ProjectionType);
		const char* projectionNames[2] = { "Perspective", "Orthographic" };
		const char* projectionName = (projectionIndex >= 0 && projectionIndex < 2) ? projectionNames[projectionIndex] : "Unknown";
		ImGui::SliderInt("Projection Type", &projectionIndex, 0, 1, projectionName);
		cameraComp->ProjectionType = static_cast<Havtorn::ECameraProjectionType>(projectionIndex);

		if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Perspective)
		{
			ImGui::DragFloat("FOV", &cameraComp->FOV, ImGui::UUtils::SliderSpeed, 1.0f, 180.0f);
			ImGui::DragFloat("Aspect Ratio", &cameraComp->AspectRatio, ImGui::UUtils::SliderSpeed, 0.1f, 10.0f);
			ImGuizmo::SetOrthographic(false);
		}
		else if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Orthographic)
		{
			ImGui::DragFloat("View Width", &cameraComp->ViewWidth, ImGui::UUtils::SliderSpeed, 0.1f, 100.0f);
			ImGui::DragFloat("View Height", &cameraComp->ViewHeight, ImGui::UUtils::SliderSpeed, 0.1f, 100.0f);
			ImGuizmo::SetOrthographic(true);
		}

		ImGui::DragFloat("Near Clip Plane", &cameraComp->NearClip, ImGui::UUtils::SliderSpeed, 0.01f, cameraComp->FarClip - 1.0f);
		ImGui::DragFloat("Far Clip Plane", &cameraComp->FarClip, ImGui::UUtils::SliderSpeed, cameraComp->NearClip + 1.0f, 10000.0f);

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
		if (!ImGui::Button("Camera Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SCameraComponent>(entity);
		scene->AddComponentEditorContext(entity, &SCameraComponentEditorContext::Context);
		return true;
	}

	bool SCameraComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("X##1"))
		{
			//ImGui::PopID();
			return false;
		}

		if (scene == nullptr || !entity.IsValid())
		{
			//ImGui::PopID();
			return false;
		}

		scene->RemoveComponent<SCameraComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SCameraComponentEditorContext::Context);
		//ImGui::PopID();
		return true;
	}
}
