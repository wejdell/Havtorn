// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpotLightComponentView.h"

#include "ECS/Components/SpotLightComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

#include "Graphics/Debug/DebugDrawUtility.h"

#include <GUI.h>

namespace Havtorn
{
    SComponentViewResult SSpotLightComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		SSpotLightComponent* spotLightComp = scene->GetComponent<SSpotLightComponent>(entityOwner);

		GUI::Checkbox("Is Active", spotLightComp->IsActive);

		SColor color = spotLightComp->ColorAndIntensity;
		GUI::ColorPicker3("Color", color);
		SVector colorFloat = color.AsVector();
		spotLightComp->ColorAndIntensity = { colorFloat.X, colorFloat.Y, colorFloat.Z, spotLightComp->ColorAndIntensity.W };

		GUI::DragFloat("Intensity", spotLightComp->ColorAndIntensity.W, GUI::SliderSpeed);	
		
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

		if (STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(spotLightComp))
		{
			const SVector pos = transformComponent->Transform.GetMatrix().GetTranslation();
			GDebugDraw::AddConeAngle(pos, spotLightComp->Direction.ToVector3(), spotLightComp->Range, spotLightComp->OuterAngle, SColor::Magenta, 0.0f);
		}

		return SComponentViewResult();
    }
}
