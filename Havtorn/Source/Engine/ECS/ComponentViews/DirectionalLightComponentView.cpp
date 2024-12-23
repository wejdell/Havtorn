// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DirectionalLightComponentView.h"

#include "ECS/Components/DirectionalLightComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult SDirectionalLightComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		if (!ImGui::UUtils::TryOpenComponentView("DirectionalLight"))
			return SComponentViewResult();

		SDirectionalLightComponent* directionalLightComp = scene->GetComponent<SDirectionalLightComponent>(entityOwner);

		Havtorn::F32 colorData[3] = { directionalLightComp->Color.X, directionalLightComp->Color.Y, directionalLightComp->Color.Z };
		ImGui::ColorPicker3("Color", colorData);
		directionalLightComp->Color.X = colorData[0];
		directionalLightComp->Color.Y = colorData[1];
		directionalLightComp->Color.Z = colorData[2];

		const SVector4 direction = directionalLightComp->Direction;
		Havtorn::F32 dirData[3] = { direction.X, direction.Y, direction.Z };
		ImGui::DragFloat3("Direction", dirData, ImGui::UUtils::SliderSpeed);
		directionalLightComp->Direction = { dirData[0], dirData[1], dirData[2], 0.0f };
		if (directionalLightComp->Direction.IsEqual(SVector4::Zero))
			directionalLightComp->Direction = SVector4(0.0f, 0.0f, 0.01f, 0.0f);

		ImGui::DragFloat("Intensity", &directionalLightComp->Color.W, ImGui::UUtils::SliderSpeed);

        return SComponentViewResult();
    }
}
