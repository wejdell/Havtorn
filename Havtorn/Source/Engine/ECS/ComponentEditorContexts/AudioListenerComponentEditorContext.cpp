// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AudioListenerComponentEditorContext.h"
#include "Scene/Scene.h"

#include "ECS/Components/AudioListenerComponent.h"

#include <GUI.h>

namespace Havtorn
{
    SAudioListenerComponentEditorContext SAudioListenerComponentEditorContext::Context = {};

    SComponentViewResult SAudioListenerComponentEditorContext::View(const SEntity& /*entityOwner*/, CScene* /*scene*/) const
    {
        return SComponentViewResult();
    }

    bool SAudioListenerComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
    {
        scene->AddComponent<SAudioListenerComponent>(entity);
        scene->AddComponentEditorContext(entity, &SAudioListenerComponentEditorContext::Context);
        return true;
    }

    bool SAudioListenerComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
    {
        scene->RemoveComponent<SAudioListenerComponent>(entity);
        scene->RemoveComponentEditorContext(entity, &SAudioListenerComponentEditorContext::Context);
        return true;
    }
}
