// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SequencerComponentEditorContext.h"

#include "ECS/Components/SequencerComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SSequencerComponentEditorContext SSequencerComponentEditorContext::Context = {};

    SComponentViewResult SSequencerComponentEditorContext::View(const SEntity& /*entityOwner*/, CScene* /*scene*/) const
    {
        // TODO.NR: Set up references to SequencerAssets?
        return SComponentViewResult();
    }

	bool SSequencerComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("Sequencer Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SSequencerComponent>(entity);
		scene->AddComponentEditorContext(entity, &SSequencerComponentEditorContext::Context);
		return true;
	}

	bool SSequencerComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("X##11"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SSequencerComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SSequencerComponentEditorContext::Context);
		return true;
	}
}
