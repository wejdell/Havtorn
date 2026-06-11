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
		SSpotLightComponent* spotLightComp = scene->GetComponent<SSpotLightComponent>(entityOwner);

		GUI::Checkbox("Is Active", spotLightComp->IsActive);

		SColor color = spotLightComp->ColorAndIntensity;
		GUI::ColorPicker3("Color", color);
		SVector colorFloat = color.AsVector();
		spotLightComp->ColorAndIntensity = { colorFloat.X, colorFloat.Y, colorFloat.Z, spotLightComp->ColorAndIntensity.W };

		GUI::DragFloat("Intensity", spotLightComp->ColorAndIntensity.W, GUI::SliderSpeed);
		
		SVector direction = SVector(spotLightComp->Direction.X, spotLightComp->Direction.Y, spotLightComp->Direction.Z);
		GUI::DragFloat3("Direction", direction, GUI::SliderSpeed);
		spotLightComp->Direction = SVector4(direction, 0.0f);
		if (spotLightComp->Direction.IsEqual(SVector4::Zero))
			spotLightComp->Direction = SVector4(0.0f, 0.0f, 0.01f, 0.0f);
		
		GUI::DragFloat("Range", spotLightComp->Range, GUI::SliderSpeed, 0.1f, 100.0f);
		GUI::DragFloat("Outer Angle", spotLightComp->OuterAngle, GUI::SliderSpeed, spotLightComp->InnerAngle, 180.0f);
		GUI::DragFloat("InnerAngle", spotLightComp->InnerAngle, GUI::SliderSpeed, 0.0f, spotLightComp->OuterAngle - 0.01f);
      
		I32 shadowmapStartIndex = STATIC_I32(spotLightComp->ShadowmapView.ShadowmapViewportIndex);
		if (GUI::InputInt("Shadowmap Index", shadowmapStartIndex))
		{
			// NW: Would be nice to pull this directly from the rendermanager, or some form of common settings
			constexpr U16 maxShadowmapViews = 184;

			shadowmapStartIndex = UMath::Clamp(shadowmapStartIndex, 0, maxShadowmapViews - 1);
			spotLightComp->ShadowmapView.ShadowmapViewportIndex = STATIC_U16(shadowmapStartIndex);
		}

		return SComponentViewResult();
    }

	bool SSpotLightComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SSpotLightComponent>(entity);
		scene->AddComponentEditorContext(entity, &SSpotLightComponentEditorContext::Context);
		return true;
	}

	bool SSpotLightComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SSpotLightComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SSpotLightComponentEditorContext::Context);
		return true;
	}
}
