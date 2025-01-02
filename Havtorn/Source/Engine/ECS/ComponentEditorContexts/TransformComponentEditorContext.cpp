// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "TransformComponentEditorContext.h"

#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
	STransformComponentEditorContext STransformComponentEditorContext::Context = {};

    SComponentViewResult STransformComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
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

	bool STransformComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("Transform Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<STransformComponent>(entity);
		scene->AddComponentEditorContext(entity, &STransformComponentEditorContext::Context);
		return true;
	}

	bool STransformComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("X##17"))
			return false;
			
		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<STransformComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &STransformComponentEditorContext::Context);
		return true;
	}

	U8 STransformComponentEditorContext::GetSortingPriority() const
	{
		return 1;
	}
}
