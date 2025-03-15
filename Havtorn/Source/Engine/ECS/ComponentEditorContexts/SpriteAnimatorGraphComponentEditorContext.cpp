// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphComponentEditorContext.h"

#include "ECS/Components/SpriteAnimatorGraphComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>


namespace Havtorn
{
    SSpriteAnimatorGraphComponentEditorContext SSpriteAnimatorGraphComponentEditorContext::Context = {};

    SComponentViewResult SSpriteAnimatorGraphComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
        if (!GUI::TryOpenComponentView("SpriteAnimatorGraph"))
            return SComponentViewResult();

        SSpriteAnimatorGraphComponent* component = scene->GetComponent<SSpriteAnimatorGraphComponent>(entityOwner);
        
        SComponentViewResult result;
        if (GUI::Button("Open Animator"))
        {
            result.Label = EComponentViewResultLabel::OpenAssetTool;
            result.ComponentViewed = component;
        }

        return result;
    }

	bool SSpriteAnimatorGraphComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Sprite Animator Graph Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SSpriteAnimatorGraphComponent>(entity);
		scene->AddComponentEditorContext(entity, &SSpriteAnimatorGraphComponentEditorContext::Context);
		return true;
	}

	bool SSpriteAnimatorGraphComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##14"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SSpriteAnimatorGraphComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SSpriteAnimatorGraphComponentEditorContext::Context);
		return true;
	}
}
