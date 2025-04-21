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
		if (GUI::Begin(Name(), nullptr, { EWindowFlag::NoMove, EWindowFlag::NoResize, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus }))
		{
			intptr_t folderIconID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::FolderIcon);

			{
				GUI::BeginChild("FolderTree", SVector2<F32>(150.0f, 0.0f), { EChildFlag::Borders, EChildFlag::ResizeX });
				GUI::Text("Project Name");
				GUI::Separator();

				if (GUI::BeginTable("ProjectName", 1))
				{
					InspectFolderTree(DefaultAssetPath, folderIconID);
					GUI::EndTable();
				}
				GUI::EndChild();
				GUI::SameLine();
			}

			GUI::BeginGroup();
			GUI::BeginChild("Browser");
			if (GUI::ArrowButton("GoBackDir", EGUIDirection::Left))
			{
				if (CurrentDirectory != std::filesystem::path(DefaultAssetPath))
					CurrentDirectory = CurrentDirectory.parent_path();
			}

			GUI::SameLine();
			Filter.Draw("Search", 180);

			GUI::Separator();

			// TODO.NR: Another magic number here, 10 cuts off the right border. 11 seems to work but feels too odd.
			F32 thumbnailPadding = 12.0f;
			F32 cellWidth = GUI::ThumbnailSizeX + thumbnailPadding;
			F32 panelWidth = GUI::GetContentRegionAvail().X;
			I32 columnCount = UMath::Max(static_cast<I32>(panelWidth / cellWidth), 1);

			U32 id = 0;
			if (GUI::BeginTable("FileStructure", columnCount))
			{
				if (Filter.IsActive())
				{
					for (const auto& entry : std::filesystem::recursive_directory_iterator(CurrentDirectory))
					{
						if (Filter.IsActive() && !Filter.PassFilter(entry.path().string().c_str()))
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
			GUI::EndChild();
			GUI::EndGroup();
		}

		if (FilePathsToImport.has_value() && !FilePathsToImport->empty())
		{
			GUI::OpenPopup("Asset Import");
			GUI::SetNextWindowPos(GUI::GetViewportCenter(), EWindowCondition::Appearing, SVector2<F32>(0.5f, 0.5f));
			AssetImportModal();
		}

		GUI::End();
		
		//// NR: Keep this here in case we want this to be a subwindow rather than an integrated element
		//if (GUI::Begin("Asset Browser Folder View", nullptr, { EWindowFlag::NoMove, EWindowFlag::NoResize, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus }))
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
		if (!GUI::BeginPopupModal("Asset Import", NULL, { EWindowFlag::AlwaysAutoResize }))
			return;

		auto closePopup = [&]() 
		{
			FilePathsToImport->erase(FilePathsToImport->begin());
			ImportOptions = SAssetImportOptions();
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
			if (ImportOptions.AssetType == EAssetType::None)
				ImportOptions.AssetType = EAssetType::Texture;

			GUI::Text("Asset Type");
			GUI::SameLine();
			if (GUI::RadioButton("Texture", ImportOptions.AssetType == EAssetType::Texture))
				ImportOptions.AssetType = EAssetType::Texture;
			GUI::SameLine();
			if (GUI::RadioButton("Material", ImportOptions.AssetType == EAssetType::Material))
				ImportOptions.AssetType = EAssetType::Material;
			GUI::SameLine();
			if (GUI::RadioButton("Sprite Animation", ImportOptions.AssetType == EAssetType::SpriteAnimation))
				ImportOptions.AssetType = EAssetType::SpriteAnimation;
		}
		else if (fileExtension == "fbx" || fileExtension == "obj" || fileExtension == "stl")
		{
			if (ImportOptions.AssetType == EAssetType::None)
				ImportOptions.AssetType = EAssetType::StaticMesh;

			GUI::Text("Asset Type");
			GUI::SameLine();
			if (GUI::RadioButton("Static Mesh", ImportOptions.AssetType == EAssetType::StaticMesh))
				ImportOptions.AssetType = EAssetType::StaticMesh;
			GUI::SameLine();
			if (GUI::RadioButton("Skeletal Mesh", ImportOptions.AssetType == EAssetType::SkeletalMesh))
				ImportOptions.AssetType = EAssetType::SkeletalMesh;
			GUI::SameLine();
			if (GUI::RadioButton("Animation", ImportOptions.AssetType == EAssetType::Animation))
				ImportOptions.AssetType = EAssetType::Animation;
		}

		GUI::Separator();

		switch (ImportOptions.AssetType)
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
		F32 width = 0.0f;
		width += GUI::CalculateTextSize("Import").X + GUI::ThumbnailPadding;
		width += GUI::GetStyleVar(EStyleVar::ItemSpacing).X;
		width += GUI::CalculateTextSize("Cancel").X + GUI::ThumbnailPadding;
		AlignForWidth(width);

		if (GUI::Button("Import"))
		{
			std::string hvaFilePath = Manager->GetResourceManager()->ConvertToHVA(filePath, CurrentDirectory.string() + "/", ImportOptions);
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
		GUI::DragFloat("Import Scale", ImportOptions.Scale, 0.01f);
	}

	void CAssetBrowserWindow::ImportOptionsSkeletalMesh()
	{
		GUI::DragFloat("Import Scale", ImportOptions.Scale, 0.01f);
	}

	void CAssetBrowserWindow::ImportOptionsAnimation()
	{
		F32 thumbnailPadding = 4.0f;
		F32 cellWidth = GUI::TexturePreviewSizeX * 0.75f + thumbnailPadding;
		F32 panelWidth = 256.0f;
		I32 columnCount = static_cast<I32>(panelWidth / cellWidth);

		intptr_t assetPickerThumbnail = ImportOptions.AssetRep != nullptr ? (intptr_t)ImportOptions.AssetRep->TextureRef : intptr_t();
		SAssetPickResult result = GUI::AssetPicker("Skeletal Rig", "Skeletal Mesh", assetPickerThumbnail, "Assets/Tests", columnCount, Manager->GetAssetInspectFunction());

		if (result.State == EAssetPickerState::AssetPicked)
			ImportOptions.AssetRep = Manager->GetAssetRepFromDirEntry(result.PickedEntry).get();

		GUI::DragFloat("Import Scale", ImportOptions.Scale, 0.01f);
	}

	void CAssetBrowserWindow::InspectFolderTree(const std::string& folderName, const intptr_t& folderIconID)
	{
		for (const auto& entry : std::filesystem::directory_iterator(folderName))
		{
			if (!entry.is_directory())
				continue;
			
			GUI::TableNextRow();
			GUI::TableNextColumn();
			GUI::PushID(entry.path().string().c_str());

			const auto& path = entry.path();
			auto relativePath = std::filesystem::relative(path);
			std::string filenameString = relativePath.filename().string();

			const bool isOpen = GUI::TreeNode(filenameString.c_str());
			GUI::SameLine();
			GUI::Image(folderIconID, SVector2<F32>(12.0f));

			if (isOpen)
			{
				std::string newPath = relativePath.string();
				InspectFolderTree(newPath, folderIconID);
				GUI::TreePop();
			}

			GUI::PopID();
			
			// NW: If not directory, do we want this?
			//else
			//{
			//	const auto& rep = Manager->GetAssetRepFromDirEntry(entry);
			//	GUI::TreeNodeEx(rep->Name.c_str(), { ETreeNodeFlag::NoTreePushOnOpen, ETreeNodeFlag::Leaf, ETreeNodeFlag::Bullet });
			//}
		}
	}

	void CAssetBrowserWindow::InspectDirectoryEntry(const std::filesystem::directory_entry& entry, U32& outCurrentID, const intptr_t& folderIconID)
	{
		GUI::TableNextColumn();
		GUI::PushID(outCurrentID++);

		const auto& path = entry.path();
		auto relativePath = std::filesystem::relative(path);
		std::string filenameString = relativePath.filename().string();

		if (entry.is_directory())
		{
			if (GUI::ImageButton("FolderIcon", folderIconID, { GUI::ThumbnailSizeX, GUI::ThumbnailSizeY }))
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

			if (GUI::ImageButton("AssetIcon", (intptr_t)rep->TextureRef, { GUI::ThumbnailSizeX, GUI::ThumbnailSizeY }))
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
