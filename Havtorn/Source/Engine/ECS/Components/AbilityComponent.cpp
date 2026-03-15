// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "AbilityComponent.h"
#include "Engine.h"

namespace Havtorn 
{
	U32 SAbilityState::GetSize() const
	{
		U32 size = 0;
		size += ActivationBlockingTags.GetSize();
		size += ActivationRequiredTags.GetSize();
		size += ActivationGrantedTags.GetSize();
		size += ActivationCancelTags.GetSize();
		size += ContinuousRequiredTags.GetSize();
		size += ContinuousBlockingTags.GetSize();

		size += AbilityTag.GetSize();
		size += ScriptReference.GetSize();

		// NW: No need to serialize State

		return size;
	}

	void SAbilityState::Serialize(char* toData, U64& pointerPosition) const
	{
		ActivationBlockingTags.Serialize(toData, pointerPosition);
		ActivationRequiredTags.Serialize(toData, pointerPosition);
		ActivationGrantedTags.Serialize(toData, pointerPosition);
		ActivationCancelTags.Serialize(toData, pointerPosition);
		ContinuousRequiredTags.Serialize(toData, pointerPosition);
		ContinuousBlockingTags.Serialize(toData, pointerPosition);

		AbilityTag.Serialize(toData, pointerPosition);
		ScriptReference.Serialize(toData, pointerPosition);

		// NW: No need to serialize State
	}

	void SAbilityState::Deserialize(const char* fromData, U64& pointerPosition)
	{
		ActivationBlockingTags.Deserialize(fromData, pointerPosition);
		ActivationRequiredTags.Deserialize(fromData, pointerPosition);
		ActivationGrantedTags.Deserialize(fromData, pointerPosition);
		ActivationCancelTags.Deserialize(fromData, pointerPosition);
		ContinuousRequiredTags.Deserialize(fromData, pointerPosition);
		ContinuousBlockingTags.Deserialize(fromData, pointerPosition);

		AbilityTag.Deserialize(fromData, pointerPosition);
		ScriptReference.Deserialize(fromData, pointerPosition);

		// NW: No need to serialize State
	}

	void SAbilityComponent::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(Owner, toData, pointerPosition);

		SerializeData(STATIC_U32(Abilities.size()), toData, pointerPosition);
		for (const SAbilityState& ability : Abilities)
			ability.Serialize(toData, pointerPosition);
	}

	void SAbilityComponent::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(Owner, fromData, pointerPosition);

		U32 abilitysCount = 0;
		DeserializeData(abilitysCount, fromData, pointerPosition);
		for (U32 i = 0; i < abilitysCount; i++)
		{
			SAbilityState ability = {};
			ability.Deserialize(fromData, pointerPosition);
			Abilities.emplace_back(ability);
		}
	}

	U32 SAbilityComponent::GetSize() const
	{
		U32 size = 0;

		size += GetDataSize(Owner);

		size += sizeof(U32);
		for (const SAbilityState& ability : Abilities)
			size += ability.GetSize();

		return size;
	}

	void SAbilityComponent::IsDeleted(CScene* /*fromScene*/)
	{
		for (const SAbilityState& ability : Abilities)
		{
			GEngine::GetAssetRegistry()->UnrequestAsset(ability.ScriptReference, Owner.GUID);
		}
	}
}
