// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "AssetBrowserWindow.h"

#include "EditorManager.h"
#include "DockSpaceWindow.h"
#include "EditorResourceManager.h"

#include <Engine.h>
#include <Core/MathTypes/EngineMath.h>
#include <FileSystem/FileSystem.h>
#include <Graphics/RenderManager.h>

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace ImGui
{
	using Havtorn::F32;

	CAssetBrowserWindow::CAssetBrowserWindow(const char* displayName, Havtorn::CEditorManager* manager)
		: CWindow(displayName, manager)
		, FileSystem(Havtorn::GEngine::GetFileSystem())
	{
		CurrentDirectory = std::filesystem::path(DefaultAssetPath);		
		Havtorn::GEngine::GetWindowHandler()->OnDragDropAccepted.AddMember(this, &CAssetBrowserWindow::OnDragDropFiles);
	}

	CAssetBrowserWindow::~CAssetBrowserWindow()
	{		
	}

	void CAssetBrowserWindow::OnEnable()
	{
	}

	void CAssetBrowserWindow::OnInspectorGUI()
	{
		if (ImGui::Begin(Name(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (ImGui::ArrowButton("GoBackDir", ImGuiDir_Up))
			{
				if (CurrentDirectory != std::filesystem::path(DefaultAssetPath))
					CurrentDirectory = CurrentDirectory.parent_path();
			}

			ImGui::SameLine();
			Filter.Draw("Search", 180);

			ImGui::Separator();

			// TODO.NR: Another magic number here, 10 cuts off the right border. 11 seems to work but feels too odd.
			F32 thumbnailPadding = 12.0f;
			F32 cellWidth = ThumbnailSize.X + thumbnailPadding;
			F32 panelWidth = ImGui::GetContentRegionAvail().x;
			Havtorn::I32 columnCount = Havtorn::UMath::Max(static_cast<Havtorn::I32>(panelWidth / cellWidth), 1);

			ImTextureID folderIconID = (ImTextureID)(intptr_t)Manager->GetResourceManager()->GetEditorTexture(Havtorn::EEditorTexture::FolderIcon);

			Havtorn::U32 id = 0;
			if (ImGui::BeginTable("FileStructure", columnCount))
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

				ImGui::EndTable();
			}
		}

		if (FilePathsToImport.has_value() && !FilePathsToImport->empty())
		{
			ImGui::OpenPopup("Asset Import");
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			AssetImportModal();
		}

		ImGui::End();
		
		// NR: Keep this here in case we want this to be a subwindow rather than an integrated element
		//if (ImGui::Begin("Asset Browser Folder View"), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus)
		//{
		//	ImGui::Text("Folder View");
		//}
		//ImGui::End();
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
		F32 avail = ImGui::GetContentRegionAvail().x;
		F32 off = (avail - width) * alignment;
		if (off > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
	}

	void CAssetBrowserWindow::AssetImportModal()
	{
		if (!ImGui::BeginPopupModal("Asset Import", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		auto closePopup = [&]() 
		{
			CurrentImportAssetType = Havtorn::EAssetType::None;
			FilePathsToImport->erase(FilePathsToImport->begin());
			Manager->SetIsModalOpen(false);
			ImGui::CloseCurrentPopup();
		};

		Manager->SetIsModalOpen(true);

		std::string filePath = *FilePathsToImport->begin();
		ImGui::Text("Importing: %s", Havtorn::UGeneralUtils::ExtractFileNameFromPath(filePath).c_str());
		ImGui::Separator();

		std::string fileExtension = Havtorn::UGeneralUtils::ExtractFileExtensionFromPath(filePath);
		
		if (fileExtension == "dds" || fileExtension == "tga" || fileExtension == "png")
		{
			if (CurrentImportAssetType == Havtorn::EAssetType::None)
				CurrentImportAssetType = Havtorn::EAssetType::Texture; 
			
			ImGui::Text("Asset Type");
			ImGui::SameLine();
			if (ImGui::RadioButton("Texture", CurrentImportAssetType == Havtorn::EAssetType::Texture))
				CurrentImportAssetType = Havtorn::EAssetType::Texture;
			ImGui::SameLine();
			if (ImGui::RadioButton("Material", CurrentImportAssetType == Havtorn::EAssetType::Material))
				CurrentImportAssetType = Havtorn::EAssetType::Material;
			ImGui::SameLine();
			if (ImGui::RadioButton("Sprite Animation", CurrentImportAssetType == Havtorn::EAssetType::SpriteAnimation))
				CurrentImportAssetType = Havtorn::EAssetType::SpriteAnimation;
		}
		else if (fileExtension == "fbx" || fileExtension == "obj" || fileExtension == "stl")
		{
			if (CurrentImportAssetType == Havtorn::EAssetType::None)
				CurrentImportAssetType = Havtorn::EAssetType::StaticMesh;

			ImGui::Text("Asset Type");
			ImGui::SameLine();
			if (ImGui::RadioButton("Static Mesh", CurrentImportAssetType == Havtorn::EAssetType::StaticMesh))
				CurrentImportAssetType = Havtorn::EAssetType::StaticMesh;
			ImGui::SameLine();
			if (ImGui::RadioButton("Skeletal Mesh", CurrentImportAssetType == Havtorn::EAssetType::SkeletalMesh))
				CurrentImportAssetType = Havtorn::EAssetType::SkeletalMesh;
			ImGui::SameLine();
			if (ImGui::RadioButton("Animation", CurrentImportAssetType == Havtorn::EAssetType::Animation))
				CurrentImportAssetType = Havtorn::EAssetType::Animation;
		}

		switch (CurrentImportAssetType)
		{
		case Havtorn::EAssetType::StaticMesh:
			ImportOptionsStaticMesh();
			break;
		case Havtorn::EAssetType::SkeletalMesh:
			ImportOptionsSkeletalMesh();
			break;
		case Havtorn::EAssetType::Texture:
			ImportOptionsTexture();
			break;
		case Havtorn::EAssetType::Material:
			ImportOptionsMaterial();
			break;
		case Havtorn::EAssetType::Animation:
			ImportOptionsAnimation();
			break;
		case Havtorn::EAssetType::SpriteAnimation:
			ImportOptionsSpriteAnimation();
			break;

		case Havtorn::EAssetType::AudioOneShot:
		case Havtorn::EAssetType::AudioCollection:
		case Havtorn::EAssetType::VisualFX:
		case Havtorn::EAssetType::Scene:
		case Havtorn::EAssetType::Sequencer:
		case Havtorn::EAssetType::None:
		default:
			ImGui::EndPopup();
			return;
		}

		// Center buttons
		ImGuiStyle& style = ImGui::GetStyle();
		F32 width = 0.0f;
		width += ImGui::CalcTextSize("Import").x + ImGui::UUtils::ThumbnailPadding;
		width += style.ItemSpacing.x;
		width += ImGui::CalcTextSize("Cancel").x + ImGui::UUtils::ThumbnailPadding;
		AlignForWidth(width);

		if (ImGui::Button("Import"))
		{
			std::string hvaFilePath = Manager->GetResourceManager()->ConvertToHVA(filePath, CurrentDirectory.string(), CurrentImportAssetType);
			Manager->CreateAssetRep(hvaFilePath);
			closePopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			closePopup();
		}

		ImGui::EndPopup();
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

	void CAssetBrowserWindow::InspectDirectoryEntry(const std::filesystem::directory_entry& entry, Havtorn::U32& outCurrentID, const ImTextureID& folderIconID)
	{
		ImGui::TableNextColumn();
		ImGui::PushID(outCurrentID++);

		const auto& path = entry.path();
		auto relativePath = std::filesystem::relative(path);
		std::string filenameString = relativePath.filename().string();

		if (entry.is_directory())
		{
			if (ImGui::ImageButton("FolderIcon", folderIconID, { ThumbnailSize.X, ThumbnailSize.Y }))
			{
				CurrentDirectory = entry.path();
			}

			ImGui::Text(filenameString.c_str());
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(filenameString.c_str());
		}
		else
		{
			const auto& rep = Manager->GetAssetRepFromDirEntry(entry);
			if (!rep->TextureRef)
				rep->TextureRef = Manager->GetResourceManager()->GetEditorTexture(Havtorn::EEditorTexture::FileIcon);

			if (ImGui::ImageButton("AssetIcon", (ImTextureID)(intptr_t)rep->TextureRef, { ThumbnailSize.X, ThumbnailSize.Y }))
			{
				// NR: Open Tool depending on asset type
			}

			ImGui::Text(rep->Name.c_str());
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(rep->Name.c_str());
		}

		ImGui::PopID();
	}
}
