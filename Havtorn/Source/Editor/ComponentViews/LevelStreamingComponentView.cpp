// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "LevelStreamingComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/LevelStreamingComponent.h>
#include <ECS/Components/MetaDataComponent.h>
#include <Scene/Scene.h>
#include <Assets/AssetRegistry.h>
#include <Engine.h>

#include <GUI.h>

namespace Havtorn
{
	void SLevelStreamingComponentView::View(const SEntity& entityOwner, CScene* scene) const
	{
		SLevelStreamingComponent* component = scene->GetComponent<SLevelStreamingComponent>(entityOwner);
		if (!SComponent::IsValid(component))
			return;

		GUI::TextDisabled("Load Status: %s", magic_enum::enum_name<ELevelLoadState>(component->ComponentLoadState).data());
		if (component->ComponentLoadState == ELevelLoadState::Unloaded)
		{
			GUI::SameLine();
			if (GUI::Button("Load Scenes"))
				component->ComponentLoadState = ELevelLoadState::Loading;
		}
		else if (component->ComponentLoadState == ELevelLoadState::Loaded)
		{
			GUI::SameLine();
			if (GUI::Button("Unload Scenes"))
				component->ComponentLoadState = ELevelLoadState::Unloading;
		}
		GUI::Separator();
		GUI::TextDisabled("Scenes");

		std::vector<SAssetReference*> assetReferences;

		// TODO.NW: Would probably want a solution to unload the associated scenes when these components get removed or changes
		// But how do we then handle overlaps between different components?
		if (component->ComponentLoadState == ELevelLoadState::Unloaded)
		{
			GUI::SameLine();
			if (GUI::Button("Add"))
				component->SceneStates.push_back(SSceneState());

			GUI::SameLine();
			if (GUI::Button("Clear"))
				component->SceneStates.clear();

			if (component->SceneStates.empty())
				return;

			I32 elementToRemoveIndex = -1;

			if (elementToRemoveIndex != -1)
				component->SceneStates.erase(component->SceneStates.begin() + elementToRemoveIndex);

			for (auto& state : component->SceneStates)
				assetReferences.push_back(&state.SceneReference);
		}
		else
		{
			GUI::SameLine();
			GUI::TextDisabled("| Unload Scenes to change assets |");
			GUI::Separator();

			for (auto& state : component->SceneStates)
			{
				const std::string sceneName = state.ScenePointer ? state.ScenePointer->SceneName.AsString().c_str() : "NullAsset";
				GUI::TextDisabled(sceneName.c_str());
			}
		}

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->InspectAssetComponent(component, EAssetType::Scene, assetReferences);
	}

	U8 SLevelStreamingComponentView::GetSortingPriority() const
	{
		return 4;
	}
}
