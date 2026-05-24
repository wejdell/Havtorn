// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/SourceAssetData.h"
#include "Graphics/GraphicsEnums.h"

namespace Havtorn 
{
	struct STextureCubeFileHeader
	{
		EAssetType AssetType = EAssetType::TextureCube;
		U16 Version = 1;
		SSourceAssetData SourceData;
		std::string Name = "";
		std::string Data = "";

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 STextureCubeFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Version);
		size += SourceData.GetSize();
		size += GetDataSize(Name);
		size += GetDataSize(Data);

		return size;
	}

	inline void STextureCubeFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Version, toData, pointerPosition);
		SourceData.Serialize(toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(Data, toData, pointerPosition);
	}

	inline void STextureCubeFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Version, fromData, pointerPosition);
		SourceData.Deserialize(fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(Data, fromData, pointerPosition);
	}
}
