// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/SourceAssetData.h"
#include "Graphics/GraphicsEnums.h"

namespace Havtorn 
{
	struct STextureCubeFileHeader
	{
		EAssetType AssetType = EAssetType::TextureCube;
		std::string Name = "";
		SSourceAssetData SourceData;
		ETextureFormat OriginalFormat = ETextureFormat::DDS;
		std::string Data = "";

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 STextureCubeFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += GetDataSize(SourceData);
		size += GetDataSize(OriginalFormat);
		size += GetDataSize(Data);

		return size;
	}

	inline void STextureCubeFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(SourceData, toData, pointerPosition);
		SerializeData(OriginalFormat, toData, pointerPosition);
		SerializeData(Data, toData, pointerPosition);
	}

	inline void STextureCubeFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(SourceData, fromData, pointerPosition);
		DeserializeData(OriginalFormat, fromData, pointerPosition);
		DeserializeData(Data, fromData, pointerPosition);
	}
}