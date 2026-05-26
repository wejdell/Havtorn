// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/AssetFileHeaderBase.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	struct SMaterialAssetFileHeader
	{
		SAssetFileHeaderBase HeaderBase = { .AssetType = EAssetType::Material, .Version = 1 };
		std::string Name = "";
		SOfflineGraphicsMaterial Material;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SMaterialAssetFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(HeaderBase);
		size += GetDataSize(Name);
		size += Material.GetSize();

		return size;
	}

	inline void SMaterialAssetFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(HeaderBase, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);

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
		DeserializeData(HeaderBase, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);

		for (auto& materialProperty : Material.Properties)
		{
			DeserializeData(materialProperty.ConstantValue, fromData, pointerPosition);
			DeserializeData(materialProperty.TexturePath, fromData, pointerPosition);
			DeserializeData(materialProperty.TextureChannelIndex, fromData, pointerPosition);
		}

		DeserializeData(Material.RecreateZ, fromData, pointerPosition);
	}
}
