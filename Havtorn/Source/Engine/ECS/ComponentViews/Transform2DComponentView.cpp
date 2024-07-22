// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Transform2DComponentView.h"

#include "ECS/Components/Transform2DComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult STransform2DComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		// TODO.NR: Make editable with gizmo
		STransform2DComponent* transform2DComp = scene->GetComponent<STransform2DComponent>(entityOwner);

		F32 position[2] = { transform2DComp->Position.X, transform2DComp->Position.Y };
		F32 scale[2] = { transform2DComp->Scale.X, transform2DComp->Scale.Y };

		ImGui::DragFloat2("Position", position, ImGui::UUtils::SliderSpeed);
		ImGui::DragFloat("DegreesRoll", &transform2DComp->DegreesRoll, ImGui::UUtils::SliderSpeed);
		ImGui::DragFloat2("Scale", scale, ImGui::UUtils::SliderSpeed);

		transform2DComp->Position = { position[0], position[1] };
		transform2DComp->Scale = { scale[0], scale[1] };

        return SComponentViewResult();
    }
}
