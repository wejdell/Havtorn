// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "TransformComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/TransformComponent.h>
#include <Scene/Scene.h>

#include <GUI.h>

namespace Havtorn
{
    void STransformComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transformComponent))
			return;
		
		Havtorn::SMatrix transformMatrix = transformComponent->Transform.GetMatrix();

		SVector matrixTranslation, matrixRotation, matrixScale;
		SMatrix::Decompose(transformMatrix, matrixTranslation, matrixRotation, matrixScale);
		GUI::DragFloat3("Position", matrixTranslation, GUI::SliderSpeed);
		GUI::DragFloat3("Rotation", matrixRotation, GUI::SliderSpeed);
		GUI::DragFloat3("Scale", matrixScale, GUI::SliderSpeed);

		SMatrix::Recompose(matrixTranslation, matrixRotation, matrixScale, transformMatrix);
		transformComponent->Transform.SetMatrix(transformMatrix);

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->UpdateTransformGizmo(transformComponent);
    }

	U8 STransformComponentView::GetSortingPriority() const
	{
		return 1;
	}
}
