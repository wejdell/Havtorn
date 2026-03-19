// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponentEditorContext.h"

#include "ECS/Components/DecalComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"
#include "Assets/AssetReference.h"

#include <GUI.h>


namespace Havtorn
{
	SDecalComponentEditorContext SDecalComponentEditorContext::Context = {};

	SComponentViewResult SDecalComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		SDecalComponent* decalComp = scene->GetComponent<SDecalComponent>(entityOwner);

		GUI::Checkbox("Render Albedo", decalComp->ShouldRenderAlbedo);
		GUI::Checkbox("Render Material", decalComp->ShouldRenderMaterial);
		GUI::Checkbox("Render Normal", decalComp->ShouldRenderNormal);

		return { EComponentViewResultLabel::InspectAssetComponent, decalComp, SAssetReference::ConvertToPointers(decalComp->AssetReferences), EAssetType::Texture};
	}

	bool SDecalComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SDecalComponent>(entity);
		scene->AddComponentEditorContext(entity, &SDecalComponentEditorContext::Context);
		return true;
	}

	bool SDecalComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SDecalComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SDecalComponentEditorContext::Context);
		return true;
	}
}
