// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"
#include <filesystem>

#include <GUI.h>
#include <EditorResourceManager.h>

#include <Input/InputTypes.h>

#include <queue>

namespace Havtorn
{
	class UFileSystem;

	class CAssetBrowserWindow : public CWindow
	{
	public:
		CAssetBrowserWindow(const char* displayName, CEditorManager* manager);
		~CAssetBrowserWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		void OnDragDropFiles(std::vector<std::string> filePaths);
		void OnAssetReloaded(const std::string& assetPath);
		
		void OnRenameEvent(const SInputActionPayload payload);

		void AssetImportModal();
		void AssetCreationModal();

		void ImportOptionsTexture();
		void ImportOptionsTextureCube();
		void ImportOptionsSpriteAnimation();
		void ImportOptionsStaticMesh();
		void ImportOptionsSkeletalMesh();
		void ImportOptionsAnimation();
		
		SAssetFileHeader CreateOptionsMaterial();
		SAssetFileHeader CreateScript();
		SAssetFileHeader CreateScene();

		void InspectFolderTree(const std::string& folderName, const intptr_t& folderIconID);
		void InspectDirectoryEntry(const std::filesystem::directory_entry& entry, U32& outCurrentID, const intptr_t& folderIconID);

		SAssetImportOptions ImportOptions;
		const std::string DefaultAssetPath = "Assets";
		std::filesystem::path CurrentDirectory = "";
		SGuiTextFilter Filter = SGuiTextFilter();
		std::optional<std::vector<std::string>> FilePathsToImport;

		std::optional<SEditorAssetRepresentation*> SelectedAsset;
		std::optional<SEditorAssetRepresentation*> AnimatingThumbnailAsset;
		SEditorAssetRepresentation* PreviouslyAnimatingThumbnailAsset;
		bool IsSelectionHovered = false;
		bool WasAnimatingThumbnail = false;
		bool IsCreatingAsset = false;
		EAssetType AssetTypeToCreate = EAssetType::None;
		std::string DirectoryToSaveTo = DefaultAssetPath;
		std::string NewAssetName = "NewAsset";
		SAssetFileHeader NewAssetFileHeader = std::monostate();

		// TODO.NW: Rather not store this like this, see if there's a better way
		std::array<SEditorAssetRepresentation*, 3> NewMaterialTextures = { nullptr, nullptr, nullptr };

		F32 LastAnimatedThumbnailTime = 0.0f;
		F32 AnimatedThumbnailTime = 0.0f;
	};
}
