// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteComponentView.h"

#include "ECS/Components/SpriteComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult Havtorn::SSpriteComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		if (!ImGui::UUtils::TryOpenComponentView("Sprite"))
			return SComponentViewResult();

		SSpriteComponent* spriteComp = scene->GetComponent<SSpriteComponent>(entityOwner);

		SVector4 colorFloat = spriteComp->Color.AsVector4();
		Havtorn::F32 color[4] = { colorFloat.X, colorFloat.Y, colorFloat.Z, colorFloat.W };
		Havtorn::F32 rect[4] = { spriteComp->UVRect.X, spriteComp->UVRect.Y, spriteComp->UVRect.Z, spriteComp->UVRect.W };

		ImGui::ColorPicker4("Color", color);
		ImGui::DragFloat4("UVRect", rect, ImGui::UUtils::SliderSpeed);

		spriteComp->Color = SVector4(color[0], color[1], color[2], color[3]);
		spriteComp->UVRect = { rect[0], rect[1], rect[2], rect[3] };

		ImGui::Text("Texture");

		return { EComponentViewResultLabel::InspectAssetComponent, spriteComp, 0 };
    }
}
