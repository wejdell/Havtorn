// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/FileHeaders/MaterialAssetFileHeader.h"
#include "Graphics/GraphicsMaterial.h"

namespace Havtorn
{
	struct SGraphicsMaterialAsset
	{
		SGraphicsMaterialAsset() = default;

		explicit SGraphicsMaterialAsset(const SMaterialAssetFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Material(assetFileData.Material, assetFileData.Name)
		{
		}

		EAssetType AssetType = EAssetType::Material;
		SEngineGraphicsMaterial Material;
	};
}