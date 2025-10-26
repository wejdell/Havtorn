// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ScriptSystem.h"
#include "Scene/World.h"
#include "Scene/Scene.h"
#include "Assets/AssetRegistry.h"
#include "ECS/Components/ScriptComponent.h"
#include "HexRune/HexRune.h"
#include "HexRune/CoreNodes/CoreNodes.h"

namespace Havtorn
{
	CScriptSystem::CScriptSystem(CWorld* world)
	{
		world->OnBeginPlayDelegate.AddMember(this, &CScriptSystem::OnBeginPlay);
		world->OnEndPlayDelegate.AddMember(this, &CScriptSystem::OnEndPlay);
		world->OnBeginOverlap.AddMember(this, &CScriptSystem::OnBeginOverlap);
		world->OnEndOverlap.AddMember(this, &CScriptSystem::OnEndOverlap);
	}

	void CScriptSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		for (Ptr<CScene>& scene : scenes)
		{
			const std::vector<SScriptComponent*>& scriptComponents = scene->GetComponents<SScriptComponent>();

			SChangePlayModeData* beginPlayData = nullptr;
			SChangePlayModeData* endPlayData = nullptr;
			for (auto& change : PlayModeChanges)
			{
				if (change.Scene == scene.get())
					change.BeganPlay ? beginPlayData = &change : endPlayData = &change;
			}

			for (SScriptComponent* component : scriptComponents)
			{
				if (!SComponent::IsValid(component) || !component->AssetReference.IsValid())
					continue;

				HexRune::SScript* script = nullptr;
				script = GEngine::GetAssetRegistry()->RequestGameAssetData(script, component->AssetReference, component->Owner.GUID);

				if (component->DataBindings.size() != script->DataBindings.size())
				{
					for (U64 i = 0; i < script->DataBindings.size(); i++)
					{
						auto& scriptBinding = script->DataBindings[i];
						if (component->DataBindings.size() <= i)
							component->DataBindings.emplace_back(scriptBinding);
					}
					component->DataBindings.resize(script->DataBindings.size());
				}

				for (U64 i = 0; i < script->DataBindings.size(); i++)
				{
					auto& scriptBinding = script->DataBindings[i];
					const auto& componentBinding = component->DataBindings[i];

					scriptBinding.Data = componentBinding.Data;
				}

				SOverlapData* beginOverlap = nullptr;
				SOverlapData* endOverlap = nullptr;
				for (auto& overlap : Overlaps)
				{
					if (overlap.Scene == scene.get() && overlap.TriggerEntity == component->Owner)
						overlap.BeganOverlap ? beginOverlap = &overlap : endOverlap = &overlap;
				}

				if (beginOverlap != nullptr && script->HasNode(HexRune::OnBeginOverlapNodeID))
				{
					HexRune::SNode* node = script->GetNode(HexRune::OnBeginOverlapNodeID);
					node->SetDataOnPin(&node->Outputs[1], beginOverlap->TriggerEntity);
					node->SetDataOnPin(&node->Outputs[2], beginOverlap->OtherEntity);
					script->TraverseFromNode(node, scene.get());
					std::erase_if(Overlaps, [beginOverlap](SOverlapData& data) { return data == *beginOverlap; });
				}

				if (endOverlap != nullptr && script->HasNode(HexRune::OnEndOverlapNodeID))
				{
					HexRune::SNode* node = script->GetNode(HexRune::OnEndOverlapNodeID);
					node->SetDataOnPin(&node->Outputs[1], endOverlap->TriggerEntity);
					node->SetDataOnPin(&node->Outputs[2], endOverlap->OtherEntity);
					script->TraverseFromNode(node, scene.get());
					std::erase_if(Overlaps, [endOverlap](SOverlapData& data) { return data == *endOverlap; });
				}

				if (beginPlayData != nullptr && script->HasNode(HexRune::BeginPlayNodeID))
					script->TraverseFromNode(HexRune::BeginPlayNodeID, scene.get());

				if (script->HasNode(HexRune::TickNodeID))
					script->TraverseFromNode(HexRune::TickNodeID, scene.get());

				if (endPlayData != nullptr && script->HasNode(HexRune::EndPlayNodeID))
					script->TraverseFromNode(HexRune::EndPlayNodeID, scene.get());
			
				for (U64 i = 0; i < script->DataBindings.size(); i++)
				{
					const auto& scriptBinding = script->DataBindings[i];
					auto& componentBinding = component->DataBindings[i];

					componentBinding.Data = scriptBinding.Data;
				}
			}

			std::erase_if(PlayModeChanges, [beginPlayData](SChangePlayModeData& data) { return beginPlayData != nullptr && data == *beginPlayData; });
			std::erase_if(PlayModeChanges, [endPlayData](SChangePlayModeData& data) { return endPlayData != nullptr && data == *endPlayData; });
		}
	}

	void CScriptSystem::OnBeginPlay(std::vector<Ptr<CScene>>& scenes)
	{
		for (Ptr<CScene>& scene : scenes)
			PlayModeChanges.push_back(SChangePlayModeData(scene.get(), true));
	}

	void CScriptSystem::OnEndPlay(std::vector<Ptr<CScene>>& scenes)
	{
		for (Ptr<CScene>& scene : scenes)
			PlayModeChanges.push_back(SChangePlayModeData(scene.get(), false));
	}

	void CScriptSystem::OnBeginOverlap(CScene* scene, const SEntity triggerEntity, const SEntity otherEntity)
	{
		Overlaps.push_back(SOverlapData(scene, triggerEntity, otherEntity, true));
	}

	void CScriptSystem::OnEndOverlap(CScene* scene, const SEntity triggerEntity, const SEntity otherEntity)
	{
		Overlaps.push_back(SOverlapData(scene, triggerEntity, otherEntity, false));
	}
}
