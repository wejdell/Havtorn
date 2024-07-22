// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "VolumetricLightComponentView.h"

#include "ECS/Components/VolumetricLightComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult SVolumetricLightComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(entityOwner);

		ImGui::Checkbox("Is Active", &volumetricLightComp->IsActive);
		ImGui::DragFloat("Number Of Samples", &volumetricLightComp->NumberOfSamples, ImGui::UUtils::SliderSpeed, 4.0f);

		volumetricLightComp->NumberOfSamples = Havtorn::UMath::Max(volumetricLightComp->NumberOfSamples, 4.0f);
		ImGui::DragFloat("Light Power", &volumetricLightComp->LightPower, ImGui::UUtils::SliderSpeed * 10000.0f, 0.0f);
		ImGui::DragFloat("Scattering Probability", &volumetricLightComp->ScatteringProbability, ImGui::UUtils::SliderSpeed * 0.1f, 0.0f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
		ImGui::DragFloat("Henyey-Greenstein G", &volumetricLightComp->HenyeyGreensteinGValue);

        return SComponentViewResult();
    }
}
