// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once 
#include "Assets/FileHeaders/TextureCubeFileHeader.h"
#include "Graphics/RenderingPrimitives/RenderTexture.h"

namespace Havtorn
{
	struct STextureCubeAsset
	{
		STextureCubeAsset() = default;
		explicit STextureCubeAsset(const STextureCubeFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Name(assetFileData.Name)
		{
			// NW: RenderTarget is assigned in CAssetRegistry::LoadAsset
		}

		EAssetType AssetType = EAssetType::TextureCube;
		std::string Name = "";
		CStaticRenderTexture RenderTexture;
	};
}