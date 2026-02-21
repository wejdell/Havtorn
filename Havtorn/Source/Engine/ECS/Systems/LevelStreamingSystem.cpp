// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "LevelStreamingSystem.h"
#include "ECS/Components/LevelStreamingComponent.h"
#include "Engine.h"
#include "Scene/World.h"

namespace Havtorn
{
	CLevelStreamingSystem::CLevelStreamingSystem()
		: ISystem()
	{
		// TODO.NW: Need to figure out how to stream scenes in on worker threads. Maybe they can be loaded in on 
		// a separate resource that then gets added to the list of active scenes after. Just want that step to be fast.
	}

	void CLevelStreamingSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		if (SceneLoader == nullptr)
			return;

		std::vector<CHavtornStaticString<255>> queuedSceneNameRemovals;
		for (Ptr<CScene>& scene : scenes)
		{
			for (SLevelStreamingComponent* component : scene->GetComponents<SLevelStreamingComponent>())
			{
				if (component->ComponentLoadState == ELevelLoadState::Loading)
				{
					bool successfullyLoaded = false;
					for (SSceneState& state : component->SceneStates)
					{
						if (SceneLoader(state.SceneReference.FilePath))
						{
							successfullyLoaded = true;
							state.ScenePointer = GEngine::GetWorld()->GetActiveScenes().back().get();
						}
					}

					if (successfullyLoaded)
						component->ComponentLoadState = ELevelLoadState::Loaded;
					else
						component->ComponentLoadState = ELevelLoadState::Unloaded;
				}
				else if (component->ComponentLoadState == ELevelLoadState::Unloading)
				{
					for (SSceneState& state : component->SceneStates)
					{
						if (state.ScenePointer != nullptr)
							queuedSceneNameRemovals.push_back(state.ScenePointer->SceneName);
					}
					component->ComponentLoadState = ELevelLoadState::Unloaded;
				}
			}
		}

		for (const CHavtornStaticString<255>& sceneName : queuedSceneNameRemovals)
			GEngine::GetWorld()->RemoveScene(sceneName);
	}

	void CLevelStreamingSystem::BindSceneLoader(const std::function<bool(const std::string&)>& loadingFunction)
	{
		SceneLoader = loadingFunction;
	}
}
