// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/FileHeaders/TextureFileHeader.h"
#include "Graphics/RenderingPrimitives/RenderTexture.h"

namespace Havtorn
{
	struct STextureAsset
	{
		STextureAsset() = default;
		explicit STextureAsset(const STextureFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Name(assetFileData.Name)
		{
			// NW: RenderTarget is assigned in CAssetRegistry::LoadAsset
		}

		EAssetType AssetType = EAssetType::Texture;
		std::string Name = "";
		CStaticRenderTexture RenderTexture;
	};
}