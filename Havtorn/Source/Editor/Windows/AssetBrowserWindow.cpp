// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "AssetBrowserWindow.h"

#include "EditorManager.h"
#include "DockSpaceWindow.h"
#include "EditorResourceManager.h"

#include <Engine.h>
#include <Timer.h>
#include <MathTypes/EngineMath.h>
#include <FileSystem.h>
#include <Graphics/RenderManager.h>
#include <PlatformManager.h>
#include <Scene/AssetRegistry.h>

namespace Havtorn
{
	CAssetBrowserWindow::CAssetBrowserWindow(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
	{
		CurrentDirectory = std::filesystem::path(DefaultAssetPath);		
		manager->GetPlatformManager()->OnDragDropAccepted.AddMember(this, &CAssetBrowserWindow::OnDragDropFiles);
		GEngine::GetAssetRegistry()->OnAssetReloaded.AddMember(this, &CAssetBrowserWindow::OnAssetReloaded);
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
			intptr_t folderIconID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::FolderIcon).GetShaderResourceView();

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

			GUI::OffsetCursorPos(SVector2<F32>(4.0f, 0.0f));

			// TODO.NR: Another magic number here, 10 cuts off the right border. 11 seems to work but feels too odd.
			F32 thumbnailPadding = 11.0f;
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

			if (GUI::BeginPopupContextWindow())
			{
				if (IsSelectionHovered)
				{
					SEditorAssetRepresentation* selectedAssetRep = SelectedAsset.value();
					const std::filesystem::directory_entry& directoryEntry = selectedAssetRep->DirectoryEntry;

					if (GUI::MenuItem("Copy Asset Path"))
						GUI::CopyToClipboard(directoryEntry.path().string().c_str());
					
					if (GUI::MenuItem("Delete Asset"))
					{
						std::filesystem::path pathToRemove = directoryEntry.path();
						Manager->RemoveAssetRep(directoryEntry);
						std::filesystem::remove(pathToRemove);
					}

					// TODO.NW: It would be nice with some sort of attribute to check
					// the enum value against (e.g. SourceFileBased), may not exist on our current version though
					if (selectedAssetRep->AssetType != EAssetType::Material
						&& selectedAssetRep->AssetType != EAssetType::Script
						&& selectedAssetRep->AssetType != EAssetType::Scene
						&& selectedAssetRep->AssetType != EAssetType::Sequencer
						)
					{
						if (selectedAssetRep->IsSourceWatched)
						{
							if (GUI::MenuItem("Stop Watching Source Changes"))
							{
								selectedAssetRep->IsSourceWatched = false;
								GEngine::GetAssetRegistry()->StopSourceFileWatch(SAssetReference(directoryEntry.path().string()));
							}
						}
						else
						{
							if (GUI::MenuItem("Start Watching Source Changes"))
							{
								selectedAssetRep->IsSourceWatched = true;
								GEngine::GetAssetRegistry()->StartSourceFileWatch(SAssetReference(directoryEntry.path().string()));
							}
						}
					}
				}
				else
				{
					if (GUI::MenuItem("Create Asset"))
					{
						IsCreatingAsset = true;
						DirectoryToSaveTo = CurrentDirectory.string();
						NewAssetName = "NewAsset";
					}
				}

				GUI::EndPopup();
			}
			else
				IsSelectionHovered = false;

			GUI::EndChild();
		}

		if (FilePathsToImport.has_value() && !FilePathsToImport->empty())
		{
			GUI::OpenPopup("Asset Import");
			GUI::SetNextWindowPos(GUI::GetViewportCenter(), EWindowCondition::Appearing, SVector2<F32>(0.5f, 0.5f));
			AssetImportModal();
		}
		
		if (IsCreatingAsset)
		{
			GUI::OpenPopup("Create Asset");
			GUI::SetNextWindowPos(GUI::GetViewportCenter(), EWindowCondition::Appearing, SVector2<F32>(0.5f, 0.5f));
			AssetCreationModal();
		}

		GUI::End();
		
		if (AnimatedThumbnailTime == LastAnimatedThumbnailTime)
			AnimatedThumbnailTime = 0.0f;

		LastAnimatedThumbnailTime = AnimatedThumbnailTime;

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

	void CAssetBrowserWindow::OnAssetReloaded(const std::string& assetPath)
	{
		// TODO.NW: Maybe add some clearer feedback here that the hot reload was successful?

		std::filesystem::directory_entry assetDir;
		assetDir.assign(std::filesystem::path(assetPath));
		auto& assetRep = Manager->GetAssetRepFromDirEntry(assetDir);

		assetRep->TextureRef = Manager->GetResourceManager()->RenderAssetTexure(assetRep->AssetType, assetPath);
	}

	void AlignForWidth(F32 width, F32 alignment = 0.5f)
	{
		F32 avail = GUI::GetContentRegionAvail().X;
		F32 off = (avail - width) * alignment;
		if (off > 0.0f)
			GUI::OffsetCursorPos(SVector2<F32>(off, 0.0f));
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
			CreateOptionsMaterial();
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
			std::string newFileName = CurrentDirectory.string() + "\\" + UGeneralUtils::ExtractFileBaseNameFromPath(filePath) + ".hva";
			std::filesystem::directory_entry newDir;
			newDir.assign(std::filesystem::path(newFileName));
			Manager->RemoveAssetRep(newDir);

			std::string hvaFilePath = Manager->GetResourceManager()->ConvertToHVA(filePath, CurrentDirectory.string() + "\\", ImportOptions);
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

	void CAssetBrowserWindow::AssetCreationModal()
	{
		if (!GUI::BeginPopupModal("Create Asset", NULL, { EWindowFlag::AlwaysAutoResize }))
			return;

		auto closePopup = [&]()
			{
				IsCreatingAsset = false;
				NewAssetFileHeader = std::monostate();
				Manager->SetIsModalOpen(false);
				GUI::CloseCurrentPopup();
			};

		Manager->SetIsModalOpen(true);


		AssetTypeToCreate = GUI::ComboEnum("Asset Type", AssetTypeToCreate);
		//GUI::SliderEnum("Asset Type", AssetTypeToCreate, 
		//{
		//"None",
		//"StaticMesh (Not Supported Yet)",
		//"SkeletalMesh (Not Supported Yet)",
		//"Texture (Not Supported Yet)",
		//"Material",
		//"Animation (Not Supported Yet)",
		//"SpriteAnimation (Not Supported Yet)",
		//"AudioOneShot (Not Supported Yet)",
		//"AudioCollection (Not Supported Yet)",
		//"VisualFX (Not Supported Yet)",
		//"Scene (Not Supported Yet)",
		//"Sequencer (Not Supported Yet)",
		//"Script"
		//	});

		if (AssetTypeToCreate == EAssetType::None)
			AssetTypeToCreate = EAssetType::StaticMesh;

		GUI::InputText("Destination Folder", DirectoryToSaveTo);
		GUI::InputText("Asset Name", NewAssetName);
		GUI::Separator();

		switch (AssetTypeToCreate)
		{
		case EAssetType::Material:
			NewAssetFileHeader = CreateOptionsMaterial();
			break;
		default:
			break;
		}

		// Center buttons
		F32 width = 0.0f;
		width += GUI::CalculateTextSize("Create").X + GUI::ThumbnailPadding;
		width += GUI::GetStyleVar(EStyleVar::ItemSpacing).X;
		width += GUI::CalculateTextSize("Cancel").X + GUI::ThumbnailPadding;
		AlignForWidth(width);

		if (GUI::Button("Create"))
		{
			switch (AssetTypeToCreate)
			{
			case EAssetType::Script:
				NewAssetFileHeader = CreateScript();
				break;
			default:
				break;
			}

			// TODO.NW: Make proper folder navigation element for choosing DirectoryToSaveTo, validate NewAssetName
			std::string newFilePath = Manager->GetResourceManager()->CreateAsset(DirectoryToSaveTo + "/", NewAssetFileHeader);
			if (newFilePath != "INVALID_PATH")
			{
				std::filesystem::directory_entry newDir;
				newDir.assign(std::filesystem::path(newFilePath));
				Manager->RemoveAssetRep(newDir);
				Manager->CreateAssetRep(newDir);
				closePopup();
			}
			else
			{
				HV_LOG_ERROR("Could not create new asset in path: %s", std::string(DirectoryToSaveTo + "/" + NewAssetName + ".hva").c_str());
			}
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

		intptr_t assetPickerThumbnail = ImportOptions.AssetRep != nullptr ? (intptr_t)ImportOptions.AssetRep->TextureRef.GetShaderResourceView() : intptr_t();
		
		SAssetPickResult result = GUI::AssetPickerFilter("Skeletal Rig", "Skeletal Mesh", assetPickerThumbnail, "Assets/Meshes", columnCount, Manager->GetAssetFilteredInspectFunction(), EAssetType::SkeletalMesh);

		if (result.State == EAssetPickerState::AssetPicked)
			ImportOptions.AssetRep = Manager->GetAssetRepFromDirEntry(result.PickedEntry).get();

		GUI::DragFloat("Import Scale", ImportOptions.Scale, 0.01f);
	}

	SAssetFileHeader CAssetBrowserWindow::CreateOptionsMaterial()
	{
		GUI::TextDisabled("Packing: AlbedoMaterialNormal_Packed");

		F32 thumbnailPadding = 4.0f;
		F32 cellWidth = GUI::TexturePreviewSizeX * 0.75f + thumbnailPadding;
		F32 panelWidth = 256.0f;
		I32 columnCount = static_cast<I32>(panelWidth / cellWidth);

		bool canCreateMaterial = true;
		const std::array<std::string, 3> labels = { "Albedo", "Material", "Normal" };
		for (U64 i = 0; i < 3; i++)
		{
			intptr_t assetPickerThumbnail = NewMaterialTextures[i] != nullptr ? (intptr_t)NewMaterialTextures[i]->TextureRef.GetShaderResourceView() : intptr_t();
			std::string pickerLabel = labels[i].c_str();
			if (NewMaterialTextures[i] != nullptr)
			{
				pickerLabel.append(": ");
				pickerLabel.append(NewMaterialTextures[i]->Name);
			}
			GUI::PushID(labels[i].c_str());
			// TODO.NW: Filter away cubemaps with Axel's filtering
			SAssetPickResult result = GUI::AssetPicker(pickerLabel.c_str(), "Texture", assetPickerThumbnail, "Assets/Textures", columnCount, Manager->GetAssetInspectFunction());
			GUI::PopID();

			if (result.State == EAssetPickerState::AssetPicked)
				NewMaterialTextures[i] = Manager->GetAssetRepFromDirEntry(result.PickedEntry).get();

			if (NewMaterialTextures[i] == nullptr)
				canCreateMaterial = false;
		}

		if (!canCreateMaterial)
			return std::monostate();

		std::string texturePath0 = NewMaterialTextures[0]->DirectoryEntry.path().string();
		std::string texturePath1 = NewMaterialTextures[1]->DirectoryEntry.path().string();
		std::string texturePath2 = NewMaterialTextures[2]->DirectoryEntry.path().string();

		SMaterialAssetFileHeader fileHeader;
		fileHeader.Name = "M_" + NewAssetName;
		fileHeader.Material.Properties[0] = { -1.0f, texturePath0, 0 };
		fileHeader.Material.Properties[1] = { -1.0f, texturePath0, 1 };
		fileHeader.Material.Properties[2] = { -1.0f, texturePath0, 2 };
		fileHeader.Material.Properties[3] = { -1.0f, texturePath0, 3 };
		fileHeader.Material.Properties[4] = { -1.0f, texturePath2, 3 };
		fileHeader.Material.Properties[5] = { -1.0f, texturePath2, 1 };
		fileHeader.Material.Properties[6] = { -1.0f, "", -1 };
		fileHeader.Material.Properties[7] = { -1.0f, texturePath2, 2 };
		fileHeader.Material.Properties[8] = { -1.0f, texturePath1, 0 };
		fileHeader.Material.Properties[9] = { -1.0f, texturePath1, 1 };
		fileHeader.Material.Properties[10] = { -1.0f, texturePath1, 2 };
		fileHeader.Material.RecreateZ = true;

		return fileHeader;
	}

	SAssetFileHeader CAssetBrowserWindow::CreateScript()
	{
		SScriptFileHeader fileHeader = SScriptFileHeader{};
		fileHeader.AssetType = EAssetType::Script;
		fileHeader.Name = NewAssetName;
		fileHeader.Script = new HexRune::SScript();
		return fileHeader;
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

			// Asset Drag
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

						CJsonDocument config = UFileSystem::OpenJson(UFileSystem::EngineConfig);
						config.WriteValueToArray("Asset Redirectors", oldPath, newPath);

						Manager->RemoveAssetRep(payloadAssetRep->DirectoryEntry);
						std::filesystem::rename(oldPath, newPath);
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
		GUI::PushID(STATIC_I32(outCurrentID++));

		if (entry.is_directory())
		{
			if (GUI::ImageButton("FolderIcon", folderIconID, { GUI::ThumbnailSizeX, GUI::ThumbnailSizeY }))
			{
				CurrentDirectory = entry.path();
			}

			auto relativePath = std::filesystem::relative(entry.path());
			std::string filenameString = relativePath.filename().string();

			GUI::Text(filenameString.c_str());
			if (GUI::IsItemHovered())
				GUI::SetTooltip(filenameString.c_str());
		}
		else
		{
			const auto& rep = Manager->GetAssetRepFromDirEntry(entry);
			if (!rep->TextureRef.IsShaderResourceValid())
				rep->TextureRef = Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::FileIcon);

			SRenderAssetCardResult result = GUI::RenderAssetCard(rep->Name.c_str(), rep.get() == SelectedAsset.value_or(nullptr), (intptr_t)rep->TextureRef.GetShaderResourceView(), GetAssetTypeDetailName(rep->AssetType).c_str(), GetAssetTypeColor(rep->AssetType), rep->IsSourceWatched ? SColor::Orange : SColor(10), rep.get(), sizeof(SEditorAssetRepresentation));

			if (result.IsClicked)
				SelectedAsset = rep.get();

			if (result.IsDoubleClicked)
			{
				// NW: Open Tool depending on asset type?
				HV_LOG_INFO("Clicked asset: %s", rep->Name.c_str());
				Manager->OpenAssetTool(rep.get());
				SelectedAsset.reset();
			}
			
			if (result.IsHovered)
			{
				if (rep->AssetType == EAssetType::Animation)
				{
					Manager->GetResourceManager()->AnimateAssetTexture(rep->TextureRef, rep->AssetType, entry.path().string(), AnimatedThumbnailTime += GTime::Dt());
				}

				if (SelectedAsset.has_value() && rep.get() == SelectedAsset.value())
				{
					IsSelectionHovered = true;
				}
			}
		}

		GUI::PopID();
	}
}
