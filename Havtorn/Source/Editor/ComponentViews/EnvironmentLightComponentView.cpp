// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EnvironmentLightComponentView.h"

#include "ECS/Components/EnvironmentLightComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"
#include "Assets/AssetReference.h"

#include <GUI.h>


namespace Havtorn
{
	SEnvironmentLightComponentView SEnvironmentLightComponentView::Context = {};

    SComponentViewResult SEnvironmentLightComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(entityOwner);

		GUI::Checkbox("Is Active", environmentLightComp->IsActive);
		GUI::Text("Ambient Static Cubemap");
	
		return { EComponentViewResultLabel::InspectAssetComponent, environmentLightComp, SAssetReference::ConvertToPointers(environmentLightComp->AssetReference), EAssetType::TextureCube };
    }
}
