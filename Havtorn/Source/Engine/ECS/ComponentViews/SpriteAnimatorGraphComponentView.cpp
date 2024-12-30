// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphComponentView.h"

#include "ECS/Components/SpriteAnimatorGraphComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SSpriteAnimatorGraphComponentView SSpriteAnimatorGraphComponentView::Context = {};

    SComponentViewResult SSpriteAnimatorGraphComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
        if (!ImGui::UUtils::TryOpenComponentView("SpriteAnimatorGraph"))
            return SComponentViewResult();

        SSpriteAnimatorGraphComponent* component = scene->GetComponent<SSpriteAnimatorGraphComponent>(entityOwner);
        
        SComponentViewResult result;
        if (ImGui::Button("Open Animator"))
        {
            result.Label = EComponentViewResultLabel::OpenAssetTool;
            result.ComponentViewed = component;
        }

        return result;
    }
}
