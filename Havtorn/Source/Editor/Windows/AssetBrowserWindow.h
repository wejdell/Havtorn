// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"
#include <filesystem>

namespace Havtorn
{
	class CFileSystem;
}

namespace ImGui
{
	struct SImportOptions
	{
		// TODO.NW: Maybe just use inheritance here, and send off SImportOptions to whatever tools needs them (for example ModelImporter)
		// Asset browser should have ownership. Should be fine to just new them here.
		void Render() {};
	};

	class CAssetBrowserWindow : public CWindow
	{
	public:
		CAssetBrowserWindow(const char* displayName, Havtorn::CEditorManager* manager);
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

		void InspectDirectoryEntry(const std::filesystem::directory_entry& entry, Havtorn::U32& outCurrentID, const ImTextureID& folderIconID);

		Havtorn::SVector2<Havtorn::F32> ThumbnailSize = { 64.0f, 64.0f };
		Havtorn::CFileSystem* FileSystem = nullptr;
		const std::string DefaultAssetPath = "Assets";
		std::filesystem::path CurrentDirectory = "";
		ImGuiTextFilter Filter = ImGuiTextFilter();
		bool IsCurrentlySelected = false;
		std::optional<std::vector<std::string>> FilePathsToImport;
		Havtorn::EAssetType CurrentImportAssetType = Havtorn::EAssetType::None;
	};
}
