// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "PointLightComponentView.h"

#include "ECS/Components/PointLightComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult Havtorn::SPointLightComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		SPointLightComponent* pointLightComp = scene->GetComponent<SPointLightComponent>(entityOwner);

		Havtorn::F32 colorData[3] = { pointLightComp->ColorAndIntensity.X, pointLightComp->ColorAndIntensity.Y, pointLightComp->ColorAndIntensity.Z };
		ImGui::ColorPicker3("Color", colorData);
		pointLightComp->ColorAndIntensity.X = colorData[0];
		pointLightComp->ColorAndIntensity.Y = colorData[1];
		pointLightComp->ColorAndIntensity.Z = colorData[2];

		ImGui::DragFloat("Intensity", &pointLightComp->ColorAndIntensity.W, ImGui::UUtils::SliderSpeed);
		ImGui::DragFloat("Range", &pointLightComp->Range, ImGui::UUtils::SliderSpeed, 0.1f, 100.0f);

        return SComponentViewResult();
    }
}
