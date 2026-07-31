// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "AssetBrowserWindow.h"

#include "EditorManager.h"
#include "DockSpaceWindow.h"
#include "EditorResourceManager.h"

#include "EditActions/BrowseFolderEditAction.h"

#include <Engine.h>
#include <Timer.h>
#include <MathTypes/EngineMath.h>
#include <FileSystem.h>
#include <Graphics/RenderManager.h>
#include <PlatformManager.h>
#include <Assets/AssetRegistry.h>

#include <Input/InputMapper.h>
#include <Input/InputTypes.h>

#include <../Game/GameScene.h>
#include <../Game/GameScript.h>

namespace Havtorn
{
	CAssetBrowserWindow::CAssetBrowserWindow(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
	{
		CurrentDirectory = std::filesystem::path(DefaultAssetPath);		
		manager->GetPlatformManager()->OnDragDropAccepted.AddMember(this, &CAssetBrowserWindow::OnDragDropFiles);
		GEngine::GetAssetRegistry()->OnAssetReloaded.AddMember(this, &CAssetBrowserWindow::OnAssetReloaded);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::Rename).AddMember(this, &CAssetBrowserWindow::OnRenameEvent);
	}

	CAssetBrowserWindow::~CAssetBrowserWindow()
	{		
	}

	void CAssetBrowserWindow::OnEnable()
	{
	}

	void CAssetBrowserWindow::OnInspectorGUI()
	{
		if (GUI::Begin(Name(), nullptr, { EWindowFlag::NoMove, /*EWindowFlag::NoResize, */EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus}))
		{
			IsHovered = IsEnabled && GUI::IsWindowHovered();

			intptr_t folderIconID = Manager->GetResourceManager()->GetStaticEditorTextureResource(EEditorTexture::FolderIcon);

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
						SetCurrentPath(CurrentDirectory.parent_path());
				}
				GUI::SameLine();
				Filter.Draw("Search", 180);
				GUI::Separator();
				
				GUI::EndChild();
			}

			{ // Folder Tree
				GUI::BeginChild("FolderTree", SVector2<F32>(150.0f, 0.0f), { EChildFlag::Borders, EChildFlag::ResizeX });
				GUI::Text(Manager->GetProjectName().data());
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
				GUI::PushClipRect(GUI::GetCursorScreenPos(), GUI::GetContentRegionAvail());
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
				GUI::PopClipRect();
				GUI::EndTable();
			}

			if (GUI::BeginPopupContextWindow())
			{
				if (HoveredAsset.has_value())
				{
					SEditorAssetRepresentation* hoveredAssetRep = HoveredAsset.value();
					if (hoveredAssetRep != nullptr)
					{
						const std::filesystem::directory_entry& directoryEntry = hoveredAssetRep->DirectoryEntry;

						if (GUI::MenuItem("Rename", "F2"))
							hoveredAssetRep->IsBeingNamed = true;

						if (GUI::MenuItem("Copy Asset Path"))
							GUI::CopyToClipboard(directoryEntry.path().string().c_str());
					
						if (GUI::MenuItem("Copy Focus Asset Link"))
							GUI::CopyToClipboard(Manager->GetAssetFocusLink(hoveredAssetRep).data());
						if (GUI::IsItemHovered())
							GUI::SetTooltip("Copies a shareable link for focusing this asset");

						if (GUI::MenuItem("Delete Asset"))
						{
							std::filesystem::path pathToRemove = directoryEntry.path();
							Manager->RemoveAssetRep(directoryEntry);
							UFileSystem::Remove(pathToRemove.string());
						}

						if (IsAssetSourceFileBased(hoveredAssetRep->AssetType))
						{
							if (hoveredAssetRep->IsSourceWatched)
							{
								if (GUI::MenuItem("Stop Watching Source Changes"))
								{
									hoveredAssetRep->IsSourceWatched = false;
									GEngine::GetAssetRegistry()->StopSourceFileWatch(SAssetReference(directoryEntry.path().string()));
								}
							}
							else
							{
								if (GUI::MenuItem("Start Watching Source Changes"))
								{
									hoveredAssetRep->IsSourceWatched = true;
									GEngine::GetAssetRegistry()->StartSourceFileWatch(SAssetReference(directoryEntry.path().string()));
								}
							}
						}
					}
				}
				else if (HoveredFolder.has_value())
				{
					if (!UFileSystem::IsEmpty(HoveredFolder.value().path().string()))
					{
						GUI::TextDisabled("Rename");
						if (GUI::IsItemHovered())
							GUI::SetTooltip("Can't yet rename folders containing items. Please move or remove the contents so they can be redirected.");
					}
					else
					{
						if (GUI::MenuItem("Rename", "F2"))
							FolderBeingRenamed = HoveredFolder;
					}

					if (GUI::MenuItem("Copy Path"))
						GUI::CopyToClipboard(HoveredFolder.value().path().string().c_str());
					
					if (!UFileSystem::IsEmpty(HoveredFolder.value().path().string()))
					{
						GUI::TextDisabled("Delete Folder");
						if (GUI::IsItemHovered())
							GUI::SetTooltip("Can't yet delete folders containing items. Please move or remove the contents.");
					}
					else
					{
						if (GUI::MenuItem("Delete Folder"))
							UFileSystem::Remove(HoveredFolder.value().path().string());
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

					if (GUI::MenuItem("Create Folder"))
					{
						std::string newFolderName = CurrentDirectory.string() + "/NewFolder";

						std::vector<std::string> folderNames;
						for (const auto& entry : std::filesystem::directory_iterator(CurrentDirectory))
						{
							if (!entry.is_directory())
								continue;

							folderNames.push_back(UGeneralUtils::ConvertToPlatformAgnosticPath(entry.path().string()));
						}

						newFolderName = UGeneralUtils::GetNonCollidingString(newFolderName, folderNames, [](const std::string& folderName){ return folderName;});
						UFileSystem::AddDirectory(newFolderName);
					}
				}

				GUI::EndPopup();
			}
			else
			{
				HoveredAsset.reset();
				HoveredFolder.reset();
			}

			GUI::EndChild();
		}

		if (IsCreatingAsset)
		{
			GUI::OpenPopup("Create Asset");
			GUI::SetNextWindowPos(GUI::GetViewportCenter(), EWindowCondition::Appearing, SVector2<F32>(0.5f, 0.5f));
			AssetCreationModal();
		}

		if (FilePathsToImport.has_value() && !FilePathsToImport->empty())
		{
			GUI::OpenPopup("Asset Import");
			GUI::SetNextWindowPos(GUI::GetViewportCenter(), EWindowCondition::Appearing, SVector2<F32>(0.5f, 0.5f));
			AssetImportModal();
		}
		
		GUI::End();

		// ANIMATED THUMBNAILS
		if (AnimatingThumbnailAsset.has_value())
		{
			SEditorAssetRepresentation* animatingThumbnailAsset = AnimatingThumbnailAsset.value();
			U32 assetID = SAssetReference(animatingThumbnailAsset->Name).UID;

			// Start hover animated thumbnail
			if (!WasAnimatingThumbnail)
			{
				Manager->GetRenderManager()->RequestRenderView(assetID);
				PreviouslyAnimatingThumbnailAsset = animatingThumbnailAsset;
			}

			Manager->GetResourceManager()->AnimateAssetTexture(animatingThumbnailAsset, animatingThumbnailAsset->DirectoryEntry.path().string(), AnimatedThumbnailTime += GTime::Dt());
			CRenderTexture* animatedThumbnail = Manager->GetRenderManager()->GetRenderTargetTexture(assetID);
			if (animatedThumbnail != nullptr)
				animatingThumbnailAsset->TextureRef = *animatedThumbnail;
		}
		else if (!AnimatingThumbnailAsset.has_value() && WasAnimatingThumbnail)
		{
			HV_ASSERT(PreviouslyAnimatingThumbnailAsset, "When we stop hovering a thumbnail asset, we expect the PreviouslyAnimatingThumbnailAsset to be set!");

			// End hover animated thumbnail
			U32 assetID = SAssetReference(PreviouslyAnimatingThumbnailAsset->Name).UID;
			Manager->GetRenderManager()->UnrequestRenderView(assetID);
			Manager->GetResourceManager()->RequestThumbnailRender(PreviouslyAnimatingThumbnailAsset, PreviouslyAnimatingThumbnailAsset->DirectoryEntry.path().string());
			PreviouslyAnimatingThumbnailAsset = nullptr;
		}

		if (AnimatedThumbnailTime == LastAnimatedThumbnailTime)
			AnimatedThumbnailTime = 0.0f;

		LastAnimatedThumbnailTime = AnimatedThumbnailTime;
		WasAnimatingThumbnail = AnimatingThumbnailAsset.has_value();
		AnimatingThumbnailAsset.reset();

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

	void CAssetBrowserWindow::BrowseTo(SEditorAssetRepresentation* assetRep)
	{
		if (assetRep == nullptr || !UFileSystem::Exists(assetRep->DirectoryEntry.path().parent_path().string()))
			return;

		SetCurrentPath(assetRep->DirectoryEntry.path().parent_path());
		Manager->SetSelectedAsset(assetRep);
	}

	void CAssetBrowserWindow::SetCurrentPath(const std::filesystem::path& path, const bool pushCommand)
	{
		if (pushCommand)
			UMetaCommandRouter::Push(SBrowseFolderEditAction::MakeEditActionCommand(CurrentDirectory, path));
		
		CurrentDirectory = path;
	}

	SRenderAssetCardResult CAssetBrowserWindow::RenderAssetCard(SEditorAssetRepresentation* assetRep, const SColor& borderColor)
	{
		SRenderAssetCardResult result;

		SVector2<F32> cardStartPos = GUI::GetCursorPos();
		SVector2<F32> framePadding = GUI::GetStyleVar(EStyleVar::FramePadding);

		SVector2<F32> cardSize = { GUI::ThumbnailSizeX + framePadding.X * 0.5f, GUI::ThumbnailSizeY + framePadding.Y * 0.5f };
		cardSize.Y *= 1.6f;
		SVector2<F32> thumbnailSize = { GUI::ThumbnailSizeX + framePadding.X * 0.5f, GUI::ThumbnailSizeY + framePadding.Y * 0.5f + 4.0f };

		// TODO.NW: Can't seem to get the leftmost line to show correctly. Maybe need to start the table as usual and then offset inwards?
		constexpr F32 borderThickness = 1.0f;
		GUI::SetCursorPos(cardStartPos + SVector2<F32>(-1.0f * borderThickness));
		GUI::AddRectFilled(GUI::GetCursorScreenPos(), cardSize + SVector2<F32>(2.0f * borderThickness), borderColor);
		GUI::SetCursorPos(cardStartPos);
		GUI::AddRectFilled(GUI::GetCursorScreenPos(), cardSize, SColor(65));
		GUI::SetCursorPos(cardStartPos);
		GUI::AddRectFilled(GUI::GetCursorScreenPos(), thumbnailSize, SColor(40));
		GUI::SetCursorPos(cardStartPos);

		if (GUI::Selectable("", false, { ESelectableFlag::AllowDoubleClick, ESelectableFlag::AllowOverlap }, cardSize))
		{
			if (GUI::IsMouseReleased())
				result.IsClicked = true;
			if (GUI::IsDoubleClick())
				result.IsDoubleClicked = true;
		}

		const char* label = assetRep->Name.c_str();
		CEditorManager::AssetDragData.TrySet(*assetRep, label, {});
		
		GUI::SetCursorPos(cardStartPos + SVector2<F32>(1.0f, 0.0f));
		GUI::Image(Manager->GetTextureResourceFromAssetRep(assetRep), { GUI::ThumbnailSizeX, GUI::ThumbnailSizeY }, SVector2<F32>(0.0f), SVector2<F32>(1.0f), SColor::White);

		SColor detailColor = GetAssetTypeColor(assetRep->AssetType);
		detailColor.A = SColor::ToU8Range(0.5f);
		GUI::AddRectFilled(GUI::GetCursorScreenPos(), SVector2<F32>(cardSize.X, 2.0f), detailColor);

		GUI::OffsetCursorPos(SVector2<F32>(2.0f, 4.0f));

		if (GUI::IsItemHovered())
		{
			result.IsHovered = true;
		}

		GUI::PushClipRect(GUI::GetCursorScreenPos(), cardSize - framePadding);

		if (assetRep->IsBeingNamed)
		{
			GUI::SetKeyboardFocusHere();

			std::string newAssetName = label;
			GUI::PushID(label);
			if (GUI::InputText("", newAssetName))
			{
				if (GUI::IsItemDeactivatedAfterEdit())
					result.NewAssetName = newAssetName;
			}

			if (GUI::IsItemDeactivated() && !result.NewAssetName.has_value())
				result.NewAssetName = label;

			GUI::PopID();
		}
		else
			GUI::Text(label);

		if (GUI::IsItemHovered())
			GUI::SetTooltip(label);

		GUI::OffsetCursorPos(SVector2<F32>(2.0f, -2.0f));
		GUI::SetSecondaryFontActive(true);
		GUI::TextDisabled(GetAssetTypeDetailName(assetRep->AssetType).c_str());
		GUI::SetSecondaryFontActive(false);
		GUI::PopClipRect();

		return result;
	}

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

		Manager->GetResourceManager()->RequestThumbnailRender(assetRep.get(), assetPath);
	}

	void CAssetBrowserWindow::OnRenameEvent(const SInputActionPayload payload)
	{
		if (payload.IsPressed && Manager->GetSelectedAsset() != nullptr)
		{
			// TODO.NW: Figure out a way to close the context menu
			Manager->GetSelectedAsset()->IsBeingNamed = true;
			return;
		}

		const std::optional<std::filesystem::directory_entry> selectedFolder = Manager->GetSelectedFolder();
		if (payload.IsPressed && selectedFolder.has_value())
		{
			if (UFileSystem::IsEmpty(selectedFolder.value().path().string()))
				FolderBeingRenamed = selectedFolder;
			else
				HV_LOG_WARN("Can't yet rename folders containing items. Please move or remove the contents so they can be redirected.");
		}
	}

	void AlignForWidth(F32 width, F32 alignment = 0.5f)
	{
		F32 avail = GUI::GetContentRegionAvail().X;
		F32 off = (avail - width) * alignment;
		if (off > 0.0f)
			GUI::OffsetCursorPos(SVector2<F32>(off, 0.0f));
	}

	void CAssetBrowserWindow::AssetCreationModal()
	{
		if (!GUI::BeginPopupModal("Create Asset", NULL, { EWindowFlag::AlwaysAutoResize }))
			return;

		auto closePopup = [&]()
		{
			IsCreatingAsset = false;
			NewAssetFileHeader = NullVariant();
			Manager->SetIsModalOpen(false);
			GUI::CloseCurrentPopup();
		};

		Manager->SetIsModalOpen(true);

		GUI::ComboEnum("Asset Type", AssetTypeToCreate, 
			{ EAssetType::None
			, EAssetType::StaticMesh
			, EAssetType::SkeletalMesh
			, EAssetType::SkeletalAnimation
			, EAssetType::Texture
			, EAssetType::TextureCube
			, EAssetType::SpriteAnimation
			, EAssetType::AudioClip
			, EAssetType::Sequencer });

		GUI::InputText("Destination Folder", DirectoryToSaveTo);
		GUI::InputText("Asset Name", NewAssetName);
		GUI::Separator();

		switch (AssetTypeToCreate)
		{
		case EAssetType::Material:
			NewAssetFileHeader = CreateOptionsMaterial();
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
				NewAssetFileHeader = SScriptFileHeader{ .Name = NewAssetName };
				break;
			case EAssetType::Scene:
				NewAssetFileHeader = SSceneFileHeader{ .Name = NewAssetName };
				break;
			case EAssetType::InputAsset:
				NewAssetFileHeader = SInputAssetFileHeader{ .Name = NewAssetName };
				break;
			case EAssetType::Prefab:
				NewAssetFileHeader = SPrefabFileHeader{ .Name = NewAssetName };
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
			intptr_t assetPickerThumbnail = Manager->GetTextureResourceFromAssetRep(NewMaterialTextures[i]);
			std::string pickerLabel = labels[i].c_str();
			if (NewMaterialTextures[i] != nullptr)
			{
				pickerLabel.append(": ");
				pickerLabel.append(NewMaterialTextures[i]->Name);
			}
			GUI::PushID(labels[i].c_str());
			SAssetPickResult result = GUI::AssetPickerDropdownFilter(pickerLabel.c_str(), GetAssetTypeDetailName(EAssetType::Texture).c_str(), assetPickerThumbnail, Manager->GetResourceManager()->GetStaticEditorTextureResource(EEditorTexture::GetFromSource), Manager->GetResourceManager()->GetStaticEditorTextureResource(EEditorTexture::FindIcon), "Assets", columnCount, Manager->GetAssetFilteredInspectFunction(), EAssetType::Texture);
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
		fileHeader.Material.Properties[6] = { -1.0f, CHavtornStaticString<128>(), -1 };
		fileHeader.Material.Properties[7] = { -1.0f, texturePath2, 2 };
		fileHeader.Material.Properties[8] = { -1.0f, texturePath1, 0 };
		fileHeader.Material.Properties[9] = { -1.0f, texturePath1, 1 };
		fileHeader.Material.Properties[10] = { -1.0f, texturePath1, 2 };
		fileHeader.Material.RecreateZ = true;

		return fileHeader;
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

		std::string filePath = UGeneralUtils::ConvertToPlatformAgnosticPath(*FilePathsToImport->begin());
		GUI::Text("Importing: %s", UGeneralUtils::ExtractFileNameFromPath(filePath).c_str());
		GUI::Separator();

		std::string fileExtension = UGeneralUtils::ExtractFileExtensionFromPath(filePath);
		
		if (fileExtension == "dds" || fileExtension == "tga" || fileExtension == "png")
		{
			if (ImportOptions.SourceData.AssetType == EAssetType::None)
				ImportOptions.SourceData.AssetType = EAssetType::Texture;

			GUI::Text("Asset Type");
			GUI::SameLine();
			if (GUI::RadioButton("Texture", ImportOptions.SourceData.AssetType == EAssetType::Texture))
				ImportOptions.SourceData.AssetType = EAssetType::Texture;
			GUI::SameLine();
			if (GUI::RadioButton("TextureCube", ImportOptions.SourceData.AssetType == EAssetType::TextureCube))
				ImportOptions.SourceData.AssetType = EAssetType::TextureCube;
			GUI::SameLine();
			if (GUI::RadioButton("Material", ImportOptions.SourceData.AssetType == EAssetType::Material))
				ImportOptions.SourceData.AssetType = EAssetType::Material;
			GUI::SameLine();
			if (GUI::RadioButton("Sprite Animation", ImportOptions.SourceData.AssetType == EAssetType::SpriteAnimation))
				ImportOptions.SourceData.AssetType = EAssetType::SpriteAnimation;
		}
		else if (fileExtension == "fbx" || fileExtension == "obj" || fileExtension == "stl")
		{
			if (ImportOptions.SourceData.AssetType == EAssetType::None)
				ImportOptions.SourceData.AssetType = EAssetType::StaticMesh;

			GUI::Text("Asset Type");
			GUI::SameLine();
			if (GUI::RadioButton("Static Mesh", ImportOptions.SourceData.AssetType == EAssetType::StaticMesh))
				ImportOptions.SourceData.AssetType = EAssetType::StaticMesh;
			GUI::SameLine();
			if (GUI::RadioButton("Skeletal Mesh", ImportOptions.SourceData.AssetType == EAssetType::SkeletalMesh))
				ImportOptions.SourceData.AssetType = EAssetType::SkeletalMesh;
			GUI::SameLine();
			if (GUI::RadioButton("Skeletal Animation", ImportOptions.SourceData.AssetType == EAssetType::SkeletalAnimation))
				ImportOptions.SourceData.AssetType = EAssetType::SkeletalAnimation;
		}
		else if (fileExtension == "bnk" || fileExtension == "bank" || fileExtension == "wav" || fileExtension == "mp3" || fileExtension == "ogg")
		{
			if (ImportOptions.SourceData.AssetType == EAssetType::None)
				ImportOptions.SourceData.AssetType = EAssetType::AudioClip;

			GUI::Text("Asset Type");
			GUI::SameLine();
			GUI::TextDisabled("Audio Clip");
		}

		GUI::Separator();

		switch (ImportOptions.SourceData.AssetType)
		{
		case EAssetType::StaticMesh:
			ImportOptionsStaticMesh(filePath);
			break;
		case EAssetType::SkeletalMesh:
			ImportOptionsSkeletalMesh(filePath);
			break;
		case EAssetType::Texture:
			ImportOptionsTexture(filePath);
			break;
		case EAssetType::TextureCube:
			ImportOptionsTextureCube(filePath);
			break;
		case EAssetType::Material:
			CreateOptionsMaterial();
			break;
		case EAssetType::SkeletalAnimation:
			ImportOptionsSkeletalAnimation(filePath);
			break;
		case EAssetType::SpriteAnimation:
			ImportOptionsSpriteAnimation(filePath);
			break;
		case EAssetType::AudioClip:
			ImportOptionsAudioClip(filePath);
			break;
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

			std::string hvaFilePath = Manager->GetResourceManager()->ConvertToHVA(filePath, CurrentDirectory.string() + "\\", ImportOptions.SourceData);
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

	void CAssetBrowserWindow::ImportOptionsTexture(const std::string& sourceFilePath)
	{
		if (!std::holds_alternative<NullVariant>(ImportOptions.SourceData.Variant))
			return;

		ETextureFormat format = {};
		if (const std::string extension = UGeneralUtils::ExtractFileExtensionFromPath(sourceFilePath); extension == "dds")
			format = ETextureFormat::DDS;
		else if (extension == "tga")
			format = ETextureFormat::TGA;

		ImportOptions.SourceData = SSourceAssetData{ .AssetType = EAssetType::Texture, .Version = 1, .SourcePath = sourceFilePath, .Variant = STextureSourceData{ .OriginalFormat = format } };
	}

	void CAssetBrowserWindow::ImportOptionsTextureCube(const std::string& sourceFilePath)
	{
		if (!std::holds_alternative<NullVariant>(ImportOptions.SourceData.Variant))
			return;

		ETextureFormat format = {};
		if (const std::string extension = UGeneralUtils::ExtractFileExtensionFromPath(sourceFilePath); extension == "dds")
			format = ETextureFormat::DDS;
		else if (extension == "tga")
			format = ETextureFormat::TGA;

		ImportOptions.SourceData = SSourceAssetData{ .AssetType = EAssetType::TextureCube, .Version = 1, .SourcePath = sourceFilePath, .Variant = STextureSourceData{ .OriginalFormat = format } };
	}

	void CAssetBrowserWindow::ImportOptionsSpriteAnimation(const std::string& /*sourceFilePath*/)
	{
		GUI::TextDisabled("Sprite Animation importing has not yet been implemented. See CAssetBrowserWindow::AssetImportModal.");
	}

	void CAssetBrowserWindow::ImportOptionsStaticMesh(const std::string& sourceFilePath)
	{
		if (!std::holds_alternative<SStaticMeshSourceData>(ImportOptions.SourceData.Variant))
			ImportOptions.SourceData = SSourceAssetData{ .AssetType = EAssetType::StaticMesh, .Version = 1, .SourcePath = sourceFilePath, .Variant = SStaticMeshSourceData{} };

		SStaticMeshSourceData& sourceData = std::get<SStaticMeshSourceData>(ImportOptions.SourceData.Variant);
		GUI::DragFloat("Import Scale", sourceData.ImportScale, 0.01f);
	}

	void CAssetBrowserWindow::ImportOptionsSkeletalMesh(const std::string& sourceFilePath)
	{
		if (!std::holds_alternative<SSkeletalMeshSourceData>(ImportOptions.SourceData.Variant))
			ImportOptions.SourceData = SSourceAssetData{ .AssetType = EAssetType::SkeletalMesh, .Version = 1, .SourcePath = sourceFilePath, .Variant = SSkeletalMeshSourceData{} };

		SSkeletalMeshSourceData& sourceData = std::get<SSkeletalMeshSourceData>(ImportOptions.SourceData.Variant);
		GUI::DragFloat("Import Scale", sourceData.ImportScale, 0.01f);
	}

	void CAssetBrowserWindow::ImportOptionsSkeletalAnimation(const std::string& sourceFilePath)
	{
		if (!std::holds_alternative<SSkeletalAnimationSourceData>(ImportOptions.SourceData.Variant))
			ImportOptions.SourceData = SSourceAssetData{ .AssetType = EAssetType::SkeletalAnimation, .Version = 1, .SourcePath = sourceFilePath, .Variant = SSkeletalAnimationSourceData{} };

		SSkeletalAnimationSourceData& sourceData = std::get<SSkeletalAnimationSourceData>(ImportOptions.SourceData.Variant);

		constexpr F32 thumbnailPadding = 4.0f;
		const F32 cellWidth = GUI::TexturePreviewSizeX * 0.75f + thumbnailPadding;
		constexpr F32 panelWidth = 256.0f;
		const I32 columnCount = static_cast<I32>(panelWidth / cellWidth);

		intptr_t assetPickerThumbnail = Manager->GetTextureResourceFromAssetRep(ImportOptions.AssetRep);
		
		SAssetPickResult result = GUI::AssetPickerFilter("Skeletal Rig", "Skeletal Mesh", assetPickerThumbnail, "Assets/Meshes", columnCount, Manager->GetAssetFilteredInspectFunction(), EAssetType::SkeletalMesh);

		if (result.State == EAssetPickerState::AssetPicked)
		{
			ImportOptions.AssetRep = Manager->GetAssetRepFromDirEntry(result.PickedEntry).get();
			sourceData.RigMeshPath = UGeneralUtils::ConvertToPlatformAgnosticPath(ImportOptions.AssetRep->DirectoryEntry.path().string());
		}

		GUI::DragFloat("Import Scale", sourceData.ImportScale, 0.01f);
	}

	void CAssetBrowserWindow::ImportOptionsAudioClip(const std::string& sourceFilePath)
	{
		if (!std::holds_alternative<SAudioClipSourceData>(ImportOptions.SourceData.Variant))
			ImportOptions.SourceData = SSourceAssetData{ .AssetType = EAssetType::AudioClip, .Version = 1, .SourcePath = sourceFilePath, .Variant = SAudioClipSourceData{} };

#ifdef HV_AUDIO_BACKEND_WWISE
		GUI::TextDisabled("Note: When using Wwise as the audio backend, looping and spatialization settings are set in Wwise Authoring.");
#elif defined HV_AUDIO_BACKEND_FMOD
		SAudioClipSourceData& sourceData = std::get<SAudioClipSourceData>(ImportOptions.SourceData.Variant);
		GUI::Checkbox("Is Looping", sourceData.Settings.IsLooping);
		GUI::Checkbox("Is Spatialized", sourceData.Settings.IsSpatialized);
#elif defined HV_AUDIO_BACKEND_SDL
		GUI::TextDisabled("Note: The SDL Audio implementation does not yet support looping and spatializing sounds. We'd need a separate plugin for that (pending decision).");
#endif
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

			const bool isOpen = GUI::TreeNodeEx(filenameString.c_str(), { ETreeNodeFlag::OpenOnDoubleClick });

			// Asset Drag
			auto result = CEditorManager::AssetDragData.TryDeliver({ EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNopreviewTooltip });
			if (result.Payload != nullptr)
			{
				// NW: Respond to target, check type
				SEditorAssetRepresentation* payloadAssetRep = result.Payload;
				GUI::SetTooltip("Move '%s' to '%s'?", payloadAssetRep->Name.c_str(), entry.path().string().c_str());

				if (result.Result == EDragDeliverResult::Delivered)
				{
					// TODO.NW: Should we move to the destination directory when moving things? Maybe auto-select the new asset rep?
					SetCurrentPath(entry.path());

					std::string oldPath = payloadAssetRep->DirectoryEntry.path().string().c_str();
					std::string newPath = (entry.path() / payloadAssetRep->DirectoryEntry.path().filename()).string().c_str();

					CJsonDocument config = UFileSystem::OpenJson(UFileSystem::EngineConfig);
					config.WriteValueToArray("Asset Redirectors", oldPath, newPath);

					Manager->RemoveAssetRep(payloadAssetRep->DirectoryEntry);
					std::filesystem::rename(oldPath, newPath);
					Manager->CreateAssetRep(newPath);
				}
			}

			if (GUI::IsItemClicked())
			{
				SetCurrentPath(entry.path());
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
			// TODO.NW: This is mostly a duplicate of GUI::RenderAssetCard. The logic is slightly different though so will generalize this at a different time
			SVector2<F32> cardStartPos = GUI::GetCursorPos();
			SVector2<F32> framePadding = GUI::GetStyleVar(EStyleVar::FramePadding);

			SVector2<F32> cardSize = { GUI::ThumbnailSizeX + framePadding.X * 0.5f, GUI::ThumbnailSizeY + framePadding.Y * 0.5f };
			cardSize.Y *= 1.6f;
			SVector2<F32> thumbnailSize = { GUI::ThumbnailSizeX + framePadding.X * 0.5f, GUI::ThumbnailSizeY + framePadding.Y * 0.5f + 4.0f };

			std::optional<std::filesystem::directory_entry> selectedFolder = Manager->GetSelectedFolder();
			SColor borderColor = SColor(10);
			if (selectedFolder.has_value() && entry == selectedFolder.value())
				borderColor = GUI::GetStyleColor(EStyleColor::HeaderHovered);

			// TODO.NW: Can't seem to get the leftmost line to show correctly. Maybe need to start the table as usual and then offset inwards?
			constexpr F32 borderThickness = 1.0f;
			GUI::SetCursorPos(cardStartPos + SVector2<F32>(-1.0f * borderThickness));
			GUI::AddRectFilled(GUI::GetCursorScreenPos(), cardSize + SVector2<F32>(2.0f * borderThickness), borderColor);
			GUI::SetCursorPos(cardStartPos);
			GUI::AddRectFilled(GUI::GetCursorScreenPos(), cardSize, SColor(65));
			GUI::SetCursorPos(cardStartPos);
			GUI::AddRectFilled(GUI::GetCursorScreenPos(), thumbnailSize, SColor(40));
			GUI::SetCursorPos(cardStartPos);

			if (GUI::Selectable("", false, {ESelectableFlag::AllowDoubleClick, ESelectableFlag::AllowOverlap}, cardSize))
			{
				if (GUI::IsDoubleClick())
				{
					SetCurrentPath(entry.path());
					Manager->SetSelectedFolder({});
				}
				else
				{
					Manager->SetSelectedFolder(entry);
				}
			}
			if (GUI::IsItemHovered())
			{
				HoveredFolder = entry;
			}

			GUI::SetCursorPos(cardStartPos + SVector2<F32>(1.0f, 0.0f));
			GUI::Image(folderIconID, { GUI::ThumbnailSizeX, GUI::ThumbnailSizeY }, SVector2<F32>(0.0f), SVector2<F32>(1.0f), SColor::White);

			SColor detailColor = SColor::White;
			detailColor.A = SColor::ToU8Range(0.5f);
			GUI::AddRectFilled(GUI::GetCursorScreenPos(), SVector2<F32>(cardSize.X, 2.0f), detailColor);

			GUI::OffsetCursorPos(SVector2<F32>(2.0f, 4.0f));

			auto relativePath = std::filesystem::relative(entry.path());
			std::string filenameString = relativePath.filename().string();

			GUI::PushClipRect(GUI::GetCursorScreenPos(), cardSize - framePadding);
			if (FolderBeingRenamed.has_value() && FolderBeingRenamed.value() == entry)
			{
				GUI::SetKeyboardFocusHere();

				std::optional<std::string> result;
				std::string newAssetName = filenameString;
				GUI::PushID(filenameString.c_str());
				if (GUI::InputText("", newAssetName))
				{
					if (GUI::IsItemDeactivatedAfterEdit())
						result = newAssetName;
				}

				if (GUI::IsItemDeactivated() && !result.has_value())
					result = filenameString;

				GUI::PopID();

				if (result.has_value())
				{
					FolderBeingRenamed.reset();

					std::string oldPath = UGeneralUtils::ConvertToPlatformAgnosticPath(entry.path().string().c_str());
					std::string newPath = UGeneralUtils::ConvertToPlatformAgnosticPath(CurrentDirectory.string()) + "/" + result.value();

					std::filesystem::rename(oldPath, newPath);
				}
			}
			else
			{
				GUI::Text(filenameString.c_str());
			}

			GUI::PopClipRect();
			if (GUI::IsItemHovered())
				GUI::SetTooltip(filenameString.c_str());
		}
		else
		{
			const auto& rep = Manager->GetAssetRepFromDirEntry(entry);
			SEditorAssetRepresentation* selectedAsset = Manager->GetSelectedAsset();
			
			SColor borderColor = SColor(10);
			if (rep.get() == selectedAsset)
				borderColor = GUI::GetStyleColor(EStyleColor::HeaderHovered);
			if (rep->IsSourceWatched)
				borderColor = SColor::Magenta;
			
			SRenderAssetCardResult result = RenderAssetCard(rep.get(), borderColor);

			if (result.IsClicked)
			{
				Manager->SetSelectedAsset(rep.get());
				selectedAsset = rep.get();
			}

			if (result.IsDoubleClicked)
			{
				// NW: Open Tool depending on asset type?
				HV_LOG_INFO("Clicked asset: %s", rep->Name.c_str());
				Manager->OpenAssetTool(rep.get());
				Manager->ClearSelectedAssets();
			}
			
			if (result.IsHovered)
			{
				if (rep->AssetType == EAssetType::SkeletalAnimation)
				{
					AnimatingThumbnailAsset = rep.get();
				}

				HoveredAsset = rep.get();
			}

			if (result.NewAssetName.has_value())
			{
				rep->IsBeingNamed = false;

				if (result.NewAssetName.value() != rep->Name)
				{
					// TODO.NW: This is basically the same as moving an asset from one directory to the other. Figure out how to unify this?

					std::string oldPath = UGeneralUtils::ConvertToPlatformAgnosticPath(rep->DirectoryEntry.path().string().c_str());
					std::string newPath = UGeneralUtils::ConvertToPlatformAgnosticPath(CurrentDirectory.string()) + "/" + result.NewAssetName.value() + ".hva";

					CJsonDocument config = UFileSystem::OpenJson(UFileSystem::EngineConfig);
					config.WriteValueToArray("Asset Redirectors", oldPath, newPath);

					Manager->RemoveAssetRep(rep->DirectoryEntry);
					std::filesystem::rename(oldPath, newPath);
					Manager->CreateAssetRep(newPath);
				}
			}
		}

		GUI::PopID();
	}
}
