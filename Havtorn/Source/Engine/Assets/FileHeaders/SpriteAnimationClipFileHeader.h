// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/SourceAssetData.h"

namespace Havtorn
{
	struct SSpriteAnimationClipFileHeader
	{
		EAssetType AssetType = EAssetType::SpriteAnimation;
		std::string Name;
		U32 UID = 0;
		SSourceAssetData SourceData;
		std::vector<SVector4> UVRects;
		std::vector<F32> Durations;
		bool IsLooping = false;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSpriteAnimationClipFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += GetDataSize(UID);
		size += GetDataSize(UVRects);
		size += GetDataSize(Durations);
		size += GetDataSize(IsLooping);
		return size;
	}

	inline void SSpriteAnimationClipFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);
		SerializeData(UVRects, toData, pointerPosition);
		SerializeData(Durations, toData, pointerPosition);
		SerializeData(IsLooping, toData, pointerPosition);
	}

	inline void SSpriteAnimationClipFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);
		DeserializeData(UVRects, fromData, pointerPosition);
		DeserializeData(Durations, fromData, pointerPosition);
		DeserializeData(IsLooping, fromData, pointerPosition);
	}
}