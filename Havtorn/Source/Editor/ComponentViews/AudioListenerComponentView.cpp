// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AudioListenerComponentView.h"
#include "Scene/Scene.h"

#include "ECS/Components/AudioListenerComponent.h"

#include <GUI.h>

namespace Havtorn
{
    SAudioListenerComponentView SAudioListenerComponentView::Context = {};

    SComponentViewResult SAudioListenerComponentView::View(const SEntity& /*entityOwner*/, CScene* /*scene*/) const
    {
        return SComponentViewResult();
    }
}
