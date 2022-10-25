// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <vector>
#include <string>	

#include "Havtorn.h"

struct ID3D11ShaderResourceView;

namespace Havtorn
{
	class CGraphicsFramework;
	class CRenderManager;

	enum class EEditorTexture
	{
		FolderIcon,
		FileIcon,
		PlayIcon,
		PauseIcon,
		StopIcon,
		Count
	};

	class CEditorResourceManager
	{
	public:
		CEditorResourceManager() = default;
		~CEditorResourceManager() = default;

		bool Init(CRenderManager* renderManager, const CGraphicsFramework* framework);
		ID3D11ShaderResourceView* GetEditorTexture(EEditorTexture texture) const;

		void* RenderAssetTexure(EAssetType assetType, const std::string& fileName);

		__declspec(dllexport) void CreateAsset(const std::string& destinationPath, EAssetType assetType) const;
		__declspec(dllexport) std::string ConvertToHVA(const std::string& filePath, const std::string& destination, const EAssetType assetType) const;

		// NR: Based on current material config: albedo, material and normal textures
		__declspec(dllexport) void CreateMaterial(const std::string& destinationPath);

	private:
		std::string GetFileName(EEditorTexture texture);
	
	private:
		std::vector<ID3D11ShaderResourceView*> Textures;
		CRenderManager* RenderManager = nullptr;
		std::string ResourceAssetPath = "Resources/";
	};
}