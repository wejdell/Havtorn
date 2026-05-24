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

		void BrowseTo(SEditorAssetRepresentation* assetRep);
		void SetCurrentPath(const std::filesystem::path& path, const bool pushCommand = true);

	private:
		void OnDragDropFiles(std::vector<std::string> filePaths);
		void OnAssetReloaded(const std::string& assetPath);
		
		void OnRenameEvent(const SInputActionPayload payload);

		void AssetCreationModal();

		SAssetFileHeader CreateOptionsMaterial();

		void AssetImportModal();

		void ImportOptionsStaticMesh(const std::string& sourceFilePath);
		void ImportOptionsSkeletalMesh(const std::string& sourceFilePath);
		void ImportOptionsSkeletalAnimation(const std::string& sourceFilePath);
		void ImportOptionsTexture(const std::string& sourceFilePath);
		void ImportOptionsTextureCube(const std::string& sourceFilePath);
		void ImportOptionsSpriteAnimation(const std::string& sourceFilePath);
		void ImportOptionsAudioClip(const std::string& sourceFilePath);
		
		void InspectFolderTree(const std::string& folderName, const intptr_t& folderIconID);
		void InspectDirectoryEntry(const std::filesystem::directory_entry& entry, U32& outCurrentID, const intptr_t& folderIconID);

		SAssetImportOptions ImportOptions;
		const std::string DefaultAssetPath = "Assets";
		std::filesystem::path CurrentDirectory = "";
		SGuiTextFilter Filter = SGuiTextFilter();
		std::optional<std::vector<std::string>> FilePathsToImport;

		std::optional<SEditorAssetRepresentation*> AnimatingThumbnailAsset;
		std::optional<SEditorAssetRepresentation*> HoveredAsset;
		std::optional<std::filesystem::directory_entry> HoveredFolder;
		std::optional<std::filesystem::directory_entry> FolderBeingRenamed;
		SEditorAssetRepresentation* PreviouslyAnimatingThumbnailAsset = nullptr;
		bool WasAnimatingThumbnail = false;
		bool IsCreatingAsset = false;
		EAssetType AssetTypeToCreate = EAssetType::None;
		std::string DirectoryToSaveTo = DefaultAssetPath;
		std::string NewAssetName = "NewAsset";
		SAssetFileHeader NewAssetFileHeader = NullVariant();

		// TODO.NW: Rather not store this like this, see if there's a better way
		std::array<SEditorAssetRepresentation*, 3> NewMaterialTextures = { nullptr, nullptr, nullptr };

		F32 LastAnimatedThumbnailTime = 0.0f;
		F32 AnimatedThumbnailTime = 0.0f;
	};
}
