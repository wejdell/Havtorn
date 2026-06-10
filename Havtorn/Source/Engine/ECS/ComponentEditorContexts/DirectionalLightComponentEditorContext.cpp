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

		SVector direction = SVector(directionalLightComp->Direction.X, directionalLightComp->Direction.Y, directionalLightComp->Direction.Z);
		GUI::DragFloat3("Direction", direction, GUI::SliderSpeed);
		directionalLightComp->Direction = { direction.X, direction.Y, direction.Z, 0.0f };
		if (directionalLightComp->Direction.IsEqual(SVector4::Zero))
			directionalLightComp->Direction = SVector4(0.0f, 0.0f, 0.01f, 0.0f);

		GUI::DragFloat2("Shadow View Size", directionalLightComp->ShadowViewSize);
		GUI::DragFloat2("Shadow View Near and Far Plane", directionalLightComp->ShadowNearAndFarPlane);

		if (STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(directionalLightComp))
		{
			SVector pos = transformComponent->Transform.GetMatrix().GetTranslation();
			GDebugDraw::AddArrow(pos, pos + directionalLightComp->Direction.ToVector3(), SColor::Magenta, 0.0f, true);
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
