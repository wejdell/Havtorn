// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "Assets/SourceAssetData.h"

namespace Havtorn
{
	struct SAudioClipSettings
	{
		bool IsSpatialized = true;
		bool IsLooping = false;
	};

	struct SAudioClipFileHeader
	{
		EAssetType AssetType = EAssetType::AudioClip;
		U16 Version = 1;
		std::string Name = "";
		SSourceAssetData SourceData;
		SAudioClipSettings Settings;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SAudioClipFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Version);
		size += GetDataSize(Name);
		size += GetDataSize(SourceData);
		size += GetDataSize(Settings);

		return size;
	}

	inline void SAudioClipFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Version, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(SourceData, toData, pointerPosition);
		SerializeData(Settings, toData, pointerPosition);
	}

	inline void SAudioClipFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Version, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(SourceData, fromData, pointerPosition);
		DeserializeData(Settings, fromData, pointerPosition);
	}
}
