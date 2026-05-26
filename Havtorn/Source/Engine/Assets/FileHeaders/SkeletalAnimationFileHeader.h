// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/AssetFileHeaderBase.h"
#include "Assets/SourceAssetData.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn 
{
	struct SSkeletalAnimationFileHeader
	{
		SAssetFileHeaderBase HeaderBase = { .AssetType = EAssetType::SkeletalAnimation, .Version = 1 };
		SSourceAssetData SourceData;
		std::string Name;
		U32 DurationInTicks = 0;
		U32 TickRate = 0;
		std::vector<SBoneAnimationTrack> BoneAnimationTracks;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSkeletalAnimationFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(HeaderBase);
		size += SourceData.GetSize();
		size += GetDataSize(Name);
		size += GetDataSize(DurationInTicks);
		size += GetDataSize(TickRate);

		size += GetDataSize(U32());
		for (auto& track : BoneAnimationTracks)
			size += track.GetSize();

		return size;
	}

	inline void SSkeletalAnimationFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(HeaderBase, toData, pointerPosition);
		SourceData.Serialize(toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(DurationInTicks, toData, pointerPosition);
		SerializeData(TickRate, toData, pointerPosition);

		SerializeData(STATIC_U32(BoneAnimationTracks.size()), toData, pointerPosition);
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
		DeserializeData(HeaderBase, fromData, pointerPosition);
		SourceData.Deserialize(fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(DurationInTicks, fromData, pointerPosition);
		DeserializeData(TickRate, fromData, pointerPosition);

		U32 numberOfBoneTracks = 0;
		DeserializeData(numberOfBoneTracks, fromData, pointerPosition);
		BoneAnimationTracks.reserve(numberOfBoneTracks);
		for (U16 i = 0; i < numberOfBoneTracks; i++)
		{
			BoneAnimationTracks.emplace_back();
			DeserializeData(BoneAnimationTracks.back().TranslationKeys, fromData, pointerPosition);
			DeserializeData(BoneAnimationTracks.back().RotationKeys, fromData, pointerPosition);
			DeserializeData(BoneAnimationTracks.back().ScaleKeys, fromData, pointerPosition);
			DeserializeData(BoneAnimationTracks.back().TrackName, fromData, pointerPosition);
		}
	}
}
