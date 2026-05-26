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
			: AssetType(assetFileData.HeaderBase.AssetType)
		{
			Scene = GEngine::GetWorld()->CreateMovableScene(assetFileData.Name);
		}
	
		EAssetType AssetType = EAssetType::Prefab;
		Ptr<CScene> Scene = nullptr;
	};
}
