// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "PointLightComponentView.h"

#include <ECS/Components/PointLightComponent.h>
#include <ECS/Components/TransformComponent.h>
#include <Scene/Scene.h>

#include <Graphics/Debug/DebugDrawUtility.h>

#include <GUI.h>

namespace Havtorn
{
    void Havtorn::SPointLightComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		SPointLightComponent* pointLightComp = scene->GetComponent<SPointLightComponent>(entityOwner);

		GUI::Checkbox("Is Active", pointLightComp->IsActive);

		SColor color = pointLightComp->ColorAndIntensity;
		GUI::ColorPicker3("Color", color);
		SVector colorFloat = color.AsVector();
		pointLightComp->ColorAndIntensity = { colorFloat.X, colorFloat.Y, colorFloat.Z, pointLightComp->ColorAndIntensity.W };

		GUI::DragFloat("Intensity", pointLightComp->ColorAndIntensity.W, GUI::SliderSpeed);
		GUI::DragFloat("Range", pointLightComp->Range, GUI::SliderSpeed, 0.1f, 100.0f);
		
		I32 shadowmapStartIndex = STATIC_I32(pointLightComp->ShadowmapViews[0].ShadowmapViewportIndex);
		if (GUI::InputInt("Shadowmap Start Index", shadowmapStartIndex))
		{
			// NW: Would be nice to pull this directly from the rendermanager, or some form of common settings
			constexpr U16 maxShadowmapViews = 184;

			shadowmapStartIndex = UMath::Clamp(shadowmapStartIndex, 0, maxShadowmapViews - 6);
			pointLightComp->ShadowmapViews[0].ShadowmapViewportIndex = STATIC_U16(shadowmapStartIndex++);
			pointLightComp->ShadowmapViews[1].ShadowmapViewportIndex = STATIC_U16(shadowmapStartIndex++);
			pointLightComp->ShadowmapViews[2].ShadowmapViewportIndex = STATIC_U16(shadowmapStartIndex++);
			pointLightComp->ShadowmapViews[3].ShadowmapViewportIndex = STATIC_U16(shadowmapStartIndex++);
			pointLightComp->ShadowmapViews[4].ShadowmapViewportIndex = STATIC_U16(shadowmapStartIndex++);
			pointLightComp->ShadowmapViews[5].ShadowmapViewportIndex = STATIC_U16(shadowmapStartIndex);
		}

		if (STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(pointLightComp))
		{
			const SVector pos = transformComponent->Transform.GetMatrix().GetTranslation();
			GDebugDraw::AddSphere(pos, SVector::Zero, SVector(pointLightComp->Range), SColor::Magenta, 0.0f);
		}
    }
}
