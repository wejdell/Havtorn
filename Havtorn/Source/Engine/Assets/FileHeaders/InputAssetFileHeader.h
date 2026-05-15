// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <InputStructs.h>

namespace Havtorn
{
	struct SInputAssetFileHeader
	{
		EAssetType AssetType = EAssetType::InputAsset;
		std::string Name = "";

		std::vector<SInputMapAction> InputActions;
		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SInputAssetFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += sizeof(U32);
		for (auto& action : InputActions)
		{
			size += GetDataSize(action.Tag);
			size += sizeof(U32);

			for (auto& mapping : action.InputMappings)
				size += GetDataSize(mapping.Data);
		}
		return size;
	}

	inline void SInputAssetFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);

		SerializeData(STATIC_U32(InputActions.size()), toData, pointerPosition);
		for (auto& action : InputActions)
		{
			action.Tag.Serialize(toData, pointerPosition);
			SerializeData(STATIC_U32(action.InputMappings.size()), toData, pointerPosition);

			for (auto& mapping : action.InputMappings)
			{
				SerializeData(STATIC_U32(mapping.Data.index()), toData, pointerPosition);
				SerializeData(mapping.Data, toData, pointerPosition);
			}
		}
	}

	inline void SInputAssetFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);

		U32 inputMapCount = 0;
		DeserializeData(inputMapCount, fromData, pointerPosition);
		InputActions.resize(inputMapCount);

		for (auto& action : InputActions)
		{
			action.Tag.Deserialize(fromData, pointerPosition);

			U32 inputMappingCount = 0;
			DeserializeData(inputMappingCount, fromData, pointerPosition);
			action.InputMappings.resize(inputMappingCount);
			for (auto& mapping : action.InputMappings)
			{
				U32 variantIndex = 0;
				DeserializeData(variantIndex, fromData, pointerPosition);

				switch (variantIndex)
				{
				case STATIC_U32(0):
				{
					SAxis data = SAxis{};
					DeserializeData(data, fromData, pointerPosition);
					mapping.Data = data;
				}
				break;
				case STATIC_U32(1):
				{
					SKey data = SKey{};
					DeserializeData(data, fromData, pointerPosition);
					mapping.Data = data;
				}
				break;
				}
			}
		}
	}
}
