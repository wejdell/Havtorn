// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/AssetFileHeaderBase.h"
#include "Assets/SourceAssetData.h"

namespace Havtorn
{
	struct SAudioClipFileHeader
	{
		SAssetFileHeaderBase HeaderBase = { .AssetType = EAssetType::AudioClip, .Version = 1 };
		SSourceAssetData SourceData;
		std::string Name = "";

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SAudioClipFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(HeaderBase);
		size += SourceData.GetSize();
		size += GetDataSize(Name);

		return size;
	}

	inline void SAudioClipFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(HeaderBase, toData, pointerPosition);
		SourceData.Serialize(toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
	}

	inline void SAudioClipFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(HeaderBase, fromData, pointerPosition);
		SourceData.Deserialize(fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
	}
}
