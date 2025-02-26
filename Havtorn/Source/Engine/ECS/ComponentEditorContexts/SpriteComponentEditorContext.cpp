// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteComponentEditorContext.h"

#include "ECS/Components/SpriteComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"

#include <GUI.h>

namespace Havtorn
{
	SSpriteComponentEditorContext SSpriteComponentEditorContext::Context = {};

    SComponentViewResult Havtorn::SSpriteComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("Sprite"))
			return SComponentViewResult();

		SSpriteComponent* spriteComp = scene->GetComponent<SSpriteComponent>(entityOwner);

		SVector4 colorFloat = spriteComp->Color.AsVector4();
		Havtorn::F32 color[4] = { colorFloat.X, colorFloat.Y, colorFloat.Z, colorFloat.W };
		Havtorn::F32 rect[4] = { spriteComp->UVRect.X, spriteComp->UVRect.Y, spriteComp->UVRect.Z, spriteComp->UVRect.W };

		GUI::ColorPicker4("Color", color);
		GUI::DragFloat4("UVRect", rect, GUI::SliderSpeed);

		spriteComp->Color = SVector4(color[0], color[1], color[2], color[3]);
		spriteComp->UVRect = { rect[0], rect[1], rect[2], rect[3] };

		GUI::Text("Texture");

		return { EComponentViewResultLabel::InspectAssetComponent, spriteComp, 0 };
    }

	bool SSpriteComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Sprite Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SSpriteComponent>(entity);
		scene->AddComponentEditorContext(entity, &SSpriteComponentEditorContext::Context);
		return true;
	}

	bool SSpriteComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##15"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SSpriteComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SSpriteComponentEditorContext::Context);
		return true;
	}
}
