// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/FileHeaders/SkeletalAnimationFileHeader.h"

namespace Havtorn
{
	struct SSkeletalAnimationAsset
	{
		SSkeletalAnimationAsset() = default;

		explicit SSkeletalAnimationAsset(const SSkeletalAnimationFileHeader& assetFileData)
			: AssetType(assetFileData.HeaderBase.AssetType)
			, Name(assetFileData.Name)
			, DurationInTicks(assetFileData.DurationInTicks)
			, TickRate(assetFileData.TickRate)
			, BoneAnimationTracks(assetFileData.BoneAnimationTracks)
		{
			SSkeletalAnimationSourceData sourceData = std::get<SSkeletalAnimationSourceData>(assetFileData.SourceData.Variant);
			RigPath = sourceData.RigMeshPath.AsString();
			ImportScale = sourceData.ImportScale;
		}

		EAssetType AssetType = EAssetType::SkeletalAnimation;
		std::string Name = "";
		std::string RigPath = "";
		U32 DurationInTicks = 0;
		U32 TickRate = 0;
		F32 ImportScale = 1.0f;
		std::vector<SBoneAnimationTrack> BoneAnimationTracks;
	};
}
