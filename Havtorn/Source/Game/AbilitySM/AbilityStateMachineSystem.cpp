// Copyright 2025 Team Havtorn. All Rights Reserved.
#include "AbilityStateMachineSystem.h"
#include <stack>

#define GETNAME(type) #type

namespace Havtorn
{
	CAbilityStateMachineSystem::CAbilityStateMachineSystem()
		: ISystem()
	{

	}

	void CAbilityStateMachineSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		for (auto& scene : scenes)
		{
			std::vector<SAbilityStateMachineComponent*> components = scene->GetComponents<SAbilityStateMachineComponent>();
			for (SAbilityStateMachineComponent* component : components)
			{
				std::vector<U64> activeAbilityGUIDs{};
				for (auto& activeAbility : component->ActiveAbilities)
					activeAbilityGUIDs.push_back(activeAbility->Guid);

				for (SAbility* ability : component->Abilites)
				{

					HV_LOG_INFO(GETNAME(*ability));
					bool isBlocked = false;
					for (auto& activeAbilityGUID : activeAbilityGUIDs)
					{
						if (component->AbilityBlockingMap.find(activeAbilityGUID) == component->AbilityBlockingMap.end())
							continue;

						isBlocked = IsBlocked(*ability, component->AbilityBlockingMap.at(activeAbilityGUID));

						if (isBlocked)
							break;
					}

					const bool isActive = std::ranges::find(component->ActiveAbilities, ability) != component->ActiveAbilities.end();

					const bool shouldRun = ability->ShouldRun(component);
					if (!isBlocked && !isActive && shouldRun)
					{
						ActivateAbility(*ability, component);
					}

					if ((isBlocked && isActive) || !shouldRun)
					{
						DeActivateAbility(*ability, component);
						continue;
					}

					if (!shouldRun)
						continue;

					ability->Execute();


					//!isActive && !isBlocked
					//	ActivateAbility(ability)

					//isActive && isBlocked
					//	DeActivateAbility(ability)

					//ShouldRun(ability)
					//	ability.Tick();

				}
			}
		}
	}

	//component->BlockerMap[&ability] 
	//std::find(component->ActiveBlockers.begin(), component->ActiveBlockers.end(), component->ActiveBlockers)		

	bool CAbilityStateMachineSystem::IsBlocked(const SAbility& ability, const std::vector<EAbilityTag>& blockingTags) const
	{
		//Return true if blockingTags contains any same elements as ability.Tags
		using std::ranges::any_of;
		return any_of(blockingTags, [&](const EAbilityTag& blockingTag)
					  {
						  return any_of(ability.Tags, [&](const EAbilityTag& tag)
										{
											return blockingTag == tag;
										});
					  });
	}

	bool CAbilityStateMachineSystem::IsActive(const SAbility& ability, SAbilityStateMachineComponent* component) const
	{
		for (const SAbility* activeAbility : component->ActiveAbilities)
			if (ability.Guid == activeAbility->Guid)
				return true;

		return false;
	}

	void CAbilityStateMachineSystem::DeActivateAbility(SAbility& ability, SAbilityStateMachineComponent* component)
	{
		for (U32 i = 0; i < component->ActiveAbilities.size(); i++)
		{
			SAbility* activeAbility = component->ActiveAbilities[i];
			if (activeAbility->Guid == ability.Guid)
			{
				//TODO.AS [DONE?]
				// Remove tag from component->BlockingTags 
				component->AbilityBlockingMap[ability.Guid] = {};


				//component->Abilites.push_back(component->ActiveAbilities[i]);
				component->ActiveAbilities[i] = component->ActiveAbilities[component->ActiveAbilities.size() - 1];
				component->ActiveAbilities.pop_back();
				break;
			}
		}
	}

	void CAbilityStateMachineSystem::ActivateAbility(SAbility& ability, SAbilityStateMachineComponent* component)
	{
		for (U32 i = 0; i < component->Abilites.size(); i++)
		{
			SAbility* activeAbility = component->Abilites[i];
			if (activeAbility->Guid == ability.Guid)
			{
				//Found this In-Ability, Adding it to Active and Removing from Abilities
				component->ActiveAbilities.push_back(component->Abilites[i]);
				component->ActiveAbilities.back()->Init(component);

				//component->Abilites[i] = component->Abilites[component->Abilites.size() - 1];
				//component->Abilites.pop_back();

				break;
			}
		}
	}



	Locomotion::Locomotion()
		: SAbility()
	{
	}

	Locomotion::~Locomotion()
	{
	}

	SAbility::SAbility()
		: Guid(Havtorn::UGUIDManager::Generate())
	{
	}

	SAbility::~SAbility()
	{
	}


	void Locomotion::DeInit(SAbilityStateMachineComponent*)
	{
	}

	bool Locomotion::ShouldRun(SAbilityStateMachineComponent*)
	{
		return true;
	}

	void Locomotion::Execute()
	{



	}


	void Locomotion::Init(SAbilityStateMachineComponent* /*component*/)
	{
		Tags.push_back(EAbilityTag::Move);
	}

	void CameraControl::Init(SAbilityStateMachineComponent* /*component*/)
	{
		Tags.push_back(EAbilityTag::Camera);
	}

	void Jump::Init(SAbilityStateMachineComponent* /*component*/)
	{
		Tags.push_back(EAbilityTag::Agile);
	}

	void Falling::Init(SAbilityStateMachineComponent* component)
	{
		Tags.push_back(EAbilityTag::Gravity);
		component->AbilityBlockingMap[Guid].push_back(EAbilityTag::Move);
		component->AbilityBlockingMap[Guid].push_back(EAbilityTag::Agile);
	}

	bool Jump::ShouldRun(SAbilityStateMachineComponent* component)
	{
		return component->Data.IsJumping;
	}

	bool Falling::ShouldRun(SAbilityStateMachineComponent* component)
	{
		return component->Data.IsFalling;
	}

	void Falling::DeInit(SAbilityStateMachineComponent* component)
	{
		component->AbilityBlockingMap[Guid].clear();
	}


}

