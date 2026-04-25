// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteComponentEditorContext.h"

#include "ECS/Components/SpriteComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"
#include "Assets/AssetReference.h"
#include <GUI.h>

namespace Havtorn
{
	SSpriteComponentEditorContext SSpriteComponentEditorContext::Context = {};

    SComponentViewResult Havtorn::SSpriteComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		SSpriteComponent* spriteComp = scene->GetComponent<SSpriteComponent>(entityOwner);

		GUI::ColorPicker4("Color", spriteComp->Color);
		GUI::DragFloat4("UVRect", spriteComp->UVRect, GUI::SliderSpeed);

		GUI::Text("Texture");

		return { EComponentViewResultLabel::InspectAssetComponent, spriteComp, SAssetReference::ConvertToPointers(spriteComp->AssetReference), EAssetType::Texture };
    }

	bool SSpriteComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SSpriteComponent>(entity);
		scene->AddComponentEditorContext(entity, &SSpriteComponentEditorContext::Context);
		return true;
	}

	bool SSpriteComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SSpriteComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SSpriteComponentEditorContext::Context);
		return true;
	}
}
