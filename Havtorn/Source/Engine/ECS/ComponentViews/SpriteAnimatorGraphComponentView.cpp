// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphComponentView.h"

#include "ECS/Components/SpriteAnimatorGraphComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>

namespace Havtorn
{
    SComponentViewResult Havtorn::SSpriteAnimatorGraphComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
        SSpriteAnimatorGraphComponent* component = scene->GetComponent<SSpriteAnimatorGraphComponent>(entityOwner);
        
        SComponentViewResult result;
        if (ImGui::Button("Open Animator"))
        {
            result.Label = EComponentViewResultLabel::OpenSpriteAnimatorGraph;
            result.ComponentViewed = component;
        }

        return SComponentViewResult();
    }
}
