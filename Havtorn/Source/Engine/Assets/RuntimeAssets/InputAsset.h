// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/FileHeaders/InputAssetFileHeader.h"

namespace Havtorn
{
	struct SInputAsset 
	{
		SInputAsset() = default;

		EAssetType AssetType = EAssetType::InputAsset;
		
		explicit SInputAsset(const SInputAssetFileHeader& assetFileData) 
		{
			assetFileData;
		}
	};
}
