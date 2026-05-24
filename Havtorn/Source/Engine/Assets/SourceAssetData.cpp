// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "SourceAssetData.h"

namespace Havtorn 
{
	U32 SSourceAssetData::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Version);
		size += GetDataSize(SourcePath);
		size += GetDataSize(Variant);

		return size;
	}

	void SSourceAssetData::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Version, toData, pointerPosition);
		SerializeData(SourcePath, toData, pointerPosition);
		SerializeData(Variant, toData, pointerPosition);
	}

	void SSourceAssetData::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Version, fromData, pointerPosition);
		DeserializeData(SourcePath, fromData, pointerPosition);

		// NW: Introduce versioning here if needed, appending newer versions to the variant
		// and translating between them. Input to versioning service is AssetType and Version
		DeserializeDataVariant(Variant, AssetType, fromData, pointerPosition);
	}

	void SSourceAssetData::DeserializeDataVariant(SSourceAssetDataVariant& data, const EAssetType sourceDataAssetType, const char* fromData, U64& pointerPosition)
	{
		switch (sourceDataAssetType)
		{
		case EAssetType::StaticMesh:		DeserializeVariant<SStaticMeshSourceData>(data, fromData, pointerPosition);			break;
		case EAssetType::SkeletalMesh:		DeserializeVariant<SSkeletalMeshSourceData>(data, fromData, pointerPosition);		break;
		case EAssetType::SkeletalAnimation:	DeserializeVariant<SSkeletalAnimationSourceData>(data, fromData, pointerPosition);	break;
		case EAssetType::Texture:			DeserializeVariant<STextureSourceData>(data, fromData, pointerPosition);			break;
		case EAssetType::TextureCube:		DeserializeVariant<STextureCubeSourceData>(data, fromData, pointerPosition);		break;
		case EAssetType::AudioClip:			DeserializeVariant<SAudioClipSourceData>(data, fromData, pointerPosition);			break;
		}
	}
}
