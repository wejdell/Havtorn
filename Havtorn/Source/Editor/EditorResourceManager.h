// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <vector>
#include <string>	

#include <Havtorn.h>
#include <Graphics/RenderingPrimitives/RenderTexture.h>

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
		MinimizeWindow,
		MaximizeWindow,
		CloseWindow,
		CameraIcon,
		GetFromSource,
		PrefabIcon,
		PrefabWidgetIcon,
		FindIcon,
		MoveGizmoIcon,
		RotateGizmoIcon,
		ScaleGizmoIcon,
		InputMapIcon,
		AudioClipIcon,
		Count,
		None = Count
	};

	struct SAssetImportOptions
	{
		SEditorAssetRepresentation* AssetRep = nullptr;
		SSourceAssetData SourceData;
	};

	class CEditorResourceManager
	{
	public:
		CEditorResourceManager() = default;
		~CEditorResourceManager() = default;

		bool Init(CEditorManager* editorManager, CRenderManager* renderManager);
		intptr_t GetStaticEditorTextureResource(const EEditorTexture texture) const;

		EDITOR_API void RequestThumbnailRender(SEditorAssetRepresentation* assetRep, const std::string& filePath) const;
		EDITOR_API void AnimateAssetTexture(SEditorAssetRepresentation* assetRep, const std::string& filePath, const F32 animationTime) const;

		EDITOR_API std::string CreateAsset(const std::string& destinationPath, const SAssetFileHeader& fileHeader) const;
		EDITOR_API std::string ConvertToHVA(const std::string& filePath, const std::string& destinationPath, const SSourceAssetData& sourceAssetData) const;

		EDITOR_API void CreateMaterial(const std::string& destinationPath, const SMaterialAssetFileHeader& fileHeader) const;
		
		// NW: Based on current material config: albedo, material and normal textures
		EDITOR_API void CreateMaterial(const std::string& destinationPath, const std::array<std::string, 3>& texturePaths) const;

	private:
		std::string GetFileName(EEditorTexture texture, const std::string& extension, const std::string& prefix = "");
	
	private:
		std::vector<CStaticRenderTexture> Textures;
		CEditorManager* Manager = nullptr;
		CRenderManager* RenderManager = nullptr;
		std::string ResourceAssetPath = "Resources/";
	};
}