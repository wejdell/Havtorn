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

			{ // Menu Bar
				GUI::BeginChild("MenuBar", SVector2<F32>(0.0f, 30.0f));
				GUI::Image(folderIconID, SVector2<F32>(12.0f));
				GUI::SameLine();
				GUI::Text(CurrentDirectory.string().c_str());
				
				// TODO.NW: Move search bar?
				GUI::SameLine();
				if (GUI::ArrowButton("GoBackDir", EGUIDirection::Left))
				{
					if (CurrentDirectory != std::filesystem::path(DefaultAssetPath))
						CurrentDirectory = CurrentDirectory.parent_path();
				}
				GUI::SameLine();
				Filter.Draw("Search", 180);
				GUI::Separator();
				
				GUI::EndChild();
			}

			{ // Folder Tree
				GUI::BeginChild("FolderTree", SVector2<F32>(150.0f, 0.0f), { EChildFlag::Borders, EChildFlag::ResizeX });
				GUI::Text("Project Name");
				GUI::Separator();

				if (GUI::BeginTable("FolderTreeTable", 1))
				{
					InspectFolderTree(DefaultAssetPath, folderIconID);
					GUI::EndTable();
				}
				GUI::EndChild();
				GUI::SameLine();
			}

			GUI::BeginChild("Browser");

			// TODO.NR: Another magic number here, 10 cuts off the right border. 11 seems to work but feels too odd.
			F32 thumbnailPadding = 12.0f;
			F32 cellWidth = GUI::ThumbnailSizeX + thumbnailPadding;
			F32 panelWidth = GUI::GetContentRegionAvail().X;
			I32 columnCount = UMath::Max(static_cast<I32>(panelWidth / cellWidth), 1);

			U32 id = 0;
			if (GUI::BeginTable("AssetTable", columnCount))
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
		}

		if (FilePathsToImport.has_value() && !FilePathsToImport->empty())
		{
			GUI::OpenPopup("Asset Import");
			GUI::SetNextWindowPos(GUI::GetViewportCenter(), EWindowCondition::Appearing, SVector2<F32>(0.5f, 0.5f));
			AssetImportModal();
		}

		//if (GUI::IsDoubleClick() && SelectedAsset.has_value())
		//{
		//	// NW: Open Tool depending on asset type?
		//	auto& rep = Manager->GetAssetRepFromDirEntry(SelectedAsset.value());
		//	HV_LOG_INFO("Clicked asset: %s", rep->Name.c_str());
		//	SelectedAsset.reset();
		//}

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
			// TODO.NW: Make sure thumbnail is re-rendered?
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

			if (GUI::BeginDragDropTarget())
			{
				SGuiPayload payload = GUI::AcceptDragDropPayload("AssetDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNopreviewTooltip });
				if (payload.Data != nullptr)
				{
					// NW: Respond to target, check type
					SEditorAssetRepresentation* payloadAssetRep = reinterpret_cast<SEditorAssetRepresentation*>(payload.Data);
					GUI::SetTooltip("Move '%s' to '%s'?", payloadAssetRep->Name.c_str(), entry.path().string().c_str());

					if (payload.IsDelivery)
					{
						// TODO.NW: Should we move to the destination directory when moving things? Maybe auto-select the new asset rep?
						CurrentDirectory = entry.path();

						std::string oldPath = payloadAssetRep->DirectoryEntry.path().string().c_str();
						std::string newPath = (entry.path() / payloadAssetRep->DirectoryEntry.path().filename()).string().c_str();
						Manager->RemoveAssetRep(payloadAssetRep->DirectoryEntry);
						std::filesystem::rename(oldPath, newPath);
						// TODO.NW: Figre out how to draw correct thumbnails when they're rendered last, must be some state persisting from ImGui
						Manager->CreateAssetRep(newPath);
					}
				}

				GUI::EndDragDropTarget();
			}

			if (GUI::IsItemClicked())
			{
				CurrentDirectory = entry.path();
			}

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

			const bool isSelected = SelectedAsset.has_value() && entry == SelectedAsset.value();
			
			SVector2<F32> selectablePos = GUI::GetCursorPos();
			SVector2<F32> framePadding = GUI::GetStyleVar(EStyleVar::FramePadding);

			// TODO.NW: Figure out strange size diff between Selectable and ImageButton. Button looks nicer with a more appealing padding
			// - Make asset cards like in UE, looks pretty good to combine into rectangular portraits with thumbnail, colored stripe for asset type,
			// - and label.
			if (GUI::Selectable("", isSelected, { ESelectableFlag::AllowDoubleClick, ESelectableFlag::AllowOverlap }, { GUI::ThumbnailSizeX + framePadding.X * 0.5f, GUI::ThumbnailSizeY + framePadding.Y * 0.5f }))
			{
				SelectedAsset = entry;
				if (GUI::IsDoubleClick())
				{
					// NW: Open Tool depending on asset type?
					HV_LOG_INFO("Clicked asset: %s", rep->Name.c_str());
					SelectedAsset.reset();
				}
			}

			if (GUI::BeginDragDropSource())
			{
				SGuiPayload payload = GUI::GetDragDropPayload();
				if (payload.Data == nullptr)
				{
					GUI::SetDragDropPayload("AssetDrag", rep.get(), sizeof(SEditorAssetRepresentation));
				}
				payload = GUI::GetDragDropPayload();
				SEditorAssetRepresentation* payloadAssetRep = reinterpret_cast<SEditorAssetRepresentation*>(payload.Data);
				GUI::Text(payloadAssetRep->Name.c_str());

				GUI::EndDragDropSource();
			}

			SVector2<F32> nextPos = GUI::GetCursorPos();
			GUI::SetCursorPos(selectablePos);
		
			SColor imageBorderColor = SColor::White;
			switch (rep->AssetType)
			{
			case EAssetType::Animation:
				imageBorderColor = SColor::Blue;
				break;
			case EAssetType::Material:
				imageBorderColor = SColor::Green;
				break;
			case EAssetType::SkeletalMesh:
				imageBorderColor = SColor::Magenta;
				break;
			case EAssetType::StaticMesh:
				imageBorderColor = SColor::Teal;
				break;
			case EAssetType::Texture:
				imageBorderColor = SColor::Red;
				break;
			default:
				break;
			}
			imageBorderColor.A = SColor::ToU8Range(0.5f);

			GUI::Image((intptr_t)rep->TextureRef, { GUI::ThumbnailSizeX, GUI::ThumbnailSizeY }, SVector2<F32>(0.0f), SVector2<F32>(1.0f), SColor::White, imageBorderColor);
			GUI::SetCursorPos(nextPos);

			GUI::Text(rep->Name.c_str());
			if (GUI::IsItemHovered())
				GUI::SetTooltip(rep->Name.c_str());
		}

		GUI::PopID();
	}
}
