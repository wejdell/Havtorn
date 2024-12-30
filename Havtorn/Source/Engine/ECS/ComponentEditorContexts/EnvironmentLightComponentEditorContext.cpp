// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EnvironmentLightComponentEditorContext.h"

#include "ECS/Components/EnvironmentLightComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
	SEnvironmentLightComponentEditorContext SEnvironmentLightComponentEditorContext::Context = {};

    SComponentViewResult SEnvironmentLightComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!ImGui::UUtils::TryOpenComponentView("EnvironmentLight"))
			return SComponentViewResult();

		SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(entityOwner);

		ImGui::Text("Ambient Static Cubemap");
	
		return { EComponentViewResultLabel::InspectAssetComponent, environmentLightComp, 0 };
    }

	bool SEnvironmentLightComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("Environment Light Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SEnvironmentLightComponent>(entity);
		scene->AddComponentEditorContext(entity, &SEnvironmentLightComponentEditorContext::Context);
		return true;
	}

	bool SEnvironmentLightComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("X##5"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SEnvironmentLightComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SEnvironmentLightComponentEditorContext::Context);
		return true;
	}
}
