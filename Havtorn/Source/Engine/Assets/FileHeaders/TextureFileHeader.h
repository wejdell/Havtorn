// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/SourceAssetData.h"
#include "Graphics/GraphicsEnums.h"

namespace Havtorn
{
	struct STextureFileHeader
	{
		EAssetType AssetType = EAssetType::Texture;
		std::string Name = "";
		// TODO.NW: remove this, should not be needed anymore now that the asset registry handles import
		U32 UID = 0;
		SSourceAssetData SourceData;
		ETextureFormat OriginalFormat = ETextureFormat::DDS;
		// TODO.NW: remove this, should not be needed anymore now that the asset registry handles import
		char Suffix = 0;
		std::string Data = "";

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 STextureFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += GetDataSize(UID);
		size += GetDataSize(SourceData);
		size += GetDataSize(OriginalFormat);
		size += GetDataSize(Suffix);
		size += GetDataSize(Data);

		return size;
	}

	inline void STextureFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);
		SerializeData(SourceData, toData, pointerPosition);
		SerializeData(OriginalFormat, toData, pointerPosition);
		SerializeData(Suffix, toData, pointerPosition);
		SerializeData(Data, toData, pointerPosition);
	}

	inline void STextureFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);
		DeserializeData(SourceData, fromData, pointerPosition);
		DeserializeData(OriginalFormat, fromData, pointerPosition);
		DeserializeData(Suffix, fromData, pointerPosition);
		DeserializeData(Data, fromData, pointerPosition);
	}
}