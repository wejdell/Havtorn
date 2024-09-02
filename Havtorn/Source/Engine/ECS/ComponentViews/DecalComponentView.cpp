// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponentView.h"

#include "ECS/Components/DecalComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
	SComponentViewResult SDecalComponentView::View(const SEntity& entityOwner, CScene* scene)
	{
		if (!ImGui::UUtils::TryOpenComponentView("Decal"))
			return SComponentViewResult();

		SDecalComponent* decalComp = scene->GetComponent<SDecalComponent>(entityOwner);

		ImGui::Checkbox("Render Albedo", &decalComp->ShouldRenderAlbedo);
		ImGui::Checkbox("Render Material", &decalComp->ShouldRenderMaterial);
		ImGui::Checkbox("Render Normal", &decalComp->ShouldRenderNormal);

		return { EComponentViewResultLabel::InspectAssetComponent, decalComp, 0 };
	}
}
