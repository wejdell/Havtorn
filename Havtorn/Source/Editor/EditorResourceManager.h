// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <vector>
#include <string>	

#include <Havtorn.h>

struct ID3D11ShaderResourceView;

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
		Count
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
		ID3D11ShaderResourceView* GetEditorTexture(EEditorTexture texture) const;

		void* RenderAssetTexure(EAssetType assetType, const std::string& fileName) const;
		void* RenderAnimatedAssetTexture(const EAssetType assetType, const std::string& fileName, const F32 animationTime) const;

		EDITOR_API void CreateAsset(const std::string& destinationPath, EAssetType assetType) const;
		EDITOR_API std::string ConvertToHVA(const std::string& filePath, const std::string& destination, const SAssetImportOptions& importOptions) const;

		EDITOR_API void CreateMaterial(const std::string& destinationPath, const SMaterialAssetFileHeader& fileHeader);
		
		// NW: Based on current material config: albedo, material and normal textures
		EDITOR_API void CreateMaterial(const std::string& destinationPath);

	private:
		std::string GetFileName(EEditorTexture texture);
	
	private:
		std::vector<ID3D11ShaderResourceView*> Textures;
		CRenderManager* RenderManager = nullptr;
		std::string ResourceAssetPath = "Resources/";
	};
}