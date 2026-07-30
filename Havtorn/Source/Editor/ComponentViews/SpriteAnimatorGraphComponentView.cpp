// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/SpriteAnimatorGraphComponent.h>
#include <Scene/Scene.h>

#include <GUI.h>

namespace Havtorn
{
    void SSpriteAnimatorGraphComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
        SSpriteAnimatorGraphComponent* component = scene->GetComponent<SSpriteAnimatorGraphComponent>(entityOwner);
        
        if (GUI::Button("Open Animator"))
        {
            CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
            inspector->OpenAssetTool(component);
        }
    }
}
