// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "VolumetricLightComponentEditorContext.h"

#include "ECS/Components/VolumetricLightComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	SVolumetricLightComponentEditorContext SVolumetricLightComponentEditorContext::Context = {};

    SComponentViewResult SVolumetricLightComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("VolumetricLight"))
			return SComponentViewResult();

		SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(entityOwner);

		GUI::Checkbox("Is Active", &volumetricLightComp->IsActive);
		GUI::DragFloat("Number Of Samples", &volumetricLightComp->NumberOfSamples, GUI::SliderSpeed, 4.0f);

		volumetricLightComp->NumberOfSamples = Havtorn::UMath::Max(volumetricLightComp->NumberOfSamples, 4.0f);
		GUI::DragFloat("Light Power", &volumetricLightComp->LightPower, GUI::SliderSpeed * 10000.0f, 0.0f);
		GUI::DragFloat("Scattering Probability", &volumetricLightComp->ScatteringProbability, GUI::SliderSpeed * 0.1f, 0.0f, 1.0f, "%.4f", EDragMode::Logarithmic);
		GUI::DragFloat("Henyey-Greenstein G", &volumetricLightComp->HenyeyGreensteinGValue);

        return SComponentViewResult();
    }

	bool SVolumetricLightComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Volumetric Light Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SVolumetricLightComponent>(entity);
		scene->AddComponentEditorContext(entity, &SVolumetricLightComponentEditorContext::Context);
		return true;
	}

	bool SVolumetricLightComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##19"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SVolumetricLightComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SVolumetricLightComponentEditorContext::Context);
		return true;
	}
}
