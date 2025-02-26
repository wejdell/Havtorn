// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "TransformComponentEditorContext.h"

#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	STransformComponentEditorContext STransformComponentEditorContext::Context = {};

    SComponentViewResult STransformComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("Transform"))
			return SComponentViewResult();

		STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(entityOwner);
		Havtorn::SMatrix transformMatrix = transformComponent->Transform.GetMatrix();

		SVector matrixTranslation, matrixRotation, matrixScale;
		GUI::DecomposeMatrixToComponents(transformMatrix, matrixTranslation, matrixRotation, matrixScale);
		GUI::DragFloat3("Position", matrixTranslation, GUI::SliderSpeed);
		GUI::DragFloat3("Rotation", matrixRotation, GUI::SliderSpeed);
		GUI::DragFloat3("Scale", matrixScale, GUI::SliderSpeed);

		// TODO.NR: Fix yaw rotation singularity here, using our own math functions. Ref: https://github.com/CedricGuillemet/ImGuizmo/issues/244
		GUI::RecomposeMatrixFromComponents(transformMatrix, matrixTranslation, matrixRotation, matrixScale);
		transformComponent->Transform.SetMatrix(transformMatrix);

		SComponentViewResult result;
		result.Label = EComponentViewResultLabel::UpdateTransformGizmo;
		result.ComponentViewed = transformComponent;

		return result;
    }

	bool STransformComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Transform Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<STransformComponent>(entity);
		scene->AddComponentEditorContext(entity, &STransformComponentEditorContext::Context);
		return true;
	}

	bool STransformComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##18"))
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
