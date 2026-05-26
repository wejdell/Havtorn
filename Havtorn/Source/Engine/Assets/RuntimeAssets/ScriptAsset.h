// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/FileHeaders/ScriptFileHeader.h"
#include "Engine.h"
#include "Scene/World.h"

namespace Havtorn
{
	struct SScriptAsset
	{
		SScriptAsset() = default;
		explicit SScriptAsset(const SScriptFileHeader& assetFileData)
			: AssetType(assetFileData.HeaderBase.AssetType)
		{
			Script = GEngine::GetWorld()->CreateMovableScript(assetFileData.Name);
		}
	
		EAssetType AssetType = EAssetType::Script;
		Ptr<HexRune::SScript> Script = nullptr;
	};
}
