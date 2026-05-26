// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/AssetFileHeaderBase.h"
#include "Assets/SequencerAsset.h"

namespace Havtorn
{
	struct SSequencerFileHeader
	{
		SAssetFileHeaderBase HeaderBase = { .AssetType = EAssetType::Sequencer, .Version = 1 };
		std::string SequencerName = "";
		U32 NumberOfEntityReferences = 0;
		std::vector<SSequencerEntityReference> EntityReferences;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSequencerFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(HeaderBase);
		size += GetDataSize(SequencerName);
		size += GetDataSize(NumberOfEntityReferences);

		for (const SSequencerEntityReference& reference : EntityReferences)
		{
			size += reference.GetSize();
		}

		return size;
	}

	inline void SSequencerFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(HeaderBase, toData, pointerPosition);
		SerializeData(SequencerName, toData, pointerPosition);
		SerializeData(NumberOfEntityReferences, toData, pointerPosition);

		for (const SSequencerEntityReference& reference : EntityReferences)
		{
			reference.Serialize(toData, pointerPosition);
		}
	}

	inline void SSequencerFileHeader::Deserialize(const char* fromData)
	{

		U64 pointerPosition = 0;
		DeserializeData(HeaderBase, fromData, pointerPosition);
		DeserializeData(SequencerName, fromData, pointerPosition);
		DeserializeData(NumberOfEntityReferences, fromData, pointerPosition);

		for (U64 index = 0; index < NumberOfEntityReferences; index++)
		{
			EntityReferences.emplace_back();
			EntityReferences.back().Deserialize(fromData, pointerPosition);
		}
	}
}
