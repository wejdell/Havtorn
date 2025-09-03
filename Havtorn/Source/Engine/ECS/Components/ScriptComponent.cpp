// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ScriptComponent.h"
#include "HexRune/HexRune.h"

namespace Havtorn
{
	void SScriptComponent::Serialize(char* toData, U64& pointerPosition) const
	{
		if (Script == nullptr)
		{
			std::string str = "NA";
			SerializeData(str, toData, pointerPosition);
		}
		else
		{
			SerializeData(Script->FileName, toData, pointerPosition);
		}

		SerializeData(Owner, toData, pointerPosition);
		SerializeData(AssetRegistryKey, toData, pointerPosition);
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
		DeserializeData(AssetRegistryKey, fromData, pointerPosition);

		DeserializeData(TriggeringSourceNode, fromData, pointerPosition);
		//GEngine::GetWorld()->LoadScript<SGameScript>(assetRep->DirectoryEntry.path().string());
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

		if (Script != nullptr)
			size += GetDataSize(Script->FileName);
		else
			size += sizeof(std::string("NA"));

		size += GetDataSize(Owner);
		size += GetDataSize(AssetRegistryKey);
		size += GetDataSize(TriggeringSourceNode);
		size += sizeof(U32);
		for (auto& databinding : DataBindings)	
			size += databinding.GetSize();	
		return size;
	}
}
