// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponentEditorContext.h"

#include "ECS/Components/DecalComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"

#include <GUI.h>


namespace Havtorn
{
	SDecalComponentEditorContext SDecalComponentEditorContext::Context = {};

	SComponentViewResult SDecalComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("Decal"))
			return SComponentViewResult();

		SDecalComponent* decalComp = scene->GetComponent<SDecalComponent>(entityOwner);

		GUI::Checkbox("Render Albedo", &decalComp->ShouldRenderAlbedo);
		GUI::Checkbox("Render Material", &decalComp->ShouldRenderMaterial);
		GUI::Checkbox("Render Normal", &decalComp->ShouldRenderNormal);

		return { EComponentViewResultLabel::InspectAssetComponent, decalComp, 0 };
	}

	bool SDecalComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Decal Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SDecalComponent>(entity);
		scene->AddComponentEditorContext(entity, &SDecalComponentEditorContext::Context);
		return true;
	}

	bool SDecalComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##3"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SDecalComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SDecalComponentEditorContext::Context);
		return true;
	}
}
