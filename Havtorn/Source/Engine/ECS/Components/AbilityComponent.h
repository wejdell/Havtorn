// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

#include "GameplayTags/GameplayTag.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
	enum class EAbilityState
	{
		Inactive,
		Active
	};

	struct SAbilityState
	{
		// If present, block activation
		SGameplayTagContainer ActivationBlockingTags;

		// Required for activation
		SGameplayTagContainer ActivationRequiredTags;
			
		// When activated, apply these tags
		SGameplayTagContainer ActivationGrantedTags;

		// When activated, cancel abilities with these tags
		SGameplayTagContainer ActivationCancelTags;
		
		// If not present, ability will end
		SGameplayTagContainer ContinuousRequiredTags;

		// If present, ability will end
		SGameplayTagContainer ContinuousBlockingTags;
	
		SGameplayTag AbilityTag;
		SAssetReference ScriptReference;
		EAbilityState State = EAbilityState::Inactive;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
	};

	struct SAbilityComponent : public SComponent
	{
		SAbilityComponent() = default;
		SAbilityComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{
		}

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		std::vector<SAbilityState> Abilities;
		SGameplayTagContainer TagContainer;
	};
}
