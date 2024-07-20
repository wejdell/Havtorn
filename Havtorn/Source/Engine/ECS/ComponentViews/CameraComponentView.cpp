// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraComponentView.h"
#include "ECS/Components/CameraComponent.h"

#include <Core/imgui.h>
#include <Core/ImGuizmo/ImGuizmo.h>

namespace Havtorn
{
	void SCameraComponentView::View(const SEntity& entityOwner, CScene* scene)
	{
		SCameraComponent* cameraComp = scene->GetComponent<SCameraComponent>(entityOwner);

		F32 slideSpeed = 0.1f;

		int projectionIndex = static_cast<int>(cameraComp->ProjectionType);
		const char* projectionNames[2] = { "Perspective", "Orthographic" };
		const char* projectionName = (projectionIndex >= 0 && projectionIndex < 2) ? projectionNames[projectionIndex] : "Unknown";
		ImGui::SliderInt("Projection Type", &projectionIndex, 0, 1, projectionName);
		cameraComp->ProjectionType = static_cast<Havtorn::ECameraProjectionType>(projectionIndex);

		if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Perspective)
		{
			ImGui::DragFloat("FOV", &cameraComp->FOV, slideSpeed, 1.0f, 180.0f);
			ImGui::DragFloat("Aspect Ratio", &cameraComp->AspectRatio, slideSpeed, 0.1f, 10.0f);
			ImGuizmo::SetOrthographic(false);
		}
		else if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Orthographic)
		{
			ImGui::DragFloat("View Width", &cameraComp->ViewWidth, slideSpeed, 0.1f, 100.0f);
			ImGui::DragFloat("View Height", &cameraComp->ViewHeight, slideSpeed, 0.1f, 100.0f);
			ImGuizmo::SetOrthographic(true);
		}

		ImGui::DragFloat("Near Clip Plane", &cameraComp->NearClip, slideSpeed, 0.01f, cameraComp->FarClip - 1.0f);
		ImGui::DragFloat("Far Clip Plane", &cameraComp->FarClip, slideSpeed, cameraComp->NearClip + 1.0f, 10000.0f);

		if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Perspective)
		{
			cameraComp->ProjectionMatrix = Havtorn::SMatrix::PerspectiveFovLH(Havtorn::UMath::DegToRad(cameraComp->FOV), cameraComp->AspectRatio, cameraComp->NearClip, cameraComp->FarClip);
		}
		else if (cameraComp->ProjectionType == Havtorn::ECameraProjectionType::Orthographic)
		{
			cameraComp->ProjectionMatrix = Havtorn::SMatrix::OrthographicLH(cameraComp->ViewWidth, cameraComp->ViewHeight, cameraComp->NearClip, cameraComp->FarClip);
		}
	}
}
