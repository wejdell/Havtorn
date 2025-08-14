// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "PointLightComponentEditorContext.h"

#include "ECS/Components/PointLightComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>


namespace Havtorn
{
	SPointLightComponentEditorContext SPointLightComponentEditorContext::Context = {};

    SComponentViewResult Havtorn::SPointLightComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("PointLight"))
			return SComponentViewResult();

		SPointLightComponent* pointLightComp = scene->GetComponent<SPointLightComponent>(entityOwner);

		GUI::Checkbox("Is Active", pointLightComp->IsActive);

		SColor color = pointLightComp->ColorAndIntensity;
		GUI::ColorPicker3("Color", color);
		SVector colorFloat = color.AsVector();
		pointLightComp->ColorAndIntensity = { colorFloat.X, colorFloat.Y, colorFloat.Z, pointLightComp->ColorAndIntensity.W };

		GUI::DragFloat("Intensity", pointLightComp->ColorAndIntensity.W, GUI::SliderSpeed);
		GUI::DragFloat("Range", pointLightComp->Range, GUI::SliderSpeed, 0.1f, 100.0f);

        return SComponentViewResult();
    }

	bool SPointLightComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Point Light Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SPointLightComponent>(entity);
		scene->AddComponentEditorContext(entity, &SPointLightComponentEditorContext::Context);
		return true;
	}

	bool SPointLightComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##10"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SPointLightComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SPointLightComponentEditorContext::Context);
		return true;
	}
}
