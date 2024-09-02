// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "TransformComponentView.h"

#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult STransformComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		if (!ImGui::UUtils::TryOpenComponentView("Transform"))
			return SComponentViewResult();

		STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(entityOwner);
		Havtorn::SMatrix transformMatrix = transformComponent->Transform.GetMatrix();

		F32 matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(transformMatrix.data, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, ImGui::UUtils::SliderSpeed);
		ImGui::DragFloat3("Rotation", matrixRotation, ImGui::UUtils::SliderSpeed);
		ImGui::DragFloat3("Scale", matrixScale, ImGui::UUtils::SliderSpeed);

		// TODO.NR: Fix yaw rotation singularity here, using our own math functions. Ref: https://github.com/CedricGuillemet/ImGuizmo/issues/244
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, transformMatrix.data);
		transformComponent->Transform.SetMatrix(transformMatrix);

		SComponentViewResult result;
		result.Label = EComponentViewResultLabel::UpdateTransformGizmo;
		result.ComponentViewed = transformComponent;

		return result;
    }
}
