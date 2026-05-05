// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AudioEmitterComponentEditorContext.h"
#include "Engine.h"
#include "Assets/AssetRegistry.h"

#include "ECS/Components/AudioEmitterComponent.h"

#include <GUI.h>

namespace Havtorn
{
    SAudioEmitterComponentEditorContext SAudioEmitterComponentEditorContext::Context = {};

    SComponentViewResult SAudioEmitterComponentEditorContext::View(const SEntity& /*entityOwner*/, CScene* /*scene*/) const
    {
        return SComponentViewResult();
    }

    bool SAudioEmitterComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
    {
        scene->AddComponent<SAudioEmitterComponent>(entity);
        scene->AddComponentEditorContext(entity, &SAudioEmitterComponentEditorContext::Context);
        return true;
    }

    bool SAudioEmitterComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
    {
        scene->RemoveComponent<SAudioEmitterComponent>(entity);
        scene->RemoveComponentEditorContext(entity, &SAudioEmitterComponentEditorContext::Context);
        return true;
    }
}
