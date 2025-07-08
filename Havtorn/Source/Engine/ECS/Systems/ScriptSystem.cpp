// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ScriptSystem.h"
#include "Scene/Scene.h"
#include "ECS/Components/ScriptComponent.h"
#include "HexRune/HexRune.h"
#include "HexRune/CoreNodes/CoreNodes.h"

namespace Havtorn
{
	CScriptSystem::CScriptSystem()
	{
		
	}

	void CScriptSystem::Update(CScene* scene)
	{
		//const F32 deltaTime = GTime::Dt();
		const std::vector<SScriptComponent*>& scriptComponents = scene->GetComponents<SScriptComponent>();

		// TODO.NW: Handle different script triggers, e.g. BeginPlay, Tick, OnCollisionTriggerEnter

		for (SScriptComponent* component : scriptComponents)
		{
			if (!component->IsValid())
				continue;

			if (!component->TriggerScript)
				continue;

			HexRune::SScript* script = component->Script;

			for (U64 i = 0; i < script->DataBindings.size(); i++)
			{
				auto& scriptBinding = script->DataBindings[i];
				const auto& componentBinding = component->DataBindings[i];

				scriptBinding.Data = componentBinding.Data;
			}

			script->TraverseScript(scene);

			for (U64 i = 0; i < script->DataBindings.size(); i++)
			{
				const auto& scriptBinding = script->DataBindings[i];
				auto& componentBinding = component->DataBindings[i];

				componentBinding.Data = scriptBinding.Data;
			}
		}
	}
}
