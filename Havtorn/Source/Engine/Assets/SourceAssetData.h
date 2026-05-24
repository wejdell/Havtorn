// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include <HavtornString.h>
#include "SourceAssetDataInclude.h"

namespace Havtorn 
{
	typedef std::variant<NullVariant, SStaticMeshSourceData, SSkeletalMeshSourceData, SSkeletalAnimationSourceData, STextureSourceData, STextureCubeSourceData, SAudioClipSourceData> SSourceAssetDataVariant;
	
	struct SSourceAssetData
	{
		EAssetType AssetType = EAssetType::None;
		U16 Version = 0;
		CHavtornStaticString<128> SourcePath;
		SSourceAssetDataVariant Variant = NullVariant();

		const bool IsValid() const { return SourcePath.Length() != 0; }
		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		void DeserializeDataVariant(SSourceAssetDataVariant& data, const EAssetType sourceDataAssetType, const char* fromData, U64& pointerPosition);
	};

	template<typename T>
	void DeserializeVariant(SSourceAssetDataVariant& data, const char* fromData, U64& pointerPosition)
	{
		T value;
		DeserializeData(value, fromData, pointerPosition);
		data = value;
	}
}
