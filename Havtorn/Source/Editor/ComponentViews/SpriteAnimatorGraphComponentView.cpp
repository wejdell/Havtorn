// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphComponentView.h"

#include "ECS/Components/SpriteAnimatorGraphComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
    SComponentViewResult SSpriteAnimatorGraphComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
        SSpriteAnimatorGraphComponent* component = scene->GetComponent<SSpriteAnimatorGraphComponent>(entityOwner);
        
        SComponentViewResult result;
        if (GUI::Button("Open Animator"))
        {
            result.Label = EComponentViewResultLabel::OpenAssetTool;
            result.ComponentViewed = component;
        }

        return result;
    }
}
