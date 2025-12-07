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

#include "Systems/EditorRenderSystem.h"
#include "Systems/PickingSystem.h"
#include <../Game/GameScene.h>
#include <../Game/GameScript.h>
#include <MathTypes/MathUtilities.h>
#include <PlatformManager.h>
#include <Color.h>
#include <Timer.h>
#include <Assets/AssetRegistry.h>

namespace Havtorn
{
	std::string CEditorManager::PreviewMaterial = "Resources/M_MeshPreview.hva";

	CEditorManager::CEditorManager()
	{
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::TranslateTransform).AddMember(this, &CEditorManager::OnInputSetTransformGizmo);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::RotateTransform).AddMember(this, &CEditorManager::OnInputSetTransformGizmo);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ScaleTransform).AddMember(this, &CEditorManager::OnInputSetTransformGizmo);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ToggleFreeCam).AddMember(this, &CEditorManager::OnInputToggleFreeCam);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::FocusEditorEntity).AddMember(this, &CEditorManager::OnInputFocusSelection);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::DeleteEvent).AddMember(this, &CEditorManager::OnDeleteEvent);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ToggleFullscreen).AddMember(this, &CEditorManager::OnToggleFullscreen);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::AltPress).AddMember(this, &CEditorManager::OnDragCopyEvent);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::AltRelease).AddMember(this, &CEditorManager::OnDragCopyEvent);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::Copy).AddMember(this, &CEditorManager::OnCopyEvent);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::Paste).AddMember(this, &CEditorManager::OnCopyEvent);
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

		SetEditorTheme(EEditorColorTheme::HavtornDark, EEditorStyleTheme::Havtorn);

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
		Windows.back()->SetEnabled(false);
		Windows.emplace_back(std::make_unique<CMaterialTool>("Material Editor", this));
		Windows.back()->SetEnabled(false);
		Windows.emplace_back(std::make_unique<CScriptTool>("Script Editor", this));
		Windows.back()->SetEnabled(false);

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

		return success;
	}

	void CEditorManager::BeginFrame()
	{
		PlatformManager->UpdateResolution();
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

			if (GUI::ImageButton("MazimizeButton", ResourceManager->GetStaticEditorTextureResource(EEditorTexture::MaximizeWindow), SVector2<F32>(menuElementHeight)))
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

		if (GUI::IsLeftMouseHeld() && !isHoveringMenuBarButton)
			PlatformManager->UpdateWindowPos();
		else
			PlatformManager->UpdateRelativeCursorToWindowPos();

		// TODO.NW: Get from style
		constexpr F32 menuBarHeight = 18.0f;
		const bool isInMenuBarRect = GUI::IsMouseInRect(SVector2<F32>(0.0f), SVector2<F32>(STATIC_F32(PlatformManager->GetResolution().X), menuBarHeight));
		if (isInMenuBarRect && GUI::IsDoubleClick() && !isHoveringMenuBarButton)
			PlatformManager->MaximizeWindow();

		// Windows
		for (const auto& window : Windows)
		{
			if (window->GetEnabled())
				window->OnInspectorGUI();

			if (!window->WasEnabled && window->GetEnabled())
				window->OnEnable();
			if (window->WasEnabled && !window->GetEnabled())
				window->OnDisable();

			window->WasEnabled = window->GetEnabled();
		}

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

	void CEditorManager::SetSelectedEntity(const SEntity& entity)
	{
		ClearSelectedEntities();
		AddSelectedEntity(entity);
	}

	void CEditorManager::AddSelectedEntity(const SEntity& entity)
	{
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

	DirEntryFunc CEditorManager::GetAssetInspectFunction() const
	{
		return [this](std::filesystem::directory_entry entry)
			{
				const Ptr<SEditorAssetRepresentation>& assetRep = GetAssetRepFromDirEntry(entry);	
				return SAssetInspectionData(assetRep->Name, (intptr_t)assetRep->TextureRef.GetShaderResourceView(), assetRep->DirectoryEntry.path().string());
			};
	}

	DirEntryEAssetTypeFunc CEditorManager::GetAssetFilteredInspectFunction() const
	{
		return [this](std::filesystem::directory_entry entry, const EAssetType assetTypeFilter)
			{
				const Ptr<SEditorAssetRepresentation>& assetRep = GetAssetRepFromDirEntry(entry);
				if (assetRep->AssetType == assetTypeFilter)
					return SAssetInspectionData(assetRep->Name, (intptr_t)assetRep->TextureRef.GetShaderResourceView(), assetRep->DirectoryEntry.path().string());

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
	}

	void CEditorManager::SetEditorTheme(EEditorColorTheme colorTheme, EEditorStyleTheme styleTheme)
	{
		CurrentColorTheme = colorTheme;

		switch (colorTheme)
		{
		case EEditorColorTheme::HavtornDark:
		{
			GUI::SetGuiColorProfile(SGuiColorProfile());
		}
		break;

		case EEditorColorTheme::HavtornRed:
		{
			SGuiColorProfile colorProfile(
				SColor(0.11f, 0.11f, 0.11f, 1.00f),
				SColor(0.198f, 0.198f, 0.198f, 1.00f),
				SColor(0.278f, 0.271f, 0.267f, 1.00f),
				SColor(0.478f, 0.188f, 0.188f, 1.00f),
				SColor(0.814f, 0.00f, 0.00f, 1.00f),
				SColor(1.00f, 0.00f, 0.00f, 1.00f)
			);
			GUI::SetGuiColorProfile(colorProfile);
		}
		break;

		case EEditorColorTheme::HavtornGreen:
		{
			SGuiColorProfile colorProfile(
				SColor(0.11f, 0.11f, 0.11f, 1.00f),
				SColor(0.198f, 0.198f, 0.198f, 1.00f),
				SColor(0.278f, 0.271f, 0.267f, 1.00f),
				SColor(0.355f, 0.478f, 0.188f, 1.00f),
				SColor(0.469f, 0.814f, 0.00f, 1.00f),
				SColor(0.576f, 1.00f, 0.00f, 1.00f)
			);
			GUI::SetGuiColorProfile(colorProfile);
		}
		break;

		case EEditorColorTheme::PlayMode:
		{
			SGuiColorProfile colorProfile(
				SColor(0.01f, 0.01f, 0.01f, 1.00f),
				SColor(0.05f, 0.05f, 0.04f, 1.00f),
				SColor(0.11f, 0.11f, 0.11f, 1.00f),
				SColor(0.11f, 0.11f, 0.11f, 1.00f),
				SColor(0.11f, 0.11f, 0.11f, 1.00f),
				SColor(0.11f, 0.11f, 0.11f, 1.00f)
			);

			colorProfile.Text = SColor(0.44f, 0.44f, 0.44f, 1.00f);
			colorProfile.Button = SColor(0.278f, 0.271f, 0.267f, 1.00f);
			colorProfile.ButtonActive = SColor(0.814f, 0.532f, 0.00f, 1.00f);
			colorProfile.ButtonHovered = SColor(0.478f, 0.361f, 0.188f, 1.00f);

			GUI::SetGuiColorProfile(colorProfile);
		}
		break;

		case EEditorColorTheme::PauseMode:
		{
			SGuiColorProfile colorProfile(
				SColor(0.05f, 0.05f, 0.04f, 1.00f),
				SColor(0.11f, 0.11f, 0.11f, 1.00f),
				SColor(0.278f, 0.271f, 0.267f, 1.00f),
				SColor(0.478f, 0.361f, 0.188f, 1.00f),
				SColor(0.814f, 0.532f, 0.00f, 1.00f),
				SColor(1.00f, 0.659f, 0.00f, 1.00f)
			);
			GUI::SetGuiColorProfile(colorProfile);
		}
		break;

		case EEditorColorTheme::Count:
		case EEditorColorTheme::DefaultDark:
			// NW: Could be imgui colors instead?
			GUI::SetGuiColorProfile(SGuiColorProfile());
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
		case EEditorColorTheme::DefaultDark:
			return "ImGui Dark";
		case EEditorColorTheme::HavtornDark:
			return "Havtorn Dark";
		case EEditorColorTheme::HavtornRed:
			return "Havtorn Red";
		case EEditorColorTheme::HavtornGreen:
			return "Havtorn Green";
		case EEditorColorTheme::Count:
			return {};
		}
		return {};
	}

	SColor CEditorManager::GetEditorColorThemeRepColor(const EEditorColorTheme colorTheme)
	{
		switch (colorTheme)
		{
		case EEditorColorTheme::DefaultDark:
			return { 0.11f, 0.16f, 0.55f, 1.00f };
		case EEditorColorTheme::HavtornDark:
			return { 0.478f, 0.361f, 0.188f, 1.00f };
		case EEditorColorTheme::HavtornRed:
			return { 0.478f, 0.188f, 0.188f, 1.00f };
		case EEditorColorTheme::HavtornGreen:
			return { 0.355f, 0.478f, 0.188f, 1.00f };
		case EEditorColorTheme::Count:
			return {};
		}
		return {};
	}

	const SEditorLayout& CEditorManager::GetEditorLayout() const
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

	void CEditorManager::InitEditorLayout()
	{
		EditorLayout = SEditorLayout();

		const SVector2<U16> resolution = PlatformManager->GetResolution();

		constexpr F32 viewportAspectRatioInv = (9.0f / 16.0f);
		const F32 viewportPaddingX = ViewportPadding;
		constexpr F32 viewportPaddingY = 0.0f;

		I16 viewportPosX = static_cast<I16>(resolution.X * viewportPaddingX);
		I16 viewportPosY = static_cast<I16>(viewportPaddingY);
		U16 viewportSizeX = STATIC_U16(resolution.X - (2.0f * STATIC_F32(viewportPosX)));
		U16 viewportSizeY = STATIC_U16(STATIC_F32(viewportSizeX) * viewportAspectRatioInv);

		// NR: This might be windows menu bar height?
		U16 sizeOffsetY = 18;

		EditorLayout.ViewportPosition = { viewportPosX, viewportPosY };
		EditorLayout.ViewportSize = { viewportSizeX, viewportSizeY };
		EditorLayout.AssetBrowserPosition = { viewportPosX, static_cast<I16>(viewportPosY + viewportSizeY) };
		EditorLayout.AssetBrowserSize = { viewportSizeX, STATIC_U16(resolution.Y - STATIC_F32(viewportSizeY) - sizeOffsetY) };
		EditorLayout.HierarchyViewPosition = { 0, viewportPosY };
		EditorLayout.HierarchyViewSize = { STATIC_U16(viewportPosX), STATIC_U16(resolution.Y - sizeOffsetY) };
		EditorLayout.InspectorPosition = { static_cast<I16>(resolution.X - STATIC_F32(viewportPosX)), viewportPosY };
		EditorLayout.InspectorSize = { STATIC_U16(viewportPosX), STATIC_U16(resolution.Y - sizeOffsetY) };
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
		IsFreeCamActive = payload.IsHeld;
	}

	void CEditorManager::OnInputFocusSelection(const SInputActionPayload payload)
	{
		if (!payload.IsPressed)
			return;

		// TODO.NW: Nice to figure out an average bounding box to focus on for all selected entities
		SEntity firstSelectedEntity = GetSelectedEntity();
		if (!firstSelectedEntity.IsValid())
			return;

		const std::vector<Ptr<CScene>>& scenes = World->GetActiveScenes();
		CScene* currentScene = UComponentAlgo::GetContainingScene(firstSelectedEntity, scenes);
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

		if (STransformComponent* transform = currentScene->GetComponent<STransformComponent>(firstSelectedEntity))
		{
			worldPos = transform->Transform.GetMatrix().GetTranslation();
		}
		else
		{
			HV_LOG_WARN("OnInputFocusSelection: could not find a transform to go to. Cannot focus current selection.");
			return;
		}

		fov = { mainCameraData.CameraComponent->AspectRatio * mainCameraData.CameraComponent->FOV, mainCameraData.CameraComponent->FOV };

		if (SStaticMeshComponent* staticMesh = currentScene->GetComponent<SStaticMeshComponent>(firstSelectedEntity))
		{
			SStaticMeshAsset* meshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SStaticMeshAsset>(staticMesh->AssetReference, CAssetRegistry::EditorManagerRequestID);
			center = meshAsset->BoundsCenter;
			bounds = SVector::GetAbsMaxKeepValue(meshAsset->BoundsMax, meshAsset->BoundsMin);
			GEngine::GetAssetRegistry()->UnrequestAsset(staticMesh->AssetReference, CAssetRegistry::EditorManagerRequestID);
			foundBounds = true;
		}
		else if (SSkeletalMeshComponent* skeletalMesh = currentScene->GetComponent<SSkeletalMeshComponent>(firstSelectedEntity))
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
		
		CScene* mainCameraScene = UComponentAlgo::GetContainingScene(mainCamera, scenes);
		if (SCameraControllerComponent* controllerComp = mainCameraScene->GetComponent<SCameraControllerComponent>(mainCamera))
		{
			SVector currentEuler = mainCameraData.TransformComponent->Transform.GetMatrix().GetEuler();
			controllerComp->CurrentPitch = UMath::Clamp(currentEuler.X, -SCameraControllerComponent::MaxPitchDegrees + 0.01f, SCameraControllerComponent::MaxPitchDegrees - 0.01f);;
			controllerComp->CurrentYaw = UMath::WrapAngle(currentEuler.Y);
		}
	}

	void CEditorManager::OnDeleteEvent(const SInputActionPayload payload)
	{
		if (!payload.IsPressed)
			return;

		const std::vector<Ptr<CScene>>& scenes = World->GetActiveScenes();
		for (SEntity& selectedEntity : GetSelectedEntities())
		{
			CScene* currentScene = UComponentAlgo::GetContainingScene(selectedEntity, scenes);
			if (currentScene == nullptr)
				continue;

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

		//if (payload.Event == EInputActionEvent::Copy)
		//	// copy
		//else if (payload.Event == EInputActionEvent::Paste)
		//	// paste
	}

	void CEditorManager::OnDragCopyEvent(const SInputActionPayload payload)
	{
		if (payload.IsPressed)
			IsDragCopyActive = true;
		if (payload.IsReleased)
			IsDragCopyActive = false;
	}

	void CEditorManager::OnResolutionChanged(SVector2<U16> newResolution)
	{
		HV_LOG_INFO("EditorMananger -> New Res X: %i, New Res Y: %i", newResolution.X, newResolution.Y);
		InitEditorLayout();
	}

	void CEditorManager::OnBeginPlay(std::vector<Ptr<CScene>>& /*scenes*/)
	{
		if (CurrentColorTheme != EEditorColorTheme::PauseMode)
			CachedColorTheme = CurrentColorTheme;

		SetSelectedEntity(SEntity::Null);
		SetEditorTheme(EEditorColorTheme::PlayMode);
		World->BlockSystem<CPickingSystem>(this);

		// TODO.NW: Change input context?
	}

	void CEditorManager::OnPausePlay(std::vector<Ptr<CScene>>& /*scenes*/)
	{
		SetEditorTheme(EEditorColorTheme::PauseMode);
		World->UnblockSystem<CPickingSystem>(this);
	}

	void CEditorManager::OnEndPlay(std::vector<Ptr<CScene>>& /*scenes*/)
	{
		SetEditorTheme(CachedColorTheme);
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

	void CEditorManager::SetGizmoSpace(const ETransformGizmoSpace space)
	{
		CurrentGizmoSpace = space;
	}

	void CEditorManager::SetGizmoSnapping(const SSnappingOption& snapping)
	{
		CurrentGizmoSnapping = snapping;
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
