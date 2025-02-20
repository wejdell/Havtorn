// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponentEditorContext.h"

#include "ECS/Components/DecalComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
	SDecalComponentEditorContext SDecalComponentEditorContext::Context = {};

	SComponentViewResult SDecalComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!ImGui::UUtils::TryOpenComponentView("Decal"))
			return SComponentViewResult();

		SDecalComponent* decalComp = scene->GetComponent<SDecalComponent>(entityOwner);

		ImGui::Checkbox("Render Albedo", &decalComp->ShouldRenderAlbedo);
		ImGui::Checkbox("Render Material", &decalComp->ShouldRenderMaterial);
		ImGui::Checkbox("Render Normal", &decalComp->ShouldRenderNormal);

		return { EComponentViewResultLabel::InspectAssetComponent, decalComp, 0 };
	}

	bool SDecalComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("Decal Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SDecalComponent>(entity);
		scene->AddComponentEditorContext(entity, &SDecalComponentEditorContext::Context);
		return true;
	}

	bool SDecalComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("X##3"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SDecalComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SDecalComponentEditorContext::Context);
		return true;
	}
}
