// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpotLightComponentView.h"

#include "ECS/Components/SpotLightComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult SSpotLightComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		SSpotLightComponent* spotLightComp = scene->GetComponent<SSpotLightComponent>(entityOwner);

		Havtorn::F32 colorData[3] = { spotLightComp->ColorAndIntensity.X, spotLightComp->ColorAndIntensity.Y, spotLightComp->ColorAndIntensity.Z };
		ImGui::ColorPicker3("Color", colorData);
		spotLightComp->ColorAndIntensity.X = colorData[0];
		spotLightComp->ColorAndIntensity.Y = colorData[1];
		spotLightComp->ColorAndIntensity.Z = colorData[2];

		ImGui::DragFloat("Intensity", &spotLightComp->ColorAndIntensity.W, ImGui::UUtils::SliderSpeed);
		ImGui::DragFloat("Range", &spotLightComp->Range, ImGui::UUtils::SliderSpeed, 0.1f, 100.0f);
		ImGui::DragFloat("Outer Angle", &spotLightComp->OuterAngle, ImGui::UUtils::SliderSpeed, spotLightComp->InnerAngle, 180.0f);
		ImGui::DragFloat("InnerAngle", &spotLightComp->InnerAngle, ImGui::UUtils::SliderSpeed, 0.0f, spotLightComp->OuterAngle - 0.01f);
      
		return SComponentViewResult();
    }
}
