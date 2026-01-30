// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/FileHeaders/SpriteAnimationClipFileHeader.h"
#include "ECS/Components/SpriteAnimatorGraphComponent.h"

namespace Havtorn 
{
	struct SSpriteAninmationClipAsset
	{
		SSpriteAninmationClipAsset() = default;

		explicit SSpriteAninmationClipAsset(const SSpriteAnimationClipFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, SpriteAnimationClip(SSpriteAnimationClip(assetFileData.UVRects, assetFileData.Durations, assetFileData.IsLooping))
		{
		}

		EAssetType AssetType = EAssetType::SpriteAnimation;
		SSpriteAnimationClip SpriteAnimationClip;
	};
}