// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once


namespace Havtorn
{
	enum class EAbilityTag
	{
		Move,
		Camera,
		Agile,
		Gravity,
	};

	struct SAbilityStateMachineComponent;
	struct SAbility;


	struct SBlackBoard
	{
		bool IsFalling = false;
		bool IsJumping = false;
	};

	struct SAbilityStateMachineComponent : SComponent
	{
		SAbilityStateMachineComponent() = default;
		SAbilityStateMachineComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		std::vector<SAbility*> Abilites;
		std::vector<SAbility*> ActiveAbilities;
		std::vector<EAbilityTag> BlockingTags;
		std::unordered_map<U64, std::vector<EAbilityTag>> AbilityBlockingMap;
		std::unordered_map<U64, CHavtornStaticString<255>> AbilityNameMap;
		SBlackBoard Data;

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;
	};
}