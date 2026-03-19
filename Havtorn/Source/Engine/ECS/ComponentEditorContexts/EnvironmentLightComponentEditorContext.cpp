// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EnvironmentLightComponentEditorContext.h"

#include "ECS/Components/EnvironmentLightComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"
#include "Assets/AssetReference.h"

#include <GUI.h>


namespace Havtorn
{
	SEnvironmentLightComponentEditorContext SEnvironmentLightComponentEditorContext::Context = {};

    SComponentViewResult SEnvironmentLightComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(entityOwner);

		GUI::Checkbox("Is Active", environmentLightComp->IsActive);
		GUI::Text("Ambient Static Cubemap");
	
		return { EComponentViewResultLabel::InspectAssetComponent, environmentLightComp, SAssetReference::ConvertToPointers(environmentLightComp->AssetReference), EAssetType::TextureCube };
    }

	bool SEnvironmentLightComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SEnvironmentLightComponent>(entity);
		scene->AddComponentEditorContext(entity, &SEnvironmentLightComponentEditorContext::Context);
		return true;
	}

	bool SEnvironmentLightComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SEnvironmentLightComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SEnvironmentLightComponentEditorContext::Context);
		return true;
	}
}
