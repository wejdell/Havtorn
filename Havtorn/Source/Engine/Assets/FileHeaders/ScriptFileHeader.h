// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/AssetFileHeaderBase.h"
#include "HexRune/HexRune.h"

namespace Havtorn
{
	struct SScriptFileHeader
	{
		SAssetFileHeaderBase HeaderBase = { .AssetType = EAssetType::Script, .Version = 1 };
		std::string Name = "";
		HexRune::SScript* Script = nullptr;
		mutable std::unordered_map<U64, SVector2<F32>> NodePositionMap;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData, HexRune::SScript* outScript);
	};

	inline U32 SScriptFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(HeaderBase);
		size += GetDataSize(Name);
		size += Script->GetSize();

		size += sizeof(U32);
		size += sizeof(U64) * STATIC_U32(NodePositionMap.size());
		
		size += sizeof(U32);
		size += sizeof(SVector2<F32>) * STATIC_U32(NodePositionMap.size());

		return size;
	}

	inline void SScriptFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(HeaderBase, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		Script->Serialize(toData, pointerPosition);
	
		std::vector<U64> nodePositionIDs;
		std::vector<SVector2<F32>> nodePositions;
		for (auto& [key, value] : NodePositionMap)
		{
			nodePositionIDs.push_back(key);
			nodePositions.push_back(value);
		}

		SerializeData(nodePositionIDs, toData, pointerPosition);
		SerializeData(nodePositions, toData, pointerPosition);
	}

	inline void SScriptFileHeader::Deserialize(const char* fromData, HexRune::SScript* outScript)
	{
		U64 pointerPosition = 0;
		DeserializeData(HeaderBase, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		outScript->Deserialize(fromData, pointerPosition);
		outScript->Name = Name;

		std::vector<U64> nodePositionIDs;
		std::vector<SVector2<F32>> nodePositions;

		DeserializeData(nodePositionIDs, fromData, pointerPosition);
		DeserializeData(nodePositions, fromData, pointerPosition);

		for (U64 i = 0; i < nodePositionIDs.size(); i++)
		{
			NodePositionMap.emplace(nodePositionIDs[i], nodePositions[i]);
		}
	}
}
