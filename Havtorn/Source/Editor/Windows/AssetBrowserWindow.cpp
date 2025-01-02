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
		std::string filePath = filePaths[0]; //temporarily the first filepath out of potentially many 0 to test
		
		//Copies file from original path and creates a HVA File in Assets/ (depending on where the AssetBrowser window is currently at)
		std::string hvaFilePath = Manager->GetResourceManager()->ConvertToHVA(filePath, CurrentDirectory.string(), Havtorn::EAssetType::Texture); 

		Manager->CreateAssetRep(hvaFilePath); //Creates a SEditorAssetRepresentation and adds it to the AssetRepresentations
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
