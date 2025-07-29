// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <vector>
#include <string>	

#include <Havtorn.h>
#include <Graphics/RenderingPrimitives/FullscreenTexture.h>

namespace Havtorn
{
	class CGraphicsFramework;
	class CRenderManager;
	class CEditorManager;
	struct SEditorAssetRepresentation;

	enum class EEditorTexture
	{
		FolderIcon,
		FileIcon,
		PlayIcon,
		PauseIcon,
		StopIcon,
		SceneIcon,
		SequencerIcon,
		EnvironmentLightIcon,
		DirectionalLightIcon,
		PointLightIcon,
		SpotlightIcon,
		DecalIcon,
		ScriptIcon,
		ColliderIcon,
		NodeBackground,
		Count,
		None = Count
	};

	struct SAssetImportOptions
	{
		// TODO.NW: Maybe just use inheritance here, and send off SImportOptions to whatever tools needs them (for example ModelImporter)
		// Asset browser should have ownership. Should be fine to just new them here.
		//void Render() {};

		// TODO.NW: Add destination path here, settable through explorer navigation? Default to the current one in the asset browser

		EAssetType AssetType = EAssetType::None;
		SEditorAssetRepresentation* AssetRep = nullptr;
		F32 Scale = 1.0f;
	};

	class CEditorResourceManager
	{
	public:
		CEditorResourceManager() = default;
		~CEditorResourceManager() = default;

		bool Init(CRenderManager* renderManager, const CGraphicsFramework* framework);
		CRenderTexture GetEditorTexture(EEditorTexture texture) const;

		CRenderTexture RenderAssetTexure(EAssetType assetType, const std::string& fileName) const;
		void AnimateAssetTexture(CRenderTexture& assetTexture, const EAssetType assetType, const std::string& fileName, const F32 animationTime) const;

		EDITOR_API std::string CreateAsset(const std::string& destinationPath, const SAssetFileHeader& fileHeader) const;
		EDITOR_API std::string ConvertToHVA(const std::string& filePath, const std::string& destination, const SAssetImportOptions& importOptions) const;

		EDITOR_API void CreateMaterial(const std::string& destinationPath, const SMaterialAssetFileHeader& fileHeader) const;
		
		// NW: Based on current material config: albedo, material and normal textures
		EDITOR_API void CreateMaterial(const std::string& destinationPath, const std::array<std::string, 3>& texturePaths) const;

	private:
		std::string GetFileName(EEditorTexture texture);
	
	private:
		std::vector<CRenderTexture> Textures;
		CRenderManager* RenderManager = nullptr;
		std::string ResourceAssetPath = "Resources/";
	};
}