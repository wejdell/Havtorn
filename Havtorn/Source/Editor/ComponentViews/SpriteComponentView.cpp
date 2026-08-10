// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/SpriteComponent.h>
#include <Scene/Scene.h>
#include <Engine.h>
#include <Assets/AssetReference.h>

#include <GUI.h>

namespace Havtorn
{
    void Havtorn::SSpriteComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		SSpriteComponent* spriteComp = scene->GetComponent<SSpriteComponent>(entityOwner);

		GUI::ColorPicker4("Color", spriteComp->Color);
		GUI::DragFloat4("UVRect", spriteComp->UVRect, GUI::SliderSpeed);

		GUI::Text("Texture");

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->InspectAssetComponent(spriteComp, EAssetType::Texture, SAssetReference::ConvertToPointers(spriteComp->AssetReference));
    }
}
