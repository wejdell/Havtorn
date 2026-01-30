// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	struct SMaterialAssetFileHeader
	{
		EAssetType AssetType = EAssetType::Material;
		std::string Name = "";
		U32 UID = 0;
		SOfflineGraphicsMaterial Material;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SMaterialAssetFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += GetDataSize(UID);
		size += Material.GetSize();

		return size;
	}

	inline void SMaterialAssetFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);

		for (auto& materialProperty : Material.Properties)
		{
			SerializeData(materialProperty.ConstantValue, toData, pointerPosition);
			SerializeData(materialProperty.TexturePath, toData, pointerPosition);
			SerializeData(materialProperty.TextureChannelIndex, toData, pointerPosition);
		}

		SerializeData(Material.RecreateZ, toData, pointerPosition);
	}

	inline void SMaterialAssetFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);

		for (auto& materialProperty : Material.Properties)
		{
			DeserializeData(materialProperty.ConstantValue, fromData, pointerPosition);
			DeserializeData(materialProperty.TexturePath, fromData, pointerPosition);
			DeserializeData(materialProperty.TextureChannelIndex, fromData, pointerPosition);
		}

		DeserializeData(Material.RecreateZ, fromData, pointerPosition);
	}
}