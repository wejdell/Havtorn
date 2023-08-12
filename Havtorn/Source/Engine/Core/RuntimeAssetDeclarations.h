// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "FileSystem/FileHeaderDeclarations.h"
#include "Graphics/GraphicsStructs.h"
#include "Graphics/GraphicsUtilities.h"
#include "Graphics/GraphicsMaterial.h"

namespace Havtorn
{
	struct SStaticMeshAsset
	{
		SStaticMeshAsset() = default;

		explicit SStaticMeshAsset(const SStaticModelFileHeader assetFileData)
			: AssetType(assetFileData.AssetType)
			, Name(assetFileData.Name)
			, NumberOfMaterials(assetFileData.NumberOfMaterials)
		{
			for (auto& mesh : assetFileData.Meshes)
			{
				DrawCallData.emplace_back();
				DrawCallData.back().IndexCount = mesh.NumberOfIndices;
			}
		}

		EAssetType AssetType = EAssetType::StaticMesh;
		std::string Name = "";
		U8 NumberOfMaterials = 0;
		std::vector<SDrawCallData> DrawCallData = {};
	};

	struct STextureAsset
	{
		STextureAsset() = default;

		explicit STextureAsset(const STextureFileHeader assetFileData, ID3D11Device* graphicsDevice)
			: AssetType(assetFileData.AssetType)
			, MaterialName(assetFileData.MaterialName)
			, MaterialConfiguration(assetFileData.MaterialConfiguration)
			, Suffix(assetFileData.Suffix)
		{
			ShaderResourceView = std::move(UGraphicsUtils::TryGetShaderResourceView(graphicsDevice, assetFileData.Data.data(), assetFileData.DataSize, assetFileData.OriginalFormat));
		}

		EAssetType AssetType = EAssetType::Texture;
		std::string MaterialName = "";
		EMaterialConfiguration MaterialConfiguration = EMaterialConfiguration::AlbedoMaterialNormal_Packed;
		char Suffix = 0;
		ID3D11ShaderResourceView* ShaderResourceView = nullptr;
	};

	struct SGraphicsMaterialAsset
	{
		SGraphicsMaterialAsset() = default;

		explicit SGraphicsMaterialAsset(const SMaterialAssetFileHeader assetFileData)
			: AssetType(assetFileData.AssetType)
			, Material(assetFileData.Material, assetFileData.MaterialName)
		{
		}

		EAssetType AssetType = EAssetType::Material;
		SEngineGraphicsMaterial Material;
	};

	struct SSpriteAninmationClipAsset
	{
		SSpriteAninmationClipAsset() = default;

		explicit SSpriteAninmationClipAsset(const SSpriteAnimationClipFileHeader assetFileData)
			: AssetType(assetFileData.AssetType)
			, SpriteAnimationClip(SSpriteAnimationClip(assetFileData.UVRects, assetFileData.Durations, assetFileData.IsLooping))
		{
		}

		EAssetType AssetType = EAssetType::SpriteAnimation;
		SSpriteAnimationClip SpriteAnimationClip;
	};
}