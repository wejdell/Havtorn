// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "AssetBrowserWindow.h"

#include "EditorManager.h"
#include "DockSpaceWindow.h"
#include "EditorResourceManager.h"

#include <Engine.h>
#include <MathTypes/EngineMath.h>
#include <FileSystem/FileSystem.h>
#include <Graphics/RenderManager.h>
#include <PlatformManager.h>

namespace Havtorn
{
	CAssetBrowserWindow::CAssetBrowserWindow(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
		, FileSystem(GEngine::GetFileSystem())
	{
		CurrentDirectory = std::filesystem::path(DefaultAssetPath);		
		manager->GetPlatformManager()->OnDragDropAccepted.AddMember(this, &CAssetBrowserWindow::OnDragDropFiles);
	}

	CAssetBrowserWindow::~CAssetBrowserWindow()
	{		
	}

	void CAssetBrowserWindow::OnEnable()
	{
	}

	void CAssetBrowserWindow::OnInspectorGUI()
	{
		if (GUI::Begin(Name(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (GUI::ArrowButton("GoBackDir", SGuiDir_Up))
			{
				if (CurrentDirectory != std::filesystem::path(DefaultAssetPath))
					CurrentDirectory = CurrentDirectory.parent_path();
			}

			GUI::SameLine();
			Filter.Draw("Search", 180);

			GUI::Separator();

			// TODO.NR: Another magic number here, 10 cuts off the right border. 11 seems to work but feels too odd.
			F32 thumbnailPadding = 12.0f;
			F32 cellWidth = ThumbnailSize.X + thumbnailPadding;
			F32 panelWidth = GUI::GetContentRegionAvail().x;
			I32 columnCount = UMath::Max(static_cast<I32>(panelWidth / cellWidth), 1);

			SGuiTextureID folderIconID = (SGuiTextureID)(intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::FolderIcon);

			U32 id = 0;
			if (GUI::BeginTable("FileStructure", columnCount))
			{
				if (Filter.IsActive())
				{
					for (const auto& entry : std::filesystem::recursive_directory_iterator(CurrentDirectory))
					{
						if (!Filter.PassFilter(entry.path().string().c_str()))
							continue;

						InspectDirectoryEntry(entry, id, folderIconID);
					}
				}
				else
				{
					for (const auto& entry : std::filesystem::directory_iterator(CurrentDirectory))
						InspectDirectoryEntry(entry, id, folderIconID);
				}

				GUI::EndTable();
			}
		}

		if (FilePathsToImport.has_value() && !FilePathsToImport->empty())
		{
			GUI::OpenPopup("Asset Import");
			GUI::SetNextWindowPos(GUI::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, SVector2<F32>(0.5f, 0.5f));
			AssetImportModal();
		}

		GUI::End();
		
		// NR: Keep this here in case we want this to be a subwindow rather than an integrated element
		//if (GUI::Begin("Asset Browser Folder View"), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus)
		//{
		//	GUI::Text("Folder View");
		//}
		//GUI::End();
	}

	void CAssetBrowserWindow::OnDisable()
	{
	}

	/*
	Folder Structure Navigation
		Open Modal Popup where you can navigate choose where to save thingy or something

	Save new file dialoguehtrt

	<applause.h>
	*/
	void CAssetBrowserWindow::OnDragDropFiles(const std::vector<std::string> filePaths)
	{
		FilePathsToImport = filePaths;
	}

	void AlignForWidth(F32 width, F32 alignment = 0.5f)
	{
		F32 avail = GUI::GetContentRegionAvail().X;
		F32 off = (avail - width) * alignment;
		if (off > 0.0f)
			GUI::SetCursorPosX(GUI::GetCursorPosX() + off);
	}

	void CAssetBrowserWindow::AssetImportModal()
	{
		if (!GUI::BeginPopupModal("Asset Import", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		auto closePopup = [&]() 
		{
			CurrentImportAssetType = EAssetType::None;
			FilePathsToImport->erase(FilePathsToImport->begin());
			Manager->SetIsModalOpen(false);
			GUI::CloseCurrentPopup();
		};

		Manager->SetIsModalOpen(true);

		std::string filePath = *FilePathsToImport->begin();
		GUI::Text("Importing: %s", UGeneralUtils::ExtractFileNameFromPath(filePath).c_str());
		GUI::Separator();

		std::string fileExtension = UGeneralUtils::ExtractFileExtensionFromPath(filePath);
		
		if (fileExtension == "dds" || fileExtension == "tga" || fileExtension == "png")
		{
			if (CurrentImportAssetType == EAssetType::None)
				CurrentImportAssetType = EAssetType::Texture; 
			
			GUI::Text("Asset Type");
			GUI::SameLine();
			if (GUI::RadioButton("Texture", CurrentImportAssetType == EAssetType::Texture))
				CurrentImportAssetType = EAssetType::Texture;
			GUI::SameLine();
			if (GUI::RadioButton("Material", CurrentImportAssetType == EAssetType::Material))
				CurrentImportAssetType = EAssetType::Material;
			GUI::SameLine();
			if (GUI::RadioButton("Sprite Animation", CurrentImportAssetType == EAssetType::SpriteAnimation))
				CurrentImportAssetType = EAssetType::SpriteAnimation;
		}
		else if (fileExtension == "fbx" || fileExtension == "obj" || fileExtension == "stl")
		{
			if (CurrentImportAssetType == EAssetType::None)
				CurrentImportAssetType = EAssetType::StaticMesh;

			GUI::Text("Asset Type");
			GUI::SameLine();
			if (GUI::RadioButton("Static Mesh", CurrentImportAssetType == EAssetType::StaticMesh))
				CurrentImportAssetType = EAssetType::StaticMesh;
			GUI::SameLine();
			if (GUI::RadioButton("Skeletal Mesh", CurrentImportAssetType == EAssetType::SkeletalMesh))
				CurrentImportAssetType = EAssetType::SkeletalMesh;
			GUI::SameLine();
			if (GUI::RadioButton("Animation", CurrentImportAssetType == EAssetType::Animation))
				CurrentImportAssetType = EAssetType::Animation;
		}

		switch (CurrentImportAssetType)
		{
		case EAssetType::StaticMesh:
			ImportOptionsStaticMesh();
			break;
		case EAssetType::SkeletalMesh:
			ImportOptionsSkeletalMesh();
			break;
		case EAssetType::Texture:
			ImportOptionsTexture();
			break;
		case EAssetType::Material:
			ImportOptionsMaterial();
			break;
		case EAssetType::Animation:
			ImportOptionsAnimation();
			break;
		case EAssetType::SpriteAnimation:
			ImportOptionsSpriteAnimation();
			break;

		case EAssetType::AudioOneShot:
		case EAssetType::AudioCollection:
		case EAssetType::VisualFX:
		case EAssetType::Scene:
		case EAssetType::Sequencer:
		case EAssetType::None:
		default:
			GUI::EndPopup();
			return;
		}

		// Center buttons
		SGuiStyle& style = GUI::GetStyle();
		F32 width = 0.0f;
		width += GUI::CalcTextSize("Import").x + GUI::ThumbnailPadding;
		width += style.ItemSpacing.x;
		width += GUI::CalcTextSize("Cancel").x + GUI::ThumbnailPadding;
		AlignForWidth(width);

		if (GUI::Button("Import"))
		{
			std::string hvaFilePath = Manager->GetResourceManager()->ConvertToHVA(filePath, CurrentDirectory.string(), CurrentImportAssetType);
			Manager->CreateAssetRep(hvaFilePath);
			closePopup();
		}

		GUI::SameLine();

		if (GUI::Button("Cancel"))
		{
			closePopup();
		}

		GUI::EndPopup();
	}

	void CAssetBrowserWindow::ImportOptionsTexture()
	{
	}

	void CAssetBrowserWindow::ImportOptionsMaterial()
	{
	}

	void CAssetBrowserWindow::ImportOptionsSpriteAnimation()
	{
	}

	void CAssetBrowserWindow::ImportOptionsStaticMesh()
	{
	}

	void CAssetBrowserWindow::ImportOptionsSkeletalMesh()
	{
	}

	void CAssetBrowserWindow::ImportOptionsAnimation()
	{
		// TODO.NW: Have a "pick asset" field. show name of asset, and if clicked, open correct browser, let specify which type we want to open. Maybe fine with image button for now

	}

	void CAssetBrowserWindow::InspectDirectoryEntry(const std::filesystem::directory_entry& entry, U32& outCurrentID, const SGuiTextureID& folderIconID)
	{
		GUI::TableNextColumn();
		GUI::PushID(outCurrentID++);

		const auto& path = entry.path();
		auto relativePath = std::filesystem::relative(path);
		std::string filenameString = relativePath.filename().string();

		if (entry.is_directory())
		{
			if (GUI::ImageButton("FolderIcon", folderIconID, { ThumbnailSize.X, ThumbnailSize.Y }))
			{
				CurrentDirectory = entry.path();
			}

			GUI::Text(filenameString.c_str());
			if (GUI::IsItemHovered())
				GUI::SetTooltip(filenameString.c_str());
		}
		else
		{
			const auto& rep = Manager->GetAssetRepFromDirEntry(entry);
			if (!rep->TextureRef)
				rep->TextureRef = Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::FileIcon);

			if (GUI::ImageButton("AssetIcon", (SGuiTextureID)(intptr_t)rep->TextureRef, { ThumbnailSize.X, ThumbnailSize.Y }))
			{
				// NR: Open Tool depending on asset type
			}

			GUI::Text(rep->Name.c_str());
			if (GUI::IsItemHovered())
				GUI::SetTooltip(rep->Name.c_str());
		}

		GUI::PopID();
	}
}
