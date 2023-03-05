// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditorManager.h"

#include "Core/imgui.h"
#include "Core/imgui_impl_win32.h"
#include "Core/imgui_impl_dx11.h"
#include "Core/ImGuizmo/ImGuizmo.h"

#include <windows.h>
#include <psapi.h>
#include <format>

#include "Engine.h"
#include "FileSystem/FileSystem.h"
#include "Application/WindowHandler.h"
#include "Input/InputMapper.h"
#include "Input/InputTypes.h"

#include "Graphics/GraphicsFramework.h"
#include "Graphics/RenderManager.h"
#include "ECS/ECSInclude.h"

#include "EditorResourceManager.h"
#include "EditorWindows.h"
#include "EditorToggleables.h"

#include <Application/ImGuiCrossProjectSetup.h>

namespace Havtorn
{
	CEditorManager::CEditorManager()
	{
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::TranslateTransform).AddMember(this, &CEditorManager::SetTransformGizmo);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::RotateTransform).AddMember(this, &CEditorManager::SetTransformGizmo);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ScaleTransform).AddMember(this, &CEditorManager::SetTransformGizmo);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ToggleFreeCam).AddMember(this, &CEditorManager::ToggleFreeCam);
	}

	CEditorManager::~CEditorManager()
	{
		RenderManager = nullptr;
		SAFE_DELETE(ResourceManager);
	}

	bool CEditorManager::Init(const CGraphicsFramework* framework, const CWindowHandler* windowHandler, CRenderManager* renderManager, CScene* scene)
	{
		CROSS_PROJECT_IMGUI_SETUP();
		windowHandler->EnableDragDrop();

		SetEditorTheme(EEditorColorTheme::HavtornDark, EEditorStyleTheme::Havtorn);
		
		MenuElements.emplace_back(std::make_unique<ImGui::CFileMenu>("File", this));
		MenuElements.emplace_back(std::make_unique<ImGui::CEditMenu>("Edit", this));
		MenuElements.emplace_back(std::make_unique<ImGui::CViewMenu>("View", this));
		MenuElements.emplace_back(std::make_unique<ImGui::CWindowMenu>("Window", this));
		MenuElements.emplace_back(std::make_unique<ImGui::CHelpMenu>("Help", this));

		Windows.emplace_back(std::make_unique<ImGui::CViewportWindow>("Viewport", this));
		Windows.emplace_back(std::make_unique<ImGui::CAssetBrowserWindow>("Asset Browser", this));
		Windows.emplace_back(std::make_unique<ImGui::CHierarchyWindow>("Hierarchy", scene, this));
		Windows.emplace_back(std::make_unique<ImGui::CInspectorWindow>("Inspector", scene, this));

		ResourceManager = new CEditorResourceManager();
		bool success = ResourceManager->Init(renderManager, framework);
		if (!success)
			return false;
		RenderManager = renderManager;

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
			ImGui::BeginMainMenuBar();
		
			for (const auto& element : MenuElements)
				element->OnInspectorGUI();
		
			ImGui::EndMainMenuBar();
		}
		
		// Windows
		for (const auto& window : Windows)
			window->OnInspectorGUI();
		
		DebugWindow();
	}

	void CEditorManager::EndFrame()
	{
	}

	void CEditorManager::DebugWindow()
	{
		if (IsDemoOpen)
		{
			ImGui::ShowDemoWindow(&IsDemoOpen);
		}

		if (IsDebugInfoOpen)
		{
			if (ImGui::Begin("Debug info", &IsDebugInfoOpen))
			{
				ImGui::Text(GetFrameRate().c_str());
				ImGui::Text(GetSystemMemory().c_str());
				ImGui::Text(GetDrawCalls().c_str());
			}

			ImGui::End();
		}
	}

	void CEditorManager::SetSelectedEntity(Ref<SEntity> entity)
	{
		SelectedEntity = entity;
	}

	Ref<SEntity> CEditorManager::GetSelectedEntity() const
	{
		return SelectedEntity;
	}

	const Ptr<SEditorAssetRepresentation>& CEditorManager::GetAssetRepFromDirEntry(const std::filesystem::directory_entry& dirEntry)
	{
		for (const auto& rep : AssetRepresentations)
		{
			if (dirEntry == rep->DirectoryEntry)
				return rep;
		}

		// NR: Return empty rep
		return AssetRepresentations[0];
	}

	const Ptr<SEditorAssetRepresentation>& CEditorManager::GetAssetRepFromName(const std::string& assetName)
	{
		for (const auto& rep : AssetRepresentations)
		{
			if (assetName == rep->Name)
				return rep;
		}

		// NR: Return empty rep
		return AssetRepresentations[0];
	}

	const Ptr<SEditorAssetRepresentation>& CEditorManager::GetAssetRepFromImageRef(void* imageRef)
	{
		for (const auto& rep : AssetRepresentations)
		{
			if (imageRef == rep->TextureRef)
				return rep;
		}

		// NR: Return empty rep
		return AssetRepresentations[0];
	}

	void CEditorManager::CreateAssetRep(const std::filesystem::path& path)
	{
		std::filesystem::directory_entry entry(path);
		HV_ASSERT(!entry.is_directory(), "You are trying to create SEditorAssetRepresentation but you're creating a new folder.");

		std::string filePath = path.string();
		const U64 fileSize = UMath::Max(GEngine::GetFileSystem()->GetFileSize(filePath), sizeof(EAssetType));
		char* data = new char[fileSize];

		GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));

		SEditorAssetRepresentation rep;

		const U32 size = sizeof(EAssetType);
		memcpy(&rep.AssetType, &data[0], size);

		rep.DirectoryEntry = entry;
		rep.Name = entry.path().filename().string();
		rep.Name = rep.Name.substr(0, rep.Name.length() - 4);
		rep.TextureRef = ResourceManager->RenderAssetTexure(rep.AssetType, filePath);

		AssetRepresentations.emplace_back(std::make_unique<SEditorAssetRepresentation>(rep));

		delete[] data;
	}

	void CEditorManager::SetEditorTheme(EEditorColorTheme colorTheme, EEditorStyleTheme styleTheme)
	{
		switch (colorTheme)
		{
		case Havtorn::EEditorColorTheme::HavtornDark:
		{
			GImGuiManager::SetImGuiColorProfile(SImGuiColorProfile());
		}
		break;

		case Havtorn::EEditorColorTheme::HavtornRed:
		{
			SImGuiColorProfile colorProfile(
				ImVec4(0.11f, 0.11f, 0.11f, 1.00f),
				ImVec4(0.198f, 0.198f, 0.198f, 1.00f),
				ImVec4(0.278f, 0.271f, 0.267f, 1.00f),
				ImVec4(0.478f, 0.188f, 0.188f, 1.00f),
				ImVec4(0.814f, 0.00f, 0.00f, 1.00f),
				ImVec4(1.00f, 0.00f, 0.00f, 1.00f)
			);
			GImGuiManager::SetImGuiColorProfile(colorProfile);
		}
		break;

		case Havtorn::EEditorColorTheme::HavtornGreen:
		{
			SImGuiColorProfile colorProfile(
				ImVec4(0.11f, 0.11f, 0.11f, 1.00f),
				ImVec4(0.198f, 0.198f, 0.198f, 1.00f),
				ImVec4(0.278f, 0.271f, 0.267f, 1.00f),
				ImVec4(0.355f, 0.478f, 0.188f, 1.00f),
				ImVec4(0.469f, 0.814f, 0.00f, 1.00f),
				ImVec4(0.576f, 1.00f, 0.00f, 1.00f)
			);
			GImGuiManager::SetImGuiColorProfile(colorProfile);
		}
		break;

		case Havtorn::EEditorColorTheme::Count:
		case Havtorn::EEditorColorTheme::DefaultDark:
			ImGui::StyleColorsDark();
			break;
		}

		switch (styleTheme)
		{
		case Havtorn::EEditorStyleTheme::Havtorn:

			GImGuiManager::SetImGuiStyleProfile(SImGuiStyleProfile());
			break;
		case Havtorn::EEditorStyleTheme::Count:
		case Havtorn::EEditorStyleTheme::Default:
			break;
		}
	}

	std::string CEditorManager::GetEditorColorThemeName(const EEditorColorTheme colorTheme)
	{
		switch (colorTheme)
		{
		case Havtorn::EEditorColorTheme::DefaultDark:
			return "ImGui Dark";
		case Havtorn::EEditorColorTheme::HavtornDark:
			return "Havtorn Dark";
		case Havtorn::EEditorColorTheme::HavtornRed:
			return "Havtorn Red";
		case Havtorn::EEditorColorTheme::HavtornGreen:
			return "Havtorn Green";
		case Havtorn::EEditorColorTheme::Count:
			return {};
		}
		return {};
	}

	ImVec4 CEditorManager::GetEditorColorThemeRepColor(const EEditorColorTheme colorTheme)
	{
		switch (colorTheme)
		{
		case Havtorn::EEditorColorTheme::DefaultDark:
			return { 0.11f, 0.16f, 0.55f, 1.00f };
		case Havtorn::EEditorColorTheme::HavtornDark:
			return { 0.478f, 0.361f, 0.188f, 1.00f };
		case Havtorn::EEditorColorTheme::HavtornRed:
			return { 0.478f, 0.188f, 0.188f, 1.00f };
		case Havtorn::EEditorColorTheme::HavtornGreen:
			return { 0.355f, 0.478f, 0.188f, 1.00f };
		case Havtorn::EEditorColorTheme::Count:
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

	ImGui::CViewportWindow* CEditorManager::GetViewportWindow() const
	{
		return reinterpret_cast<ImGui::CViewportWindow*>(Windows[0].get());
	}

	const CRenderManager* CEditorManager::GetRenderManager() const
	{
		return RenderManager;
	}

	const CEditorResourceManager* CEditorManager::GetResourceManager() const
	{
		return ResourceManager;
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

		const Havtorn::SVector2<F32> resolution = GEngine::GetWindowHandler()->GetResolution();

		constexpr F32 viewportAspectRatioInv = (9.0f / 16.0f);
		const F32 viewportPaddingX = ViewportPadding;
		constexpr F32 viewportPaddingY = 0.0f;

		I16 viewportPosX = static_cast<I16>(resolution.X * viewportPaddingX);
		I16 viewportPosY = static_cast<I16>(viewportPaddingY);
		U16 viewportSizeX = static_cast<U16>(resolution.X - (2.0f * static_cast<F32>(viewportPosX)));
		U16 viewportSizeY = static_cast<U16>(static_cast<F32>(viewportSizeX) * viewportAspectRatioInv);

		// NR: This might be windows menu bar height?
		U16 sizeOffsetY = 20;

		EditorLayout.ViewportPosition = { viewportPosX, viewportPosY };
		EditorLayout.ViewportSize = { viewportSizeX, viewportSizeY };
		EditorLayout.AssetBrowserPosition = { viewportPosX, static_cast<I16>(viewportPosY + viewportSizeY) };
		EditorLayout.AssetBrowserSize = { viewportSizeX, static_cast<U16>(resolution.Y - static_cast<F32>(viewportSizeY) - sizeOffsetY) };
		EditorLayout.HierarchyViewPosition = { 0, viewportPosY };
		EditorLayout.HierarchyViewSize = { static_cast<U16>(viewportPosX), static_cast<U16>(resolution.Y - sizeOffsetY) };
		EditorLayout.InspectorPosition = { static_cast<I16>(resolution.X - static_cast<F32>(viewportPosX)), viewportPosY };
		EditorLayout.InspectorSize = { static_cast<U16>(viewportPosX), static_cast<U16>(resolution.Y - sizeOffsetY) };
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
		//Import non-.hva files to .hva
		for (const auto& entry : std::filesystem::recursive_directory_iterator(std::filesystem::path("Assets")))
		{
			if (entry.path().extension() == ".hva" || entry.is_directory())
				continue;

			std::string fileName = entry.path().string();
			std::string extension = entry.path().extension().string();
			for (auto& character : extension)
				character = std::tolower(character, {});

			EAssetType assetType = EAssetType::None;

			// TODO.NR: Make sure this works for animations as well
			if (extension == ".fbx")
				assetType = EAssetType::StaticMesh;
			else if (extension == ".dds" || extension == ".tga")
				assetType = EAssetType::Texture;
			else
				continue;

			ResourceManager->ConvertToHVA(fileName, fileName.substr(0, fileName.find_last_of('/')), assetType);
		}
	}

	void CEditorManager::SetTransformGizmo(const SInputActionPayload payload)
	{
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

	void CEditorManager::ToggleFreeCam(const SInputActionPayload payload)
	{
		IsFreeCamActive = payload.IsHeld;
	}

	ETransformGizmo CEditorManager::GetCurrentGizmo() const
	{
		return CurrentGizmo;
	}

	bool CEditorManager::GetIsFreeCamActive() const
	{
		return IsFreeCamActive;
	}

	std::string CEditorManager::GetFrameRate() const
	{
		std::string frameRateString = "Framerate: ";
		const U32 frameRate = static_cast<U32>(GTime::AverageFrameRate());
		const float frameTime = 1000.0f / frameRate;
		std::string frameTimeString = std::format("{:.2f}", frameTime);

		frameRateString.append(std::to_string(frameRate));
		frameRateString.append(" (");
		frameRateString.append(frameTimeString);
		frameRateString.append(" ms)");

		frameRateString.append(" | CPU: ");
		const U32 frameRateCPU = static_cast<U32>(GTime::AverageFrameRate(ETimerCategory::CPU));
		const float frameTimeCPU = 1000.0f / frameRateCPU;
		std::string frameTimeStringCPU = std::format("{:.2f}", frameTimeCPU);
		frameRateString.append(frameTimeStringCPU);
		frameRateString.append(" ms");
		
		frameRateString.append(" | GPU: ");
		const U32 frameRateGPU = static_cast<U32>(GTime::AverageFrameRate(ETimerCategory::GPU));
		const float frameTimeGPU = 1000.0f / frameRateGPU;
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
