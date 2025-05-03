// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"
#include <filesystem>

#include <GUI.h>
#include <EditorResourceManager.h>

namespace Havtorn
{
	class CFileSystem;

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
		void AssetImportModal();

		void ImportOptionsTexture();
		void ImportOptionsMaterial();
		void ImportOptionsSpriteAnimation();
		void ImportOptionsStaticMesh();
		void ImportOptionsSkeletalMesh();
		void ImportOptionsAnimation();

		void InspectFolderTree(const std::string& folderName, const intptr_t& folderIconID);
		void InspectDirectoryEntry(const std::filesystem::directory_entry& entry, U32& outCurrentID, const intptr_t& folderIconID);

		SAssetImportOptions ImportOptions;
		CFileSystem* FileSystem = nullptr;
		const std::string DefaultAssetPath = "Assets";
		std::filesystem::path CurrentDirectory = "";
		SGuiTextFilter Filter = SGuiTextFilter();
		bool IsCurrentlySelected = false;
		std::optional<std::vector<std::string>> FilePathsToImport;

		std::optional<std::filesystem::directory_entry> SelectedAsset;

		F32 LastAnimatedThumbnailTime = 0.0f;
		F32 AnimatedThumbnailTime = 0.0f;
	};
}
