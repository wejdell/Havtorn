// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/SourceAssetData.h"

namespace Havtorn 
{
	struct SInputAssetFileHeader
	{
		EAssetType AssetType = EAssetType::InputAsset;
		std::string Name = "";

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SInputAssetFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		return size;
	}

	inline void SInputAssetFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
	}

	inline void SInputAssetFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
	}
}
