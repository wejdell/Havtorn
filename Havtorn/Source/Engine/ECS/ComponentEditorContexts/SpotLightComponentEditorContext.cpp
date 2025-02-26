// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpotLightComponentEditorContext.h"

#include "ECS/Components/SpotLightComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>


namespace Havtorn
{
	SSpotLightComponentEditorContext SSpotLightComponentEditorContext::Context = {};

    SComponentViewResult SSpotLightComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("SpotLight"))
			return SComponentViewResult();

		SSpotLightComponent* spotLightComp = scene->GetComponent<SSpotLightComponent>(entityOwner);

		Havtorn::F32 colorData[3] = { spotLightComp->ColorAndIntensity.X, spotLightComp->ColorAndIntensity.Y, spotLightComp->ColorAndIntensity.Z };
		GUI::ColorPicker3("Color", colorData);
		spotLightComp->ColorAndIntensity.X = colorData[0];
		spotLightComp->ColorAndIntensity.Y = colorData[1];
		spotLightComp->ColorAndIntensity.Z = colorData[2];

		GUI::DragFloat("Intensity", &spotLightComp->ColorAndIntensity.W, GUI::SliderSpeed);
		
		SVector direction = SVector(spotLightComp->Direction.X, spotLightComp->Direction.Y, spotLightComp->Direction.Z);
		GUI::DragFloat3("Direction", direction, GUI::SliderSpeed);
		spotLightComp->Direction = SVector4(direction, 0.0f);
		if (spotLightComp->Direction.IsEqual(SVector4::Zero))
			spotLightComp->Direction = SVector4(0.0f, 0.0f, 0.01f, 0.0f);
		
		GUI::DragFloat("Range", &spotLightComp->Range, GUI::SliderSpeed, 0.1f, 100.0f);
		GUI::DragFloat("Outer Angle", &spotLightComp->OuterAngle, GUI::SliderSpeed, spotLightComp->InnerAngle, 180.0f);
		GUI::DragFloat("InnerAngle", &spotLightComp->InnerAngle, GUI::SliderSpeed, 0.0f, spotLightComp->OuterAngle - 0.01f);
      
		return SComponentViewResult();
    }

	bool SSpotLightComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Spotlight Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SSpotLightComponent>(entity);
		scene->AddComponentEditorContext(entity, &SSpotLightComponentEditorContext::Context);
		return true;
	}

	bool SSpotLightComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##13"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SSpotLightComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SSpotLightComponentEditorContext::Context);
		return true;
	}
}
