// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditorManager.h"

#include <WindowsInclude.h>
#include <psapi.h>
#include <format>

#include <Engine.h>
#include <FileSystem/FileSystem.h>
#include <Input/InputMapper.h>
#include <Input/InputTypes.h>

#include <Graphics/RenderManager.h>
#include <ECS/ECSInclude.h>

#include "EditorResourceManager.h"
#include "EditorWindows.h"
#include "EditorToggleable.h"
#include "EditorToggleables.h"

#include "Systems/PickingSystem.h"

//#include <Application/ImGuiCrossProjectSetup.h>
#include <MathTypes/MathUtilities.h>
#include <PlatformManager.h>
#include <Color.h>
#include <Timer.h>

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
	}

	CEditorManager::~CEditorManager()
	{
		RenderManager = nullptr;
		SAFE_DELETE(ResourceManager);
	}

	bool CEditorManager::Init(CPlatformManager* platformManager, const CGraphicsFramework* framework, CRenderManager* renderManager)
	{
		PlatformManager = platformManager;
		if (PlatformManager == nullptr)
			return false;

		SetEditorTheme(EEditorColorTheme::HavtornDark, EEditorStyleTheme::Havtorn);

		// TODO.NR: Figure out why we can't use unique ptrs with these namespaced imgui classes
		MenuElements.emplace_back(new CFileMenu("File", this));
		MenuElements.emplace_back(new CEditMenu("Edit", this));
		MenuElements.emplace_back(new CViewMenu("View", this));
		MenuElements.emplace_back(new CWindowMenu("Window", this));
		MenuElements.emplace_back(new CHelpMenu("Help", this));

		Windows.emplace_back(new CViewportWindow("Viewport", this));
		Windows.emplace_back(new CDockSpaceWindow("Dock Space", this));
		Windows.emplace_back(new CAssetBrowserWindow("Asset Browser", this));
		Windows.emplace_back(new COutputLogWindow("Output Log", this));
		Windows.emplace_back(new CHierarchyWindow("Hierarchy", this));
		Windows.emplace_back(new CInspectorWindow("Inspector", this));
		Windows.emplace_back(new CSpriteAnimatorGraphNodeWindow("Sprite Animator", this));
		Windows.back()->SetEnabled(false);

		ResourceManager = new CEditorResourceManager();
		bool success = ResourceManager->Init(renderManager, framework);
		if (!success)
			return false;
		RenderManager = renderManager;

		PlatformManager->OnResolutionChanged.AddMember(this, &CEditorManager::OnResolutionChanged);
		World = GEngine::GetWorld();
		World->OnBeginPlayDelegate.AddMember(this, &CEditorManager::OnBeginPlay);
		World->OnPausePlayDelegate.AddMember(this, &CEditorManager::OnPausePlay);
		World->OnStopPlayDelegate.AddMember(this, &CEditorManager::OnStopPlay);
		World->RequestSystem<CPickingSystem>(this, this);

		InitEditorLayout();
		InitAssetRepresentations();

		return success;
	}

	void CEditorManager::BeginFrame()
	{
	}

	void CEditorManager::Render()
	{
		// Main Menu bar
		if (IsEnabled)
		{
			GUI::BeginMainMenuBar();

			for (const auto& element : MenuElements)
				element->OnInspectorGUI();

			GUI::EndMainMenuBar();
		}

		// Windows
		for (const auto& window : Windows)
		{
			if (!window->GetEnabled())
				continue;

			window->OnInspectorGUI();
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
				GUI::Text(GetDrawCalls().c_str());
			}

			GUI::End();
		}
	}

	void CEditorManager::SetCurrentScene(CScene* scene)
	{
		CurrentScene = scene;
		SelectedEntities.clear();
	}

	CScene* CEditorManager::GetCurrentScene() const
	{
		return CurrentScene;
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

	//const Ptr<SEditorAssetRepresentation>& CEditorManager::GetAssetRepFromImageRef(void* imageRef) const
	//{
	//	for (const auto& rep : AssetRepresentations)
	//	{
	//		if (imageRef == rep->TextureRef)
	//			return rep;
	//	}

	//	// NR: Return empty rep
	//	return AssetRepresentations[0];
	//}

	std::function<SAssetInspectionData(std::filesystem::directory_entry)> CEditorManager::GetAssetInspectFunction() const
	{
		return [this](std::filesystem::directory_entry entry)
			{
				const Ptr<SEditorAssetRepresentation>& assetRep = GetAssetRepFromDirEntry(entry);
				return SAssetInspectionData(assetRep->Name, (intptr_t)assetRep->TextureRef.GetShaderResourceView());
			};
	}

	void CEditorManager::CreateAssetRep(const std::filesystem::path& path)
	{
		std::filesystem::directory_entry entry(path);
		HV_ASSERT(!entry.is_directory(), "You are trying to create SEditorAssetRepresentation but you're creating a new folder.");

		std::string filePath = path.string();
		const U64 fileSize = UMath::Max(GEngine::GetFileSystem()->GetFileSize(filePath), sizeof(EAssetType));
		char* data = new char[fileSize];

		GEngine::GetFileSystem()->Deserialize(filePath, data, STATIC_U32(fileSize));

		SEditorAssetRepresentation rep;

		const U32 size = sizeof(EAssetType);
		memcpy(&rep.AssetType, &data[0], size);

		rep.DirectoryEntry = entry;
		rep.Name = entry.path().filename().string();
		rep.Name = rep.Name.substr(0, rep.Name.length() - 4);
		// TODO.NW: Save these in the textureBank? Might be impossible with animated thumbnails. Figure out ownership
		rep.TextureRef = ResourceManager->RenderAssetTexure(rep.AssetType, filePath);

		AssetRepresentations.emplace_back(std::make_unique<SEditorAssetRepresentation>(rep));

		delete[] data;
	}

	void CEditorManager::RemoveAssetRep(const std::filesystem::directory_entry& sourceEntry)
	{
		auto& rep = GetAssetRepFromDirEntry(sourceEntry);
		auto it = std::ranges::find(AssetRepresentations, rep);
		if (it != AssetRepresentations.end())
		{
			it->get()->TextureRef.Release();
			AssetRepresentations.erase(it);	
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

		if (CurrentScene == nullptr)
			return;

		SEntity cameraEntity = CurrentScene->MainCameraEntity;
		if (!cameraEntity.IsValid())
			return;

		SVector worldPos = SVector::Zero;
		SVector center = SVector::Zero;
		SVector bounds = SVector::Zero;
		SVector2<F32> fov = SVector2<F32>::Zero;
		bool foundBounds = false;

		if (STransformComponent* transform = CurrentScene->GetComponent<STransformComponent>(firstSelectedEntity))
		{
			worldPos = transform->Transform.GetMatrix().GetTranslation();
		}
		else
		{
			HV_LOG_WARN("OnInputFocusSelection: could not find a transform to go to. Cannot focus current selection.");
			return;
		}

		if (SCameraComponent* camera = CurrentScene->GetComponent<SCameraComponent>(cameraEntity))
		{
			fov = { camera->AspectRatio * camera->FOV, camera->FOV };
		}
		else
		{
			HV_LOG_WARN("OnInputFocusSelection: could not find a camera component to move. Cannot focus current selection.");
			return;
		}

		if (SStaticMeshComponent* staticMesh = CurrentScene->GetComponent<SStaticMeshComponent>(firstSelectedEntity))
		{
			center = staticMesh->BoundsCenter;
			bounds = SVector::GetAbsMaxKeepValue(staticMesh->BoundsMax, staticMesh->BoundsMin);
			foundBounds = true;
		}

		if (!foundBounds)
		{
			HV_LOG_WARN("OnInputFocusSelection: could not find component to derive bounds from. Cannot focus current selection.");
			return;
		}

		constexpr F32 focusMarginPercentage = 1.1f;

		STransform newTransform;
		newTransform.Orbit(SVector4(), SMatrix::CreateRotationFromEuler(10.0f, 10.0f, 0.0f));
		newTransform.Translate(worldPos + SVector(center.X, center.Y, -UMathUtilities::GetFocusDistanceForBounds(center, bounds, fov, focusMarginPercentage)));
		CurrentScene->GetComponent<STransformComponent>(cameraEntity)->Transform = newTransform;
	}

	void CEditorManager::OnDeleteEvent(const SInputActionPayload payload)
	{
		if (!payload.IsPressed)
			return;

		for (SEntity& selectedEntity : GetSelectedEntities())
			CurrentScene->RemoveEntity(selectedEntity);
		
		ClearSelectedEntities();
	}

	void CEditorManager::OnResolutionChanged(SVector2<U16> newResolution)
	{
		HV_LOG_INFO("EditorMananger -> New Res X: %i, New Res Y: %i", newResolution.X, newResolution.Y);
		InitEditorLayout();
	}

	void CEditorManager::OnBeginPlay(CScene* /*scene*/)
	{
		if (CurrentColorTheme != EEditorColorTheme::PauseMode)
			CachedColorTheme = CurrentColorTheme;

		SetSelectedEntity(SEntity::Null);
		SetEditorTheme(EEditorColorTheme::PlayMode);
		World->BlockSystem<CPickingSystem>(this);

		// TODO.NW: Change input context?
	}

	void CEditorManager::OnPausePlay(CScene* /*scene*/)
	{
		SetEditorTheme(EEditorColorTheme::PauseMode);
		World->UnblockSystem<CPickingSystem>(this);
	}

	void CEditorManager::OnStopPlay(CScene* /*scene*/)
	{
		SetEditorTheme(CachedColorTheme);
		World->UnblockSystem<CPickingSystem>(this);
	}

	ETransformGizmo CEditorManager::GetCurrentGizmo() const
	{
		return CurrentGizmo;
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

	std::string CEditorManager::GetDrawCalls() const
	{
		std::string drawCalls = "Draw Calls: ";
		drawCalls.append(std::to_string(CRenderManager::NumberOfDrawCallsThisFrame));
		return drawCalls;
	}
}
