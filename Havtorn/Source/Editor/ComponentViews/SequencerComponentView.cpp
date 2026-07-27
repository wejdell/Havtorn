// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SequencerComponentView.h"

#include "ECS/Components/SequencerComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>


namespace Havtorn
{
    SComponentViewResult SSequencerComponentView::View(const SEntity& /*entityOwner*/, CScene* /*scene*/) const
    {
        // TODO.NR: Set up references to SequencerAssets?
        return SComponentViewResult();
    }
}
