// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DirectionalLightComponentEditorContext.h"

#include "ECS/Components/DirectionalLightComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	SDirectionalLightComponentEditorContext SDirectionalLightComponentEditorContext::Context = {};

    SComponentViewResult SDirectionalLightComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("DirectionalLight"))
			return SComponentViewResult();

		SDirectionalLightComponent* directionalLightComp = scene->GetComponent<SDirectionalLightComponent>(entityOwner);

		Havtorn::F32 colorData[3] = { directionalLightComp->Color.X, directionalLightComp->Color.Y, directionalLightComp->Color.Z };
		GUI::ColorPicker3("Color", colorData);
		directionalLightComp->Color.X = colorData[0];
		directionalLightComp->Color.Y = colorData[1];
		directionalLightComp->Color.Z = colorData[2];

		SVector direction = SVector(directionalLightComp->Direction.X, directionalLightComp->Direction.Y, directionalLightComp->Direction.Z);
		GUI::DragFloat3("Direction", direction, GUI::SliderSpeed);
		directionalLightComp->Direction = { direction.X, direction.Y, direction.Z, 0.0f };
		if (directionalLightComp->Direction.IsEqual(SVector4::Zero))
			directionalLightComp->Direction = SVector4(0.0f, 0.0f, 0.01f, 0.0f);

		GUI::DragFloat("Intensity", &directionalLightComp->Color.W, GUI::SliderSpeed);

        return SComponentViewResult();
    }

	bool SDirectionalLightComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Directional Light Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SDirectionalLightComponent>(entity);
		scene->AddComponentEditorContext(entity, &SDirectionalLightComponentEditorContext::Context);
		return true;
	}

	bool SDirectionalLightComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##4"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SDirectionalLightComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SDirectionalLightComponentEditorContext::Context);
		return true;
	}
}
