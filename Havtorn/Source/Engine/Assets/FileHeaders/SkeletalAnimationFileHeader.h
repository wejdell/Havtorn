// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/SourceAssetData.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn 
{
	struct SSkeletalAnimationFileHeader
	{
		EAssetType AssetType = EAssetType::Animation;
		std::string Name;
		U32 UID = 0;
		SSourceAssetData SourceData;
		U32 DurationInTicks = 0;
		U32 TickRate = 0;
		U32 NumberOfBones = 0;
		std::vector<SBoneAnimationTrack> BoneAnimationTracks;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSkeletalAnimationFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += GetDataSize(UID);
		size += GetDataSize(SourceData);
		size += GetDataSize(DurationInTicks);
		size += GetDataSize(TickRate);
		size += GetDataSize(NumberOfBones);

		for (auto& track : BoneAnimationTracks)
			size += track.GetSize();

		return size;
	}

	inline void SSkeletalAnimationFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);
		SerializeData(SourceData, toData, pointerPosition);
		SerializeData(DurationInTicks, toData, pointerPosition);
		SerializeData(TickRate, toData, pointerPosition);
		SerializeData(NumberOfBones, toData, pointerPosition);

		for (auto& track : BoneAnimationTracks)
		{
			SerializeData(track.TranslationKeys, toData, pointerPosition);
			SerializeData(track.RotationKeys, toData, pointerPosition);
			SerializeData(track.ScaleKeys, toData, pointerPosition);
			SerializeData(track.TrackName, toData, pointerPosition);
		}
	}

	inline void SSkeletalAnimationFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);
		DeserializeData(SourceData, fromData, pointerPosition);
		DeserializeData(DurationInTicks, fromData, pointerPosition);
		DeserializeData(TickRate, fromData, pointerPosition);
		DeserializeData(NumberOfBones, fromData, pointerPosition);

		for (U16 i = 0; i < NumberOfBones; i++)
		{
			BoneAnimationTracks.emplace_back();
			DeserializeData(BoneAnimationTracks.back().TranslationKeys, fromData, pointerPosition);
			DeserializeData(BoneAnimationTracks.back().RotationKeys, fromData, pointerPosition);
			DeserializeData(BoneAnimationTracks.back().ScaleKeys, fromData, pointerPosition);
			DeserializeData(BoneAnimationTracks.back().TrackName, fromData, pointerPosition);
		}
	}
}