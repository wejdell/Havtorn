// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EnvironmentLightComponentView.h"

#include "ECS/Components/EnvironmentLightComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult SEnvironmentLightComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		if (!ImGui::UUtils::TryOpenComponentView("EnvironmentLight"))
			return SComponentViewResult();

		SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(entityOwner);

		ImGui::Text("Ambient Static Cubemap");
	
		return { EComponentViewResultLabel::InspectAssetComponent, environmentLightComp, 0 };
    }
}
