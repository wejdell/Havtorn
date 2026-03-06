// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/FileHeaders/PrefabAssetFileHeader.h"
#include "Engine.h"
#include "Scene/World.h"

namespace Havtorn
{
	struct SPrefabAsset
	{
		SPrefabAsset() = default;
		explicit SPrefabAsset(const SPrefabFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
		{
			Scene = GEngine::GetWorld()->CreateMovableGameScene("Prefab");
		}
	
		EAssetType AssetType = EAssetType::Prefab;
		Ptr<CScene> Scene = nullptr;
	};
}
