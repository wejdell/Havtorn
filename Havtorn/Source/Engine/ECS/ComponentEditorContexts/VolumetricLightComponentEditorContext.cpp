// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "VolumetricLightComponentEditorContext.h"

#include "ECS/Components/VolumetricLightComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
	SVolumetricLightComponentEditorContext SVolumetricLightComponentEditorContext::Context = {};

    SComponentViewResult SVolumetricLightComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!ImGui::UUtils::TryOpenComponentView("VolumetricLight"))
			return SComponentViewResult();

		SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(entityOwner);

		ImGui::Checkbox("Is Active", &volumetricLightComp->IsActive);
		ImGui::DragFloat("Number Of Samples", &volumetricLightComp->NumberOfSamples, ImGui::UUtils::SliderSpeed, 4.0f);

		volumetricLightComp->NumberOfSamples = Havtorn::UMath::Max(volumetricLightComp->NumberOfSamples, 4.0f);
		ImGui::DragFloat("Light Power", &volumetricLightComp->LightPower, ImGui::UUtils::SliderSpeed * 10000.0f, 0.0f);
		ImGui::DragFloat("Scattering Probability", &volumetricLightComp->ScatteringProbability, ImGui::UUtils::SliderSpeed * 0.1f, 0.0f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
		ImGui::DragFloat("Henyey-Greenstein G", &volumetricLightComp->HenyeyGreensteinGValue);

        return SComponentViewResult();
    }

	bool SVolumetricLightComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("Volumetric Light Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SVolumetricLightComponent>(entity);
		scene->AddComponentEditorContext(entity, &SVolumetricLightComponentEditorContext::Context);
		return true;
	}

	bool SVolumetricLightComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("X##19"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SVolumetricLightComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SVolumetricLightComponentEditorContext::Context);
		return true;
	}
}
