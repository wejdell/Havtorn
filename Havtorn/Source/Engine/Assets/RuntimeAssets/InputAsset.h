// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/FileHeaders/InputAssetFileHeader.h"
#include <Input/InputTypes.h>
#include <GameplayTags/GameplayTag.h>
#include <InputStructs.h>

#include <variant>

namespace Havtorn
{

	struct SInputAsset 
	{
		SInputAsset() = default;
		explicit SInputAsset(const SInputAssetFileHeader& assetFileData) 
		{
			InputActions = assetFileData.InputActions;
		}
		
		EAssetType AssetType = EAssetType::InputAsset;
		std::vector<SInputMapAction> InputActions;
	};
}
