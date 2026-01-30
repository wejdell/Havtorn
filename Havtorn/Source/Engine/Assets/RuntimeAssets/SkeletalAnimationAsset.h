// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/FileHeaders/SkeletalAnimationFileHeader.h"

namespace Havtorn
{
	struct SSkeletalAnimationAsset
	{
		SSkeletalAnimationAsset() = default;

		explicit SSkeletalAnimationAsset(const SSkeletalAnimationFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Name(assetFileData.Name)
			, RigPath(assetFileData.SourceData.AssetDependencyPath.AsString())
			, DurationInTicks(assetFileData.DurationInTicks)
			, TickRate(assetFileData.TickRate)
			, NumberOfTracks(assetFileData.NumberOfBones)
			, ImportScale(assetFileData.SourceData.ImportScale)
			, BoneAnimationTracks(assetFileData.BoneAnimationTracks)
		{
		}

		EAssetType AssetType = EAssetType::Animation;
		std::string Name = "";
		std::string RigPath = "";
		U32 DurationInTicks = 0;
		U32 TickRate = 0;
		U32 NumberOfTracks = 0;
		F32 ImportScale = 1.0f;
		std::vector<SBoneAnimationTrack> BoneAnimationTracks;
	};
}