// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AbilitySystem.h"
#include "ECS/Components/AbilityComponent.h"

#include <GameplayTags/GameplayTagManager.h>

namespace Havtorn 
{
	void CAbilitySystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		for (Ptr<CScene>& scene : scenes)
		{
			std::vector<SAbilityComponent*> components = scene->GetComponents<SAbilityComponent>();

			// TODO.NW: Read this from input/instructions on other components, or hook up requests to this system?
			SGameplayTagContainer requestedEnters;
			SGameplayTagContainer requestedExits;

			for (SAbilityComponent* component : components)
			{
				std::vector<SGameplayTag> tagsToAdd;
				std::vector<SGameplayTag> tagsToRemove;
				const SGameplayTagContainer& componentTags = component->TagContainer;

				for (const SAbilityState& ability : component->Abilities)
				{
					const bool isActive = GGameplayTagManager::AnyTagsMatch(ability.AbilityTag, componentTags, false);
					const bool requestingEntry = GGameplayTagManager::AnyTagsMatch(ability.AbilityTag, requestedEnters, false);
					const bool requestingExit = GGameplayTagManager::AnyTagsMatch(ability.AbilityTag, requestedExits, false);

					const bool canActivate = GGameplayTagManager::NoTagsMatch(ability.ActivationBlockingTags, componentTags) && GGameplayTagManager::AllTagsMatch(ability.ActivationRequiredTags, componentTags);
					const bool canContinue = GGameplayTagManager::NoTagsMatch(ability.ContinuousBlockingTags, componentTags) && GGameplayTagManager::AllTagsMatch(ability.ContinuousRequiredTags, componentTags);

					const bool startAbility = !isActive && requestingEntry && canActivate;
					const bool stopAbility = isActive && (requestingExit || !canContinue);
					const bool tickAbility = isActive && !requestingExit && canContinue;

					if (!startAbility && !stopAbility && !tickAbility)
						continue;

					const SScriptAsset* abilityScript = GEngine::GetAssetRegistry()->RequestAssetData<SScriptAsset>(ability.ScriptReference, component->Owner.GUID);
					if (abilityScript == nullptr)
						continue;

					HexRune::SScript* script = abilityScript->Script.get();

					if (startAbility)
					{	
						// TODO.NW: Do we need data bindings on these scripts, or can we provide some base ones?				
						if (script->HasNode(HexRune::BeginPlayNodeID))
							script->TraverseFromNode(HexRune::BeginPlayNodeID, scene.get());

						// TODO.NW: Make requests to add tags, so we can track instigators
						for (const SGameplayTag& tag : ability.ActivationGrantedTags.Tags)
							tagsToAdd.emplace_back(tag);

						// TODO.NW: Request exits for all abilities with ability.ActivationCancelTags, next tick	
					}
					else if (stopAbility)
					{
						if (script->HasNode(HexRune::EndPlayNodeID))
							script->TraverseFromNode(HexRune::EndPlayNodeID, scene.get());

						// TODO.NW: Make requests to add tags, so we can track instigators
						for (const SGameplayTag& tag : ability.ActivationGrantedTags.Tags)
							tagsToRemove.emplace_back(tag);	
					}
					else if (tickAbility)
					{
						if (script->HasNode(HexRune::TickNodeID))
							script->TraverseFromNode(HexRune::TickNodeID, scene.get());	
					}
				}

				// TODO.NW: Track these changes with instigators/requesters
				for (const SGameplayTag& tag : tagsToAdd)
					component->TagContainer.AddTag(tag);

				for (const SGameplayTag& tag : tagsToRemove)
					component->TagContainer.RemoveTag(tag);
			}
		}
	}
}
