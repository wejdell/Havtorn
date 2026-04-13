// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditorManager.h"

#include <WindowsInclude.h>
#include <psapi.h>
#include <format>

#include <Engine.h>
#include <FileSystem.h>
#include <Input/InputMapper.h>
#include <Input/InputTypes.h>

#include <Graphics/RenderManager.h>
#include <ECS/ECSInclude.h>
#include <ECS/ComponentAlgo.h>

#include "EditorResourceManager.h"
#include "EditorWindows.h"
#include "EditorToggleable.h"
#include "EditorToggleables.h"

#include "EditActions/RemoveEntityEditAction.h"

#include "Systems/EditorRenderSystem.h"
#include "Systems/PickingSystem.h"
#include <../Game/GameScene.h>
#include <../Game/GameScript.h>
#include <MathTypes/MathUtilities.h>
#include <PlatformManager.h>
#include <Color.h>
#include <Timer.h>
#include <Assets/AssetRegistry.h>
#include <GeneralUtilities.h>

namespace Havtorn
{
	std::string CEditorManager::PreviewMaterial = "Resources/M_MeshPreview.hva";

	CEditorManager::CEditorManager()
		: EditHistory(CEditHistory(this))
		, DeepLinkParser(CEditorDeepLinkParser(this))
	{
		CInputMapper* mapper = GEngine::GetInput();
		mapper->GetActionDelegate(EInputActionEvent::TranslateTransform).AddMember(this, &CEditorManager::OnInputSetTransformGizmo);
		mapper->GetActionDelegate(EInputActionEvent::RotateTransform).AddMember(this, &CEditorManager::OnInputSetTransformGizmo);
		mapper->GetActionDelegate(EInputActionEvent::ScaleTransform).AddMember(this, &CEditorManager::OnInputSetTransformGizmo);
		mapper->GetActionDelegate(EInputActionEvent::ToggleFreeCam).AddMember(this, &CEditorManager::OnInputToggleFreeCam);
		mapper->GetActionDelegate(EInputActionEvent::FocusEditorEntity).AddMember(this, &CEditorManager::OnInputFocusSelection);
		mapper->GetActionDelegate(EInputActionEvent::DeleteEvent).AddMember(this, &CEditorManager::OnDeleteEvent);
		mapper->GetActionDelegate(EInputActionEvent::ToggleFullscreen).AddMember(this, &CEditorManager::OnToggleFullscreen);
		mapper->GetActionDelegate(EInputActionEvent::StartPlay).AddMember(this, &CEditorManager::OnPlayStateEvent);
		mapper->GetActionDelegate(EInputActionEvent::StopPlay).AddMember(this, &CEditorManager::OnPlayStateEvent);
		mapper->GetActionDelegate(EInputActionEvent::AltPress).AddMember(this, &CEditorManager::OnDragCopyEvent);
		mapper->GetActionDelegate(EInputActionEvent::AltRelease).AddMember(this, &CEditorManager::OnDragCopyEvent);
		mapper->GetActionDelegate(EInputActionEvent::Copy).AddMember(this, &CEditorManager::OnCopyEvent);
		mapper->GetActionDelegate(EInputActionEvent::Paste).AddMember(this, &CEditorManager::OnCopyEvent);
		mapper->GetActionDelegate(EInputActionEvent::Undo).AddMember(this, &CEditorManager::OnEditorActionTreeEvent);
		mapper->GetActionDelegate(EInputActionEvent::Redo).AddMember(this, &CEditorManager::OnEditorActionTreeEvent);
		mapper->GetActionDelegate(EInputActionEvent::MovePivot).AddMember(this, &CEditorManager::OnPivotMoving);
		mapper->GetActionDelegate(EInputActionEvent::VertexSnapping).AddMember(this, &CEditorManager::OnVertexSnapping);
		mapper->GetActionDelegate(EInputActionEvent::GridSnapping).AddMember(this, &CEditorManager::OnGridSnapping);
	}

	CEditorManager::~CEditorManager()
	{
		RenderManager = nullptr;
		SAFE_DELETE(ResourceManager);
	}

	bool CEditorManager::Init(CPlatformManager* platformManager, CRenderManager* renderManager)
	{
		PlatformManager = platformManager;
		if (PlatformManager == nullptr)
			return false;

		SetEditorTheme(EEditorColorTheme::HavtornYellow, EEditorStyleTheme::Havtorn);

		// TODO.NR: Figure out why we can't use unique ptrs with these namespaced imgui classes
		MenuElements.emplace_back(std::make_unique<CFileMenu>("File", this));
		MenuElements.emplace_back(std::make_unique<CEditMenu>("Edit", this));
		MenuElements.emplace_back(std::make_unique<CViewMenu>("View", this));
		MenuElements.emplace_back(std::make_unique<CWindowMenu>("Window", this));
		MenuElements.emplace_back(std::make_unique<CHelpMenu>("Help", this));

		Windows.emplace_back(std::make_unique<CViewportWindow>("Viewport", this));
		Windows.emplace_back(std::make_unique<CDockSpaceWindow>("Dock Space", this));
		Windows.emplace_back(std::make_unique<CAssetBrowserWindow>("Asset Browser", this));
		Windows.emplace_back(std::make_unique<COutputLogWindow>("Output Log", this));
		Windows.emplace_back(std::make_unique<CHierarchyWindow>("Hierarchy", this));
		Windows.emplace_back(std::make_unique<CInspectorWindow>("Inspector", this));

		Windows.emplace_back(std::make_unique<CSpriteAnimatorGraphNodeWindow>("Sprite Animator", this));
		Windows.emplace_back(std::make_unique<CMaterialTool>("Material Editor", this));
		Windows.emplace_back(std::make_unique<CScriptTool>("Script Editor", this));
		Windows.emplace_back(std::make_unique<CInputTool>("Input Editor", this));
		Windows.emplace_back(std::make_unique<CPrefabTool>("Prefab Editor", this));

		ResourceManager = new CEditorResourceManager();
		bool success = ResourceManager->Init(this, renderManager);
		if (!success)
			return false;
		RenderManager = renderManager;

		PlatformManager->OnResolutionChanged.AddMember(this, &CEditorManager::OnResolutionChanged);
		World = GEngine::GetWorld();
		World->OnBeginPlayDelegate.AddMember(this, &CEditorManager::OnBeginPlay);
		World->OnPausePlayDelegate.AddMember(this, &CEditorManager::OnPausePlay);
		World->OnEndPlayDelegate.AddMember(this, &CEditorManager::OnEndPlay);
		World->RequestSystem<CEditorRenderSystem>(this, RenderManager, World, this);
		World->RequestSystem<CPickingSystem>(this, this);

		InitEditorLayout();
		InitAssetRepresentations();
		InitEditorPreferences();

		return success;
	}

	void CEditorManager::BeginFrame()
	{
	}

	void CEditorManager::Render()
	{
		if (IsFullscreen)
			return;

		bool isHoveringMenuBarButton = false;

		// Main Menu bar
		if (IsEnabled)
		{
			GUI::BeginMainMenuBar();

			GUI::TextDisabled("ICON");
			for (const auto& element : MenuElements)
			{
				element->OnInspectorGUI();
				isHoveringMenuBarButton = GUI::IsMouseInRect(GUI::GetLastRect()) ? true : isHoveringMenuBarButton;
			}

			const std::string projectName = "Project Name";
			const std::string windowTitle = "Havtorn Editor | " + projectName + " | " + HAVTORN_VERSION;
			GUI::CenterText(windowTitle, GUI::GetCurrentWindowSize());
			
			const F32 windowWidth = GUI::GetCurrentWindowSize().X;
			GUI::SetCursorPosX(windowWidth - 92.0f);

			// TODO.NW: Derive this from style params
			constexpr F32 menuElementHeight = 16.0f;
			if (GUI::ImageButton("MinimizeButton", ResourceManager->GetStaticEditorTextureResource(EEditorTexture::MinimizeWindow), SVector2<F32>(menuElementHeight)))
			{
				PlatformManager->MinimizeWindow();
			}
			isHoveringMenuBarButton = GUI::IsMouseInRect(GUI::GetLastRect()) ? true : isHoveringMenuBarButton;

			if (GUI::ImageButton("MaximizeButton", ResourceManager->GetStaticEditorTextureResource(EEditorTexture::MaximizeWindow), SVector2<F32>(menuElementHeight)))
			{
				PlatformManager->MaximizeWindow();
			}
			isHoveringMenuBarButton = GUI::IsMouseInRect(GUI::GetLastRect()) ? true : isHoveringMenuBarButton;

			if (GUI::ImageButton("CloseWindowButton", ResourceManager->GetStaticEditorTextureResource(EEditorTexture::CloseWindow), SVector2<F32>(menuElementHeight)))
			{
				PlatformManager->CloseWindow();
			}
			isHoveringMenuBarButton = GUI::IsMouseInRect(GUI::GetLastRect()) ? true : isHoveringMenuBarButton;

			GUI::EndMainMenuBar();
		}

		PlatformManager->SetBlockWindowHitTest(isHoveringMenuBarButton);

		// Windows
		for (const auto& window : Windows)
		{
			if (window->GetEnabled())
				window->OnInspectorGUI();

			window->UpdateState();
		}

		ReinitEditorLayout();

		DebugWindow();
	}

	void CEditorManager::EndFrame()
	{
	}

	void CEditorManager::DebugWindow()
	{
		if (IsDemoOpen)
		{
			GUI::ShowDemoWindow(&IsDemoOpen);
		}

		if (IsDebugInfoOpen)
		{
			if (GUI::Begin("Debug info", &IsDebugInfoOpen))
			{
				GUI::Text(GetFrameRate().c_str());
				GUI::Text(GetSystemMemory().c_str());
				GUI::Text(GetRenderInfo().c_str());

				static bool debugRegistry = false;
				GUI::Checkbox("Show Registry Details", debugRegistry);
				GUI::Text(GEngine::GetAssetRegistry()->GetDebugString(debugRegistry).c_str());
			}

			GUI::End();
		}
		
		if (IsPreferencesOpen)
		{
			if (GUI::Begin("Editor Preferences", &IsPreferencesOpen))
			{
				F32 sensitivity = GetEditorSensitivity();
				if (GUI::DragFloat("Editor Sensitivity", sensitivity, 0.01f, 0.01f, 5.0f))
				{
					SetEditorSensitivity(sensitivity);
				}
				
				auto ColorSelectTreeNode = [&](EWorldPlayState dedicatedPlayState)
				{
					const F32 sz = GUI::GetTextLineHeight();
					for (U16 i = 0; i < static_cast<U16>(EEditorColorTheme::Count); i++)
					{
						auto colorTheme = static_cast<EEditorColorTheme>(i);
						std::string name = GetEditorColorThemeName(colorTheme);
						SVector2<F32> cursorPos = GUI::GetCursorScreenPos();
						SColor previewColor = GetEditorColorThemeRepColor(colorTheme);
						GUI::AddRectFilled(cursorPos, SVector2<F32>(sz), previewColor);
						GUI::Dummy({ sz, sz });
						GUI::SameLine();
						if (GUI::MenuItem(name.c_str()))
						{
							SetEditorModeColorTheme(dedicatedPlayState, colorTheme);	
						}
					}	
				};
				
				GUI::Text("Color Theme");
				if (GUI::TreeNode("Editor Color Themes"))
				{
					ColorSelectTreeNode(EWorldPlayState::Stopped);
					GUI::TreePop();
				}
				
				if (GUI::TreeNode("Play Color Themes"))
				{
					ColorSelectTreeNode(EWorldPlayState::Playing);
					GUI::TreePop();
				}
				
				if (GUI::TreeNode("Pause Color Themes"))
				{
					ColorSelectTreeNode(EWorldPlayState::Paused);
					GUI::TreePop();
				}
			}
			
			GUI::End();
		}

		if (IsGamePreferencesOpen)
		{
			if (GUI::Begin("Game Preferences", &IsGamePreferencesOpen))
			{
				SPostProcessingBufferData bufferData = RenderManager->GetPostProcessingBufferData();
				bool dataChanged = false;
				
				if (GUI::ColorPicker4("White Point Color", bufferData.WhitePointColor))
					dataChanged = true;

				if (GUI::DragFloat("White Point Intensity", bufferData.WhitePointIntensity))
					dataChanged = true;
				
				if (GUI::DragFloat("Exposure", bufferData.Exposure))
					dataChanged = true;

				if (GUI::Checkbox("Reinhard", bufferData.IsReinhard))
				{
					dataChanged = true;
					bufferData.IsUncharted = false;
					bufferData.IsACES = false;
					bufferData.IsAgX = false;
				}

				if (GUI::Checkbox("Uncharted", bufferData.IsUncharted))
				{
					dataChanged = true;
					bufferData.IsReinhard = false;
					bufferData.IsACES = false;
					bufferData.IsAgX = false;
				}

				if (GUI::Checkbox("ACES", bufferData.IsACES))
				{
					dataChanged = true;
					bufferData.IsReinhard = false;
					bufferData.IsUncharted = false;
					bufferData.IsAgX = false;
				}

				if (GUI::Checkbox("AgX", bufferData.IsAgX))
				{
					dataChanged = true;
					bufferData.IsReinhard = false;
					bufferData.IsUncharted = false;
					bufferData.IsACES = false;
				}

				if (!bufferData.IsAgX && !bufferData.IsUncharted && !bufferData.IsACES && !bufferData.IsReinhard)
					bufferData.IsUncharted = true;

				if (GUI::DragFloat("SSAO Radius", bufferData.SSAORadius))
					dataChanged = true;

				if (GUI::DragFloat("SSAO Sample Bias", bufferData.SSAOSampleBias))
					dataChanged = true;

				if (GUI::DragFloat("SSAO Magnitude", bufferData.SSAOMagnitude))
					dataChanged = true;

				if (GUI::DragFloat("SSAO Contrast", bufferData.SSAOContrast))
					dataChanged = true;

				if (GUI::DragFloat("Emissive Strength", bufferData.EmissiveStrength))
					dataChanged = true;

				if (GUI::DragFloat("Vignette Strength", bufferData.VignetteStrength))
					dataChanged = true;

				if (GUI::ColorPicker4("Vignette Color", bufferData.VignetteColor))
					dataChanged = true;

				GUI::Separator();
				GUI::Text("AgX Settings");

				if (GUI::DragFloat("Middle Gray", bufferData.AgXMiddleGray, 0.01f, 0.0f, 10.0f))
					dataChanged = true;

				if (GUI::DragFloat("Slope", bufferData.AgXSlope, 0.01f, -10.0f, 10.0f))
					dataChanged = true;

				if (GUI::DragFloat("Toe Power", bufferData.AgXToePower, 0.01f, -10.0f, 10.0f))
					dataChanged = true;

				if (GUI::DragFloat("Shoulder Power", bufferData.AgXShoulderPower, 0.01f, -10.0f, 10.0f))
					dataChanged = true;

				if (GUI::DragFloat("Compression R", bufferData.AgXCompressionR, 0.01f, 0.0f, 1.0f))
					dataChanged = true;

				if (GUI::DragFloat("Compression G", bufferData.AgXCompressionG, 0.01f, 0.0f, 1.0f))
					dataChanged = true;

				if (GUI::DragFloat("Compression B", bufferData.AgXCompressionB, 0.01f, 0.0f, 1.0f))
					dataChanged = true;

				if (GUI::DragFloat("Saturation", bufferData.AgXSaturation, 0.01f))
					dataChanged = true;

				if (GUI::DragFloat("Lerp", bufferData.AgXLerp, 0.01f, 0.0f, 1.0f))
					dataChanged = true;


				if (dataChanged)
					RenderManager->SetPostProcessingBufferData(bufferData);
			}

			GUI::End();
		}

		if (IsEditHistoryOpen)
		{
			if (GUI::Begin("Edit History", &IsEditHistoryOpen))
			{
				EditHistory.Render();
			}

			GUI::End();
		}
	}

	void CEditorManager::SetCurrentWorkingScene(const I64 sceneIndex)
	{
		if (sceneIndex < 0)
		{
			CurrentWorkingScene = nullptr;
			SelectedEntities.clear();
			World->SetMainCamera(SEntity::Null);
			return;
		}

		std::vector<Ptr<CScene>>& scenes = World->GetActiveScenes();
		if (scenes.empty())
		{
			CurrentWorkingScene = nullptr;
			SelectedEntities.clear();
			World->SetMainCamera(SEntity::Null);
			return;
		}

		I64 activeSceneIndex = UMath::Clamp(sceneIndex, STATIC_I64(0), STATIC_I64(scenes.size()) - 1);

		CurrentWorkingScene = scenes[activeSceneIndex].get();
		
		std::vector<SCameraComponent*> cameras = CurrentWorkingScene->GetComponents<SCameraComponent>();
		for (SCameraComponent* camera : cameras)
		{
			if (camera->IsActive)
			{
				World->SetMainCamera(camera->Owner);
				SetEditorSensitivity(GetEditorSensitivity());
				break;
			}
		}
		SelectedEntities.clear();
	}

	CScene* CEditorManager::GetCurrentWorkingScene() const
	{
		return CurrentWorkingScene;
	}

	std::vector<Ptr<CScene>>& CEditorManager::GetScenes() const
	{
		return World->GetActiveScenes();
	}

	CScene* CEditorManager::GetContainingScene(const SEntity& entity) const
	{
		CScene* containingScene = UComponentAlgo::GetContainingScene(entity, World->GetActiveScenes());

		if (containingScene == nullptr)
		{
			CPrefabTool* prefabTool = GetEditorWindow<CPrefabTool>();
			if (prefabTool != nullptr && prefabTool->GetEnabled())
			{
				CScene* prefabScene = prefabTool->GetWorkingScene();
				if (prefabScene->HasEntity(entity.GUID))
					return prefabScene;
			}
		}

		return containingScene;
	}

	void CEditorManager::SetSelectedEntity(const SEntity& entity)
	{
		ClearSelectedEntities();
		AddSelectedEntity(entity);
	}

	void CEditorManager::AddSelectedEntity(const SEntity& entity)
	{
		SEntity prefabParent = GetPackedPrefabParent(entity);
		if (prefabParent.IsValid())
			SelectedEntities.emplace_back(prefabParent);
		else
			SelectedEntities.emplace_back(entity);
	}

	void CEditorManager::RemoveSelectedEntity(const SEntity& entity)
	{
		if (auto it = std::ranges::find(SelectedEntities, entity); it != SelectedEntities.end())
			SelectedEntities.erase(it);
	}

	bool CEditorManager::IsEntitySelected(const SEntity& entity)
	{
		auto it = std::ranges::find(SelectedEntities, entity);
		return it != SelectedEntities.end();
	}

	void CEditorManager::ClearSelectedEntities()
	{
		SelectedEntities.clear();
	}

	const SEntity& CEditorManager::GetSelectedEntity() const
	{
		return SelectedEntities.empty() ? SEntity::Null : SelectedEntities[0];
	}

	const SEntity& CEditorManager::GetLastSelectedEntity() const
	{
		return SelectedEntities.empty() ? SEntity::Null : SelectedEntities.back();
	}

	std::vector<SEntity> CEditorManager::GetSelectedEntities() const
	{
		return SelectedEntities;
	}

	bool CEditorManager::IsEntityInsidePackedPrefab(const SEntity& entity) const
	{
		return GetPackedPrefabParent(entity).IsValid();
	}

	SEntity CEditorManager::GetPackedPrefabParent(const SEntity& entity) const
	{
		SEntity currentEntity = entity;
		CScene* containingScene = GetContainingScene(currentEntity);

		if (containingScene == nullptr)
			return SEntity::Null;

		while (currentEntity.IsValid())
		{
			STransformComponent* transform = containingScene->GetComponent<STransformComponent>(currentEntity);
			if (!SComponent::IsValid(transform))
				return SEntity::Null;

			SEntity parentEntity = transform->ParentEntity;
			if (!parentEntity.IsValid())
				return SEntity::Null;

			SPrefabComponent* parentPrefabComponent = containingScene->GetComponent<SPrefabComponent>(parentEntity);
			if (SComponent::IsValid(parentPrefabComponent) && parentPrefabComponent->PrefabMode == EPrefabMode::Packed)
				return parentEntity;

			currentEntity = parentEntity;
		};

		return SEntity::Null;
	}

	std::string CEditorManager::GetEntityFocusLink(const SEntity& entity) const
	{
		return DeepLinkParser.GetEntityFocusLink(entity);
	}

	std::string CEditorManager::GetCameraFocusLink() const
	{
		return DeepLinkParser.GetCameraFocusLink();
	}

	std::string CEditorManager::GetAssetFocusLink(SEditorAssetRepresentation* assetRep) const
	{
		return DeepLinkParser.GetAssetFocusLink(assetRep);
	}

	void CEditorManager::SetSelectedAsset(SEditorAssetRepresentation* asset)
	{
		ClearSelectedAssets();
		AddSelectedAsset(asset);
	}

	void CEditorManager::AddSelectedAsset(SEditorAssetRepresentation* asset)
	{
		SelectedAssets.emplace_back(asset);
	}

	void CEditorManager::RemoveSelectedAsset(SEditorAssetRepresentation* asset)
	{
		if (auto it = std::ranges::find(SelectedAssets, asset); it != SelectedAssets.end())
			SelectedAssets.erase(it);
	}

	bool CEditorManager::IsAssetSelected(SEditorAssetRepresentation* asset) const
	{
		auto it = std::ranges::find(SelectedAssets, asset);
		return it != SelectedAssets.end();
	}

	void CEditorManager::ClearSelectedAssets()
	{
		SelectedAssets.clear();
	}

	SEditorAssetRepresentation* CEditorManager::GetSelectedAsset() const
	{
		return SelectedAssets.empty() ? nullptr : SelectedAssets[0];
	}

	SEditorAssetRepresentation* CEditorManager::GetLastSelectedAsset() const
	{
		return SelectedAssets.empty() ? nullptr : SelectedAssets.back();
	}

	std::vector<SEditorAssetRepresentation*> CEditorManager::GetSelectedAssets() const
	{
		return SelectedAssets;
	}

	const Ptr<SEditorAssetRepresentation>& CEditorManager::GetAssetRepFromDirEntry(const std::filesystem::directory_entry& dirEntry) const
	{
		for (const auto& rep : AssetRepresentations)
		{
			if (dirEntry == rep->DirectoryEntry)
				return rep;
		}

		// NR: Return empty rep
		return AssetRepresentations[0];
	}

	const Ptr<SEditorAssetRepresentation>& CEditorManager::GetAssetRepFromName(const std::string& assetName) const
	{
		for (const auto& rep : AssetRepresentations)
		{
			if (assetName == rep->Name)
				return rep;
		}

		// NR: Return empty rep
		return AssetRepresentations[0];
	}

	const intptr_t CEditorManager::GetTextureResourceFromAssetRep(SEditorAssetRepresentation* assetRepresentation) const
	{
		intptr_t repRenderTexture = 0;
		if (assetRepresentation == nullptr)
			return repRenderTexture;

		switch (assetRepresentation->AssetType)
		{
		case EAssetType::StaticMesh:
		case EAssetType::SkeletalMesh:
		case EAssetType::Material:
		case EAssetType::Animation:
		case EAssetType::Texture:
		case EAssetType::TextureCube:
		{
			CRenderTexture* renderTexture = &assetRepresentation->TextureRef;

			if (renderTexture->IsShaderResourceValid())
				repRenderTexture = (intptr_t)renderTexture->GetShaderResourceView();
			else
				repRenderTexture = ResourceManager->GetStaticEditorTextureResource(EEditorTexture::FileIcon);
		}
		break;
		case EAssetType::Scene:
			repRenderTexture = ResourceManager->GetStaticEditorTextureResource(EEditorTexture::SceneIcon);
			break;
		case EAssetType::Sequencer:
			repRenderTexture = ResourceManager->GetStaticEditorTextureResource(EEditorTexture::SequencerIcon);
			break;
		case EAssetType::Script:
			repRenderTexture = ResourceManager->GetStaticEditorTextureResource(EEditorTexture::ScriptIcon);
			break;
		case EAssetType::Prefab:
			repRenderTexture = ResourceManager->GetStaticEditorTextureResource(EEditorTexture::PrefabIcon);
			break;
		default:
			break;
		}

		return repRenderTexture;
	}

	DirEntryFunc CEditorManager::GetAssetInspectFunction() const
	{
		return [this](std::filesystem::directory_entry entry)
			{
				const Ptr<SEditorAssetRepresentation>& assetRep = GetAssetRepFromDirEntry(entry);	
				return SAssetInspectionData(assetRep->Name, GetTextureResourceFromAssetRep(assetRep.get()), assetRep->DirectoryEntry.path().string());
			};
	}

	DirEntryEAssetTypeFunc CEditorManager::GetAssetFilteredInspectFunction() const
	{
		return [this](std::filesystem::directory_entry entry, const EAssetType assetTypeFilter)
			{
				const Ptr<SEditorAssetRepresentation>& assetRep = GetAssetRepFromDirEntry(entry);
				if (assetRep->AssetType == assetTypeFilter)
					return SAssetInspectionData(assetRep->Name, GetTextureResourceFromAssetRep(assetRep.get()), assetRep->DirectoryEntry.path().string());

				return SAssetInspectionData("", 0, "");
			};
	}

	void CEditorManager::CreateAssetRep(const std::filesystem::path& path)
	{
		if (!UFileSystem::Exists(path.string()))
		{
			HV_LOG_ERROR("CEditorManager::CreateAssetRep failed to create an asset representation! File was not found!");
			return;
		}

		ClearSelectedAssets();

		std::filesystem::directory_entry entry(path);
		HV_ASSERT(!entry.is_directory(), "You are trying to create SEditorAssetRepresentation but you're creating a new folder.");

		std::string filePath = path.string();
		const U64 fileSize = UMath::Max(UFileSystem::GetFileSize(filePath), sizeof(EAssetType));
		char* data = new char[fileSize];

		UFileSystem::Deserialize(filePath, data, STATIC_U32(fileSize));

		SEditorAssetRepresentation rep;

		const U32 size = sizeof(EAssetType);
		memcpy(&rep.AssetType, &data[0], size);

		rep.DirectoryEntry = entry;
		rep.Name = entry.path().filename().string();
		rep.Name = rep.Name.substr(0, rep.Name.length() - 4);

		switch (rep.AssetType)
		{
		case EAssetType::StaticMesh:
		case EAssetType::SkeletalMesh:
		case EAssetType::Material:
		case EAssetType::Animation:
		case EAssetType::Texture:
		case EAssetType::TextureCube:
			rep.UsingEditorTexture = false;
			ResourceManager->RequestThumbnailRender(&rep, filePath);
			break;
		default:
			rep.UsingEditorTexture = true;
			break;
		}

		AssetRepresentations.emplace_back(std::make_unique<SEditorAssetRepresentation>(rep));

		delete[] data;
	}

	void CEditorManager::RemoveAssetRep(const std::filesystem::directory_entry& sourceEntry)
	{
		SelectedAssets.clear();

		auto& rep = GetAssetRepFromDirEntry(sourceEntry);
		if (rep == AssetRepresentations[0])
			return;

		auto it = std::ranges::find(AssetRepresentations, rep);
		if (it != AssetRepresentations.end())
		{
			if (!it->get()->UsingEditorTexture)
				RenderManager->UnrequestRenderView(SAssetReference(it->get()->Name).UID);

			AssetRepresentations.erase(it);	
		}
	}

	void CEditorManager::OpenAssetTool(SEditorAssetRepresentation* asset)
	{
		if (asset->AssetType == EAssetType::Material)
		{
			GetEditorWindow<CMaterialTool>()->OpenMaterial(asset);
		}

		if (asset->AssetType == EAssetType::Script)
		{
			GetEditorWindow<CScriptTool>()->OpenScript(asset);
		}

		if (asset->AssetType == EAssetType::Scene)
		{
			GEngine::GetWorld()->ChangeScene<CGameScene>(asset->DirectoryEntry.path().string());
			SetCurrentWorkingScene(0);
		}

		if (asset->AssetType == EAssetType::InputAsset)
		{
			GetEditorWindow<CInputTool>()->OpenInputAsset(asset);
		}

		if (asset->AssetType == EAssetType::Prefab)
		{
			GetEditorWindow<CPrefabTool>()->OpenPrefab(asset);
		}
	}

	void CEditorManager::FocusEntity(const SEntity& entity)
	{
		CScene* currentScene = GetContainingScene(entity);
		if (currentScene == nullptr)
			return;

		CWorld* world = GEngine::GetWorld();
		const SEntity& mainCamera = world->GetMainCamera();
		SCameraData mainCameraData = UComponentAlgo::GetCameraData(mainCamera, world->GetActiveScenes());

		if (!mainCameraData.IsValid())
		{
			HV_LOG_WARN("OnInputFocusSelection: could not find a camera component to move. Cannot focus current selection.");
			return;
		}

		SVector worldPos = SVector::Zero;
		SVector center = SVector::Zero;
		SVector bounds = SVector::Zero;
		SVector2<F32> fov = SVector2<F32>::Zero;
		bool foundBounds = false;

		if (STransformComponent* transform = currentScene->GetComponent<STransformComponent>(entity))
		{
			worldPos = transform->Transform.GetMatrix().GetTranslation();
		}
		else
		{
			HV_LOG_WARN("OnInputFocusSelection: could not find a transform to go to. Cannot focus current selection.");
			return;
		}

		fov = { mainCameraData.CameraComponent->AspectRatio * mainCameraData.CameraComponent->FOV, mainCameraData.CameraComponent->FOV };

		if (SStaticMeshComponent* staticMesh = currentScene->GetComponent<SStaticMeshComponent>(entity))
		{
			SStaticMeshAsset* meshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SStaticMeshAsset>(staticMesh->AssetReference, CAssetRegistry::EditorManagerRequestID);
			center = meshAsset->BoundsCenter;
			bounds = SVector::GetAbsMaxKeepValue(meshAsset->BoundsMax, meshAsset->BoundsMin);
			GEngine::GetAssetRegistry()->UnrequestAsset(staticMesh->AssetReference, CAssetRegistry::EditorManagerRequestID);
			foundBounds = true;
		}
		else if (SSkeletalMeshComponent* skeletalMesh = currentScene->GetComponent<SSkeletalMeshComponent>(entity))
		{
			SSkeletalMeshAsset* meshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalMeshAsset>(skeletalMesh->AssetReference, CAssetRegistry::EditorManagerRequestID);
			center = meshAsset->BoundsCenter;
			bounds = SVector::GetAbsMaxKeepValue(meshAsset->BoundsMax, meshAsset->BoundsMin);
			GEngine::GetAssetRegistry()->UnrequestAsset(skeletalMesh->AssetReference, CAssetRegistry::EditorManagerRequestID);
			foundBounds = true;
		}

		if (!foundBounds)
		{
			HV_LOG_WARN("OnInputFocusSelection: could not find component to derive bounds from. Cannot focus current selection.");
			return;
		}

		constexpr F32 focusMarginPercentage = 1.1f;

		const SVector worldSpaceFocusPoint = worldPos + center;
		const SVector cameraLocation = mainCameraData.TransformComponent->Transform.GetMatrix().GetTranslation();
		const SVector targetToCamera = (cameraLocation - worldSpaceFocusPoint).GetNormalized();
		SMatrix newMatrix = SMatrix::LookAtLH(cameraLocation, worldSpaceFocusPoint, SVector::Up).FastInverse();
		newMatrix.SetTranslation(worldSpaceFocusPoint + targetToCamera * UMathUtilities::GetFocusDistanceForBounds(center, bounds, fov, focusMarginPercentage));

		mainCameraData.TransformComponent->Transform.SetMatrix(newMatrix);

		CScene* mainCameraScene = GetContainingScene(mainCamera);
		if (SCameraControllerComponent* controllerComp = mainCameraScene->GetComponent<SCameraControllerComponent>(mainCamera))
		{
			SVector currentEuler = mainCameraData.TransformComponent->Transform.GetMatrix().GetEuler();
			controllerComp->CurrentPitch = UMath::Clamp(currentEuler.X, -SCameraControllerComponent::MaxPitchDegrees + 0.01f, SCameraControllerComponent::MaxPitchDegrees - 0.01f);;
			controllerComp->CurrentYaw = UMath::WrapAngle(currentEuler.Y);
		}
	}

	void CEditorManager::SetEditorTheme(EEditorColorTheme colorTheme, EEditorStyleTheme styleTheme, F32 darknessOffset)
	{
	    auto applyDarkness = [darknessOffset](F32 r, F32 g, F32 b, F32 a = 1.0f) -> SColor
	    {
	        return SColor(r * darknessOffset, g * darknessOffset, b * darknessOffset, a);
	    };

	    switch (colorTheme)
	    {
		case EEditorColorTheme::HavtornDefault:
		{
			SGuiColorProfile colorProfile(
				applyDarkness(0.11f, 0.11f, 0.11f),
				applyDarkness(0.198f, 0.198f, 0.198f),
				applyDarkness(0.278f, 0.271f, 0.267f),
				applyDarkness(0.478f, 0.361f, 0.188f),
				applyDarkness(0.814f, 0.532f, 0.000f),
				applyDarkness(1.000f, 0.659f, 0.000f)
			);
			GUI::SetGuiColorProfile(colorProfile);
		}
		break;
	    case EEditorColorTheme::HavtornYellow:
	    {
	        SGuiColorProfile colorProfile(
	            applyDarkness(0.11f,  0.11f,  0.11f),
	            applyDarkness(0.198f, 0.198f, 0.198f),
	            applyDarkness(0.278f, 0.271f, 0.267f),
	            applyDarkness(0.694f, 0.573f, 0.129f),
	            applyDarkness(0.918f, 0.722f, 0.055f),
	            applyDarkness(1.00f,  0.855f, 0.165f)
	        );
	        GUI::SetGuiColorProfile(colorProfile);
	    }
	    break;
	    case EEditorColorTheme::HavtornRed:
	    {
	        SGuiColorProfile colorProfile(
	            applyDarkness(0.11f,  0.11f,  0.11f),
	            applyDarkness(0.198f, 0.198f, 0.198f),
	            applyDarkness(0.278f, 0.271f, 0.267f),
	            applyDarkness(0.478f, 0.188f, 0.188f),
	            applyDarkness(0.814f, 0.00f,  0.00f),
	            applyDarkness(1.00f,  0.00f,  0.00f)
	        );
	        GUI::SetGuiColorProfile(colorProfile);
	    }
	    break;
	    case EEditorColorTheme::HavtornGreen:
	    {
	        SGuiColorProfile colorProfile(
	            applyDarkness(0.11f,  0.11f,  0.11f),
	            applyDarkness(0.198f, 0.198f, 0.198f),
	            applyDarkness(0.278f, 0.271f, 0.267f),
	            applyDarkness(0.355f, 0.478f, 0.188f),
	            applyDarkness(0.469f, 0.814f, 0.00f),
	            applyDarkness(0.576f, 1.00f,  0.00f)
	        );
	        GUI::SetGuiColorProfile(colorProfile);
	    }
	    break;
	    case EEditorColorTheme::HavtornDarkBlue:
	    {
	        SGuiColorProfile colorProfile(
	            applyDarkness(0.11f,  0.11f,  0.11f),
	            applyDarkness(0.198f, 0.198f, 0.198f),
	            applyDarkness(0.278f, 0.271f, 0.267f),
	            applyDarkness(0.188f, 0.278f, 0.478f),
	            applyDarkness(0.00f,  0.314f, 0.814f),
	            applyDarkness(0.00f,  0.376f, 1.00f)
	        );
	        GUI::SetGuiColorProfile(colorProfile);
	    }
	    break;
	    case EEditorColorTheme::HavtornLightBlue:
	    {
	        SGuiColorProfile colorProfile(
	            applyDarkness(0.11f,  0.11f,  0.11f),
	            applyDarkness(0.198f, 0.198f, 0.198f),
	            applyDarkness(0.278f, 0.271f, 0.267f),
	            applyDarkness(0.318f, 0.478f, 0.678f),
	            applyDarkness(0.469f, 0.714f, 0.914f),
	            applyDarkness(0.576f, 0.824f, 1.00f)
	        );
	        GUI::SetGuiColorProfile(colorProfile);
	    }
	    break;
	    case EEditorColorTheme::HavtornPurple:
	    {
	        SGuiColorProfile colorProfile(
	            applyDarkness(0.11f,  0.11f,  0.11f),
	            applyDarkness(0.198f, 0.198f, 0.198f),
	            applyDarkness(0.278f, 0.271f, 0.267f),
	            applyDarkness(0.361f, 0.278f, 0.478f),
	            applyDarkness(0.561f, 0.314f, 0.814f),
	            applyDarkness(0.686f, 0.376f, 1.00f)
	        );
	        GUI::SetGuiColorProfile(colorProfile);
	    }
	    break;
	    case EEditorColorTheme::HavtornPink:
	    {
	        SGuiColorProfile colorProfile(
	            applyDarkness(0.11f,  0.11f,  0.11f),
	            applyDarkness(0.198f, 0.198f, 0.198f),
	            applyDarkness(0.278f, 0.271f, 0.267f),
	            applyDarkness(0.478f, 0.278f, 0.361f),
	            applyDarkness(0.814f, 0.314f, 0.561f),
	            applyDarkness(1.00f,  0.376f, 0.686f)
	        );
	        GUI::SetGuiColorProfile(colorProfile);
	    }
	    break;
	    case EEditorColorTheme::Count:
	        break;
	    }

	    switch (styleTheme)
	    {
	    case EEditorStyleTheme::Havtorn:
	        GUI::SetGuiStyleProfile(SGuiStyleProfile());
	        break;
	    case EEditorStyleTheme::Count:
	    case EEditorStyleTheme::Default:
	        break;
	    }
	}

	std::string CEditorManager::GetEditorColorThemeName(const EEditorColorTheme colorTheme)
	{
		switch (colorTheme)
		{
		case EEditorColorTheme::HavtornDefault:
			return "Havtorn Default";
		case EEditorColorTheme::HavtornYellow:
			return "Havtorn Yellow";
		case EEditorColorTheme::HavtornRed:
			return "Havtorn Red";
		case EEditorColorTheme::HavtornGreen:
			return "Havtorn Green";
		case EEditorColorTheme::HavtornDarkBlue:
			return "Havtorn Dark Blue";
		case EEditorColorTheme::HavtornLightBlue:
			return "Havtorn Light Blue";
		case EEditorColorTheme::HavtornPurple:
			return "Havtorn Purple";
		case EEditorColorTheme::HavtornPink:
			return "Havtorn Pink";
		case EEditorColorTheme::Count:
			return {};
		}
		return {};
	}

	SColor CEditorManager::GetEditorColorThemeRepColor(const EEditorColorTheme colorTheme)
	{
		switch (colorTheme)
		{
		case EEditorColorTheme::HavtornDefault:
			return { 0.478f, 0.361f, 0.188f, 1.00f };
		case EEditorColorTheme::HavtornYellow:
			return { 0.694f, 0.573f, 0.129f, 1.00f };
		case EEditorColorTheme::HavtornRed:
			return { 0.478f, 0.188f, 0.188f, 1.00f };
		case EEditorColorTheme::HavtornGreen:
			return { 0.355f, 0.478f, 0.188f, 1.00f };
		case EEditorColorTheme::HavtornDarkBlue:
			return { 0.11f, 0.18f, 0.32f, 1.00f };
		case EEditorColorTheme::HavtornLightBlue:
			return { 0.46f, 0.64f, 0.88f, 1.00f };
		case EEditorColorTheme::HavtornPurple:
			return { 0.48f, 0.36f, 0.60f, 1.00f };
		case EEditorColorTheme::HavtornPink:
			return { 0.78f, 0.36f, 0.52f, 1.00f };
		case EEditorColorTheme::Count:
			return {};
		}
		return {};
	}

	SEditorLayout& CEditorManager::GetEditorLayout()
	{
		return EditorLayout;
	}

	F32 CEditorManager::GetViewportPadding() const
	{
		return ViewportPadding;
	}

	void CEditorManager::SetViewportPadding(const F32 padding)
	{
		ViewportPadding = padding;
		InitEditorLayout();
	}

	F32 CEditorManager::GetEditorSensitivity() const
	{
		return EditorPreferences.Sensitivity;
	}

	void CEditorManager::SetEditorSensitivity(const F32 sensitivity)
	{
		EditorPreferences.Sensitivity = sensitivity;
		EditorPreferencesDocument.Set("Sensitivity", EditorPreferences.Sensitivity);
		
		if (!CurrentWorkingScene) 
			return;
		if (!World || !World->GetMainCamera().IsValid()) 
			return;
		
		SCameraControllerComponent* controllerComp = CurrentWorkingScene->GetComponent<SCameraControllerComponent>(World->GetMainCamera());
		if (!SComponent::IsValid(controllerComp)) 
			return;
		
		controllerComp->RotationSpeed = GetEditorSensitivity();
	}

	void CEditorManager::SetEditorModeColorTheme(const EWorldPlayState dedicatedPlayState, const EEditorColorTheme newColorTheme)
	{
		switch (dedicatedPlayState)
		{
			case EWorldPlayState::Stopped:
				EditorPreferencesDocument.Set(EditorColorThemeKey, STATIC_I32(newColorTheme));
				EditorPreferences.EditorColorTheme = newColorTheme;
				SetEditorTheme(newColorTheme);
				break;
			case EWorldPlayState::Paused:
				EditorPreferencesDocument.Set(PauseColorThemeKey, STATIC_I32(newColorTheme));
				EditorPreferences.PauseColorTheme = newColorTheme;
				break;
			case EWorldPlayState::Playing:
				EditorPreferencesDocument.Set(PlayColorThemeKey, STATIC_I32(newColorTheme));
				EditorPreferences.PlayColorTheme = newColorTheme;
				break;
		}
	}

	bool CEditorManager::GetIsWorldPlaying() const
	{
		return World->GetWorldPlayState() == EWorldPlayState::Playing;
	}

	CRenderManager* CEditorManager::GetRenderManager() const
	{
		return RenderManager;
	}

	const CEditorResourceManager* CEditorManager::GetResourceManager() const
	{
		return ResourceManager;
	}

	CPlatformManager* CEditorManager::GetPlatformManager() const
	{
		return PlatformManager;
	}

	void CEditorManager::ToggleDebugInfo()
	{
		IsDebugInfoOpen = !IsDebugInfoOpen;
	}

	void CEditorManager::ToggleDemo()
	{
		IsDemoOpen = !IsDemoOpen;
	}
	
	void CEditorManager::TogglePreferences()
	{
		IsPreferencesOpen = !IsPreferencesOpen;
	}

	void CEditorManager::ToggleGamePreferences()
	{
		IsGamePreferencesOpen = !IsGamePreferencesOpen;
	}

	void CEditorManager::ToggleEditHistory()
	{
		IsEditHistoryOpen = !IsEditHistoryOpen;
	}

	void CEditorManager::InitEditorLayout()
	{
		EditorLayout = SEditorLayout();

		const SVector2<U16> resolution = PlatformManager->GetResolution();

		constexpr F32 viewportAspectRatioInv = (9.0f / 16.0f);
		const F32 viewportPaddingX = ViewportPadding;

		I16 viewportPosX = STATIC_I16(resolution.X * viewportPaddingX);
		I16 viewportPosY = STATIC_I16(19);
		U16 viewportSizeX = STATIC_U16(resolution.X - (2.0f * STATIC_F32(viewportPosX)));
		U16 viewportSizeY = STATIC_U16(STATIC_F32(viewportSizeX) * viewportAspectRatioInv);

		EditorLayout.HierarchyViewPosition = { 0, viewportPosY };
		EditorLayout.ViewportPosition = { viewportPosX, viewportPosY };
		EditorLayout.InspectorPosition = { STATIC_I16(viewportPosX + STATIC_I16(viewportSizeX)), viewportPosY };
		EditorLayout.DockSpacePosition = { viewportPosX, STATIC_I16(viewportPosY + viewportSizeY) };
		
		EditorLayout.HierarchyViewSize = { STATIC_U16(viewportPosX), STATIC_U16(resolution.Y - viewportPosY) };
		EditorLayout.ViewportSize = { viewportSizeX, viewportSizeY };
		EditorLayout.InspectorSize = { STATIC_U16(viewportPosX), STATIC_U16(resolution.Y - viewportPosY) };
		EditorLayout.DockSpaceSize = { viewportSizeX, STATIC_U16(resolution.Y - STATIC_F32(viewportSizeY) - viewportPosY) };
	}

	void CEditorManager::ReinitEditorLayout()
	{
		if (!EditorLayout.HasLayoutChanged())
			return;

		const SVector2<U16> resolution = PlatformManager->GetResolution();
		const SVector2<F32>& viewportWorkPos = GUI::GetViewportWorkPos();

		I16 viewportPosY = STATIC_I16(viewportWorkPos.Y);

		I16 viewportPosX = EditorLayout.ViewportPosition.X;
		if (EditorLayout.HierarchyViewChanged)
			viewportPosX = STATIC_I16(EditorLayout.HierarchyViewSize.X);

		I16 inspectorPosX = EditorLayout.InspectorPosition.X;
		if (EditorLayout.ViewportChanged)
			inspectorPosX = EditorLayout.ViewportPosition.X + STATIC_I16(EditorLayout.ViewportSize.X);

		I16 dockSpacePosY = EditorLayout.DockSpacePosition.Y;
		if (EditorLayout.ViewportChanged)
			dockSpacePosY = EditorLayout.ViewportPosition.Y + STATIC_I16(EditorLayout.ViewportSize.Y);

		EditorLayout.HierarchyViewPosition = { 0, viewportPosY };
		EditorLayout.ViewportPosition = { viewportPosX, viewportPosY };
		EditorLayout.InspectorPosition = { inspectorPosX, viewportPosY };
		EditorLayout.DockSpacePosition = { viewportPosX, dockSpacePosY };

		EditorLayout.HierarchyViewSize = { STATIC_U16(viewportPosX), STATIC_U16(resolution.Y - STATIC_U16(viewportPosY)) };
		EditorLayout.ViewportSize = { STATIC_U16(inspectorPosX - viewportPosX), STATIC_U16(dockSpacePosY - viewportPosY) };
		EditorLayout.InspectorSize = { STATIC_U16(resolution.X - STATIC_U16(inspectorPosX)), STATIC_U16(resolution.Y - STATIC_U16(viewportPosY)) };
		EditorLayout.DockSpaceSize = { STATIC_U16(inspectorPosX - viewportPosX), STATIC_U16(resolution.Y - STATIC_U16(dockSpacePosY)) };

		EditorLayout.ViewportChanged = false;
		EditorLayout.HierarchyViewChanged = false;
		EditorLayout.InspectorChanged = false;
		EditorLayout.DockSpaceChanged = false;

		GUI::SetWindowPos("Viewport", { STATIC_F32(EditorLayout.ViewportPosition.X), STATIC_F32(EditorLayout.ViewportPosition.Y) });
		GUI::SetWindowSize("Viewport", { STATIC_F32(EditorLayout.ViewportSize.X), STATIC_F32(EditorLayout.ViewportSize.Y) });
		GUI::SetWindowPos("Hierarchy", { STATIC_F32(EditorLayout.HierarchyViewPosition.X), STATIC_F32(EditorLayout.HierarchyViewPosition.Y) });
		GUI::SetWindowSize("Hierarchy", { STATIC_F32(EditorLayout.HierarchyViewSize.X), STATIC_F32(EditorLayout.HierarchyViewSize.Y) });
		GUI::SetWindowPos("Inspector", { STATIC_F32(EditorLayout.InspectorPosition.X), STATIC_F32(EditorLayout.InspectorPosition.Y) });
		GUI::SetWindowSize("Inspector", { STATIC_F32(EditorLayout.InspectorSize.X), STATIC_F32(EditorLayout.InspectorSize.Y) });
		GUI::SetWindowPos("Dock Space", { STATIC_F32(EditorLayout.DockSpacePosition.X), STATIC_F32(EditorLayout.DockSpacePosition.Y) });
		GUI::SetWindowSize("Dock Space", { STATIC_F32(EditorLayout.DockSpaceSize.X), STATIC_F32(EditorLayout.DockSpaceSize.Y) });
	}

	void CEditorManager::InitAssetRepresentations()
	{
		// NR: Fill first slot with a null entry
		AssetRepresentations.emplace_back(std::make_unique<SEditorAssetRepresentation>());

		PreProcessAssets();

		for (const auto& entry : std::filesystem::recursive_directory_iterator(std::filesystem::path("Assets")))
		{
			if (entry.is_directory())
				continue;

			CreateAssetRep(entry.path());
		}
	}

	void CEditorManager::PreProcessAssets()
	{
		// NW: This probably doesn't make sense anymore now that we have import options. May return to this at some point though
		
		////Import non-.hva files to .hva
		//for (const auto& entry : std::filesystem::recursive_directory_iterator(std::filesystem::path("Assets")))
		//{
		//	if (entry.path().extension() == ".hva" || entry.is_directory())
		//		continue;

		//	std::string fileName = entry.path().string();
		//	std::string extension = entry.path().extension().string();
		//	for (auto& character : extension)
		//		character = std::tolower(character, {});

		//	EAssetType assetType = EAssetType::None;

		//	// TODO.NR: Make sure this works for animations as well
		//	if (extension == ".fbx")
		//		assetType = EAssetType::StaticMesh;
		//	else if (extension == ".dds" || extension == ".tga")
		//		assetType = EAssetType::Texture;
		//	else
		//		continue;

		//	ResourceManager->ConvertToHVA(fileName, fileName.substr(0, fileName.find_last_of('\\')), assetType);
		//}
	}

	void CEditorManager::InitEditorPreferences()
	{
		if (!UFileSystem::Exists(UserEditorSettingsPath))
		{
			std::filesystem::copy_file(
			DefaultEditorSettingsPath,	
				UserEditorSettingsPath);
		}
		
		UFileSystem::ReconcileJsonFiles(DefaultEditorSettingsPath, UserEditorSettingsPath);
		
		EditorPreferencesDocument = UFileSystem::OpenJson(UserEditorSettingsPath);
		
		EditorPreferences.Sensitivity = EditorPreferencesDocument.Get("Sensitivity", 0.5f);	
		EditorPreferences.EditorColorTheme = static_cast<EEditorColorTheme>(EditorPreferencesDocument.Get(EditorColorThemeKey, 1));
		EditorPreferences.PlayColorTheme = static_cast<EEditorColorTheme>(EditorPreferencesDocument.Get(PlayColorThemeKey, 1));
		EditorPreferences.PauseColorTheme = static_cast<EEditorColorTheme>(EditorPreferencesDocument.Get(PauseColorThemeKey, 1));
		
		SetEditorTheme(EditorPreferences.EditorColorTheme);
	}

	void CEditorManager::OnInputSetTransformGizmo(const SInputActionPayload payload)
	{
		if (GetIsFreeCamActive())
			return;

		switch (payload.Event)
		{
		case EInputActionEvent::TranslateTransform:
			CurrentGizmo = ETransformGizmo::Translate;
			break;
		case EInputActionEvent::RotateTransform:
			CurrentGizmo = ETransformGizmo::Rotate;
			break;
		case EInputActionEvent::ScaleTransform:
			CurrentGizmo = ETransformGizmo::Scale;
			break;
		default:
			break;
		}
	}

	void CEditorManager::OnInputToggleFreeCam(const SInputActionPayload payload)
	{
		IsFreeCamActive = GetEditorWindow<CViewportWindow>()->GetIsHovered() && payload.IsHeld;
	}

	void CEditorManager::OnInputFocusSelection(const SInputActionPayload payload)
	{
		if (!payload.IsPressed)
			return;

		// TODO.NW: Nice to figure out an average bounding box to focus on for all selected entities
		SEntity firstSelectedEntity = GetSelectedEntity();
		if (!firstSelectedEntity.IsValid())
			return;

		FocusEntity(firstSelectedEntity);
	}

	void CEditorManager::OnDeleteEvent(const SInputActionPayload payload)
	{
		if (!payload.IsPressed)
			return;

		for (SEntity& selectedEntity : GetSelectedEntities())
		{
			CScene* currentScene = GetContainingScene(selectedEntity);
			if (currentScene == nullptr)
				continue;

			if (IsEntityInsidePackedPrefab(selectedEntity))
			{
				SMetaDataComponent* metaDataComponent = currentScene->GetComponent<SMetaDataComponent>(selectedEntity);
				const std::string entityName = SComponent::IsValid(metaDataComponent) ? metaDataComponent->Name.AsString() : "UNNAMED";
				HV_LOG_WARN("CEditorManager::OnDeleteEvent: Can't delete entity %s from inside packed prefab! Use Prefab Editor or unpack the prefab.", entityName.c_str());
				continue;
			}

			UMetaCommandRouter::Push(SRemoveEntityEditAction::MakeEditActionCommand(this, selectedEntity, true));
			currentScene->RemoveEntity(selectedEntity);
		}
		
		ClearSelectedEntities();
	}

	void CEditorManager::OnToggleFullscreen(const SInputActionPayload payload)
	{
		if (payload.IsPressed)
			IsFullscreen = !IsFullscreen;
	}

	void CEditorManager::OnCopyEvent(const SInputActionPayload payload)
	{
		if (!payload.IsPressed)
			return;

		if (payload.Event == EInputActionEvent::Copy)
		{
			// NW: May choose to copy multiple entities here but might have to figure out how to store all the buffers then?
			const SEntity& selectedEntity = GetSelectedEntity();
			if (!selectedEntity.IsValid())
				return;

			CScene* scene = UComponentAlgo::GetContainingScene(selectedEntity, World->GetActiveScenes());
			if (scene == nullptr)
				return;

			EntityCopyBuffer = scene->GetEntityStringBuffer(selectedEntity);
		}
		else if (payload.Event == EInputActionEvent::Paste)
		{
			CScene* workingScene = GetCurrentWorkingScene();
			if (workingScene == nullptr)
				return;
			
			constexpr std::string_view bufferName = "ENTITYBUFFER";
			if (EntityCopyBuffer.size() <= bufferName.size())
				return;

			if (EntityCopyBuffer.substr(EntityCopyBuffer.size() - bufferName.size()) != bufferName.data())
				return;

			SEntity newEntity = workingScene->AddEntityFromStringBuffer(EntityCopyBuffer, true);
			if (!newEntity.IsValid())
				return;

			if (STransformComponent* transform = workingScene->GetComponent<STransformComponent>(newEntity))
			{
				SMatrix transformCopy = transform->Transform.GetMatrix();

				CViewportWindow* viewport = GetEditorWindow<CViewportWindow>();
				transformCopy.SetTranslation(viewport->GetWorldPositionOnPixel());
				
				transform->Transform.SetMatrix(transformCopy);
			}

			EntityCopyBuffer.clear();
		}
	}

	void CEditorManager::OnDragCopyEvent(const SInputActionPayload payload)
	{
		if (payload.IsPressed)
			IsDragCopyActive = true;
		if (payload.IsReleased)
			IsDragCopyActive = false;
	}

	void CEditorManager::OnEditorActionTreeEvent(const SInputActionPayload payload)
	{
		if (!payload.IsPressed)
			return;

		if (payload.Key == EInputKey::KeyZ)
			EditHistory.Undo();
		else if (payload.Key == EInputKey::KeyY)
			EditHistory.Redo();
	}

	void CEditorManager::OnPlayStateEvent(const SInputActionPayload payload)
	{
		CWorld* world = GEngine::GetWorld();

		if (payload.Event == EInputActionEvent::StartPlay && payload.IsPressed)
			world->BeginPlay();
		else if (payload.Event == EInputActionEvent::StopPlay && payload.IsPressed)
			world->StopPlay();
	}

	void CEditorManager::OnPivotMoving(const SInputActionPayload payload)
	{
		if (payload.IsPressed)
			IsPivotOffsetSet = !IsPivotOffsetSet;

		if (IsPivotOffsetSet)
			IsPivotMovingActive = payload.IsHeld;

	}

	void CEditorManager::OnVertexSnapping(const SInputActionPayload payload)
	{
		IsVertexSnappingActive = payload.IsHeld;
		// TODO.NW: Potentially make it a lest of exempt entities, and let the all selected entities be exempt
		payload.IsHeld ? World->SetEditorRenderExemptEntity(GetLastSelectedEntity()) : World->SetEditorRenderExemptEntity(SEntity::Null);
	}

	void CEditorManager::OnGridSnapping(const SInputActionPayload payload)
	{
		IsGridSnappingActive = payload.IsHeld;
	}

	void CEditorManager::OnResolutionChanged(SVector2<U16> newResolution)
	{
		HV_LOG_INFO("EditorMananger -> New Res X: %i, New Res Y: %i", newResolution.X, newResolution.Y);
		InitEditorLayout();
	}

	void CEditorManager::OnBeginPlay(std::vector<Ptr<CScene>>& /*scenes*/)
	{
		SetSelectedEntity(SEntity::Null);
		SetEditorTheme(EditorPreferences.PlayColorTheme, EEditorStyleTheme::Havtorn, DarknessOffsetPlayTheme);
		World->BlockSystem<CPickingSystem>(this);

		// TODO.NW: Change input context?
	}

	void CEditorManager::OnPausePlay(std::vector<Ptr<CScene>>& /*scenes*/)
	{
		SetEditorTheme(EditorPreferences.PauseColorTheme, EEditorStyleTheme::Havtorn, DarknessOffsetPausedTheme);
		World->UnblockSystem<CPickingSystem>(this);
	}

	void CEditorManager::OnEndPlay(std::vector<Ptr<CScene>>& /*scenes*/)
	{
		SetEditorTheme(EditorPreferences.EditorColorTheme, EEditorStyleTheme::Havtorn, DarknessOffsetStoppedTheme);
		World->UnblockSystem<CPickingSystem>(this);
	}

	ETransformGizmo CEditorManager::GetCurrentGizmo() const
	{
		return CurrentGizmo;
	}

	ETransformGizmoSpace CEditorManager::GetCurrentGizmoSpace() const
	{
		return CurrentGizmoSpace;
	}

	const SSnappingOption& CEditorManager::GetCurrentGizmoSnapping() const
	{
		return CurrentGizmoSnapping;
	}

	bool CEditorManager::GetIsFreeCamActive() const
	{
		return IsFreeCamActive;
	}

	bool CEditorManager::GetIsOverGizmo() const
	{
		return GUI::IsOverGizmo();
	}

	bool CEditorManager::GetIsModalOpen() const
	{
		return IsModalOpen;
	}

	bool CEditorManager::GetIsDragCopyActive() const
	{
		return IsDragCopyActive;
	}

	bool CEditorManager::GetIsPivotOffsetSet() const
	{
		return IsPivotOffsetSet;
	}

	bool CEditorManager::GetIsPivotMovingActive() const
	{
		return IsPivotMovingActive;
	}

	bool CEditorManager::GetIsVertexSnappingActive() const
	{
		return IsVertexSnappingActive;
	}

	bool CEditorManager::GetIsGridSnappingActive() const
	{
		return IsGridSnappingActive;
	}

	void CEditorManager::SetGizmoSpace(const ETransformGizmoSpace space)
	{
		CurrentGizmoSpace = space;
	}

	void CEditorManager::SetGizmoSnapping(const SSnappingOption& snapping)
	{
		CurrentGizmoSnapping = snapping;
	}

	void CEditorManager::SetPivotMoving(const bool active)
	{
		IsPivotMovingActive = active;
		IsPivotOffsetSet = IsPivotMovingActive;
	}

	void CEditorManager::SetVertexSnapping(const bool active)
	{
		IsVertexSnappingActive = active;
	}

	void CEditorManager::SetGridSnapping(const bool active)
	{
		IsGridSnappingActive = active;
	}

	void CEditorManager::SetIsModalOpen(const bool isModalOpen)
	{
		IsModalOpen = isModalOpen;
	}

	std::string CEditorManager::GetFrameRate() const
	{
		std::string frameRateString = "Framerate: ";
		const U32 frameRate = STATIC_U32(GTime::AverageFrameRate());
		const F32 frameTime = 1000.0f / frameRate;
		std::string frameTimeString = std::format("{:.2f}", frameTime);

		frameRateString.append(std::to_string(frameRate));
		frameRateString.append(" (");
		frameRateString.append(frameTimeString);
		frameRateString.append(" ms)");

		frameRateString.append(" | CPU: ");
		const U32 frameRateCPU = STATIC_U32(GTime::AverageFrameRate(ETimerCategory::CPU));
		const F32 frameTimeCPU = 1000.0f / frameRateCPU;
		std::string frameTimeStringCPU = std::format("{:.2f}", frameTimeCPU);
		frameRateString.append(frameTimeStringCPU);
		frameRateString.append(" ms");

		frameRateString.append(" | GPU: ");
		const U32 frameRateGPU = STATIC_U32(GTime::AverageFrameRate(ETimerCategory::GPU));
		const F32 frameTimeGPU = 1000.0f / frameRateGPU;
		std::string frameTimeStringGPU = std::format("{:.2f}", frameTimeGPU);
		frameRateString.append(frameTimeStringGPU);
		frameRateString.append(" ms");

		return frameRateString;
	}

	std::string CEditorManager::GetSystemMemory() const
	{
		PROCESS_MEMORY_COUNTERS memCounter;
		if (GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof memCounter))
		{
			const SIZE_T memUsed = (memCounter.WorkingSetSize) / 1024;
			const SIZE_T memUsedMb = (memCounter.WorkingSetSize) / 1024 / 1024;

			std::string mem = "System Memory: ";
			mem.append(std::to_string(memUsed));
			mem.append("Kb (");
			mem.append(std::to_string(memUsedMb));
			mem.append(" Mb)");

			return mem;
		}

		return "";
	}

	std::string CEditorManager::GetRenderInfo() const
	{
		std::string info = "Draw Calls: ";
		info.append(std::to_string(CRenderManager::NumberOfDrawCallsThisFrame));
		info.append("\nRender Views: ");
		info.append(std::to_string(RenderManager->GetNumberOfRenderViews()));
		return info;
	}
}
