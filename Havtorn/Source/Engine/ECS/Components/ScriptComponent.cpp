// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ScriptComponent.h"
#include "HexRune/HexRune.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
	void SScriptComponent::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(Owner, toData, pointerPosition);
		AssetReference.Serialize(toData, pointerPosition);
		SerializeData(TriggeringSourceNode, toData, pointerPosition);

		SerializeData(static_cast<U32>(DataBindings.size()), toData, pointerPosition);
		for (auto& db : DataBindings)
		{
			db.Serialize(toData, pointerPosition);
		}
	}

	void SScriptComponent::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(Owner, fromData, pointerPosition);
		AssetReference.Deserialize(fromData, pointerPosition);
		DeserializeData(TriggeringSourceNode, fromData, pointerPosition);

		U32 databindingCount = 0;
		DeserializeData(databindingCount, fromData, pointerPosition);
		for (U32 i = 0; i < databindingCount; i++)
		{
			HexRune::SScriptDataBinding databinding = {};
			databinding.Deserialize(fromData, pointerPosition);
			DataBindings.emplace_back(databinding);
		}
	}

	U32 SScriptComponent::GetSize() const
	{
		U32 size = 0;

		size += GetDataSize(Owner);
		size += AssetReference.GetSize();
		size += GetDataSize(TriggeringSourceNode);
		
		size += sizeof(U32);
		for (auto& databinding : DataBindings)	
			size += databinding.GetSize();	
		
		return size;
	}
		
	void SScriptComponent::IsDeleted(CScene* /*fromScene*/)
	{
		GEngine::GetAssetRegistry()->UnrequestAsset(AssetReference, Owner.GUID);
	}
}
