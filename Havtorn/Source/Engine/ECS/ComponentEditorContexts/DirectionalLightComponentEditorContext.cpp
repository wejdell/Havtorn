// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DirectionalLightComponentEditorContext.h"

#include "ECS/Components/DirectionalLightComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

#include "Graphics/Debug/DebugDrawUtility.h"

#include <GUI.h>

namespace Havtorn
{
	SDirectionalLightComponentEditorContext SDirectionalLightComponentEditorContext::Context = {};

    SComponentViewResult SDirectionalLightComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		SDirectionalLightComponent* directionalLightComp = scene->GetComponent<SDirectionalLightComponent>(entityOwner);

		GUI::Checkbox("Is Active", directionalLightComp->IsActive);

		SColor color = directionalLightComp->Color;
		GUI::ColorPicker3("Color", color);
		SVector colorFloat = color.AsVector();
		directionalLightComp->Color = { colorFloat.X, colorFloat.Y, colorFloat.Z, directionalLightComp->Color.W };

		GUI::DragFloat2("Shadow View Size", directionalLightComp->ShadowViewSize);
		GUI::DragFloat2("Shadow View Near and Far Plane", directionalLightComp->ShadowNearAndFarPlane);

		I32 shadowmapStartIndex = STATIC_I32(directionalLightComp->ShadowmapView.ShadowmapViewportIndex);
		if (GUI::InputInt("Shadowmap Index", shadowmapStartIndex))
		{
			// NW: Would be nice to pull this directly from the rendermanager, or some form of common settings
			constexpr U16 maxShadowmapViews = 184;

			shadowmapStartIndex = UMath::Clamp(shadowmapStartIndex, 0, maxShadowmapViews - 1);
			directionalLightComp->ShadowmapView.ShadowmapViewportIndex = STATIC_U16(shadowmapStartIndex);
		}

		if (STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(directionalLightComp))
		{
			const SVector pos = transformComponent->Transform.GetMatrix().GetTranslation();
			GDebugDraw::AddArrow(pos, pos + directionalLightComp->Direction.ToVector3(), SColor::Magenta, 0.0f, true, 0.01f);
		}

		GUI::DragFloat("Intensity", directionalLightComp->Color.W, GUI::SliderSpeed);

        return SComponentViewResult();
    }

	bool SDirectionalLightComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SDirectionalLightComponent>(entity);
		scene->AddComponentEditorContext(entity, &SDirectionalLightComponentEditorContext::Context);
		return true;
	}

	bool SDirectionalLightComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SDirectionalLightComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SDirectionalLightComponentEditorContext::Context);
		return true;
	}
}
