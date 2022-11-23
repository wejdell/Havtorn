// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Havtorn.h"
#include "Input/InputTypes.h"

#include <filesystem>

struct ID3D11Device;
struct ID3D11DeviceContext;

struct ImFontAtlas;
struct ImVec4;

namespace ImGui 
{
	class CWindow;
	class CToggleable;
	class CViewportWindow;
}

namespace Havtorn
{
	struct SEntity;
	class CGraphicsFramework;
	class CWindowHandler;
	class CRenderManager;
	class CEditorResourceManager;
	class CScene;

	enum class EEditorColorTheme
	{
		DefaultDark,
		HavtornDark,
		HavtornRed,
		HavtornGreen,
		Count
	};

	enum class EEditorStyleTheme
	{
		Default,
		Havtorn,
		Count
	};

	// NR: Wrapper for ImGuizmo operations
	enum class ETransformGizmo
	{
		Translate = 7,
		Rotate = 120,
		Scale = 896
	};

	struct SEditorLayout
	{
		SVector2<I16> ViewportPosition		= SVector2<I16>::Zero;
		SVector2<U16> ViewportSize			= SVector2<U16>::Zero;
		SVector2<I16> AssetBrowserPosition	= SVector2<I16>::Zero;
		SVector2<U16> AssetBrowserSize		= SVector2<U16>::Zero;
		SVector2<I16> HierarchyViewPosition	= SVector2<I16>::Zero;
		SVector2<U16> HierarchyViewSize		= SVector2<U16>::Zero;
		SVector2<I16> InspectorPosition		= SVector2<I16>::Zero;
		SVector2<U16> InspectorSize			= SVector2<U16>::Zero;
	};

	struct SEditorAssetRepresentation
	{
		EAssetType AssetType = EAssetType::None;
		std::filesystem::directory_entry DirectoryEntry = {};
		void* TextureRef = nullptr;
		std::string Name = "";
	};

	class CEditorManager
	{
	public:
		__declspec(dllexport) CEditorManager();
		__declspec(dllexport) ~CEditorManager();

		bool __declspec(dllexport) Init(const CGraphicsFramework* framework, const CWindowHandler* windowHandler, CRenderManager* renderManager, CScene* scene);
		void __declspec(dllexport) BeginFrame();
		void __declspec(dllexport) Render();
		void __declspec(dllexport) EndFrame();
		void __declspec(dllexport) DebugWindow();

	public:
		void SetSelectedEntity(Ref<SEntity> entity);
		Ref<SEntity> GetSelectedEntity() const;

		const Ptr<SEditorAssetRepresentation>& GetAssetRepFromDirEntry(const std::filesystem::directory_entry& dirEntry);
		const Ptr<SEditorAssetRepresentation>& GetAssetRepFromName(const std::string& assetName);
		const Ptr<SEditorAssetRepresentation>& GetAssetRepFromImageRef(void* imageRef);

		void CreateAssetRep(const std::filesystem::path& destinationPath);

		void SetEditorTheme(EEditorColorTheme colorTheme = EEditorColorTheme::HavtornDark, EEditorStyleTheme styleTheme = EEditorStyleTheme::Havtorn);
		std::string GetEditorColorThemeName(const EEditorColorTheme colorTheme);
		ImVec4 GetEditorColorThemeRepColor(const EEditorColorTheme colorTheme);
		[[nodiscard]] const SEditorLayout& GetEditorLayout() const;

		[[nodiscard]] ETransformGizmo GetCurrentGizmo() const;
		[[nodiscard]] bool GetIsFreeCamActive() const;

		[[nodiscard]] F32 GetViewportPadding() const;
		void SetViewportPadding(const F32 padding);
	
		ImGui::CViewportWindow* GetViewportWindow() const;

		[[nodiscard]] const CRenderManager* GetRenderManager() const;
		[[nodiscard]] const CEditorResourceManager* GetResourceManager() const;

		void ToggleDebugInfo();
		void ToggleDemo();

	private:
		void InitEditorLayout(); 
		void InitAssetRepresentations();
		void PreProcessAssets();

		void SetTransformGizmo(const SInputActionPayload payload);

		void ToggleFreeCam(const SInputActionPayload payload);

		[[nodiscard]] std::string GetFrameRate() const;
		[[nodiscard]] std::string GetSystemMemory() const;
		[[nodiscard]] std::string GetDrawCalls() const;

	private:
		const CRenderManager* RenderManager = nullptr;
		CEditorResourceManager* ResourceManager = nullptr;

		// TODO.NR: Should be a weak ptr
		Ref<SEntity> SelectedEntity = nullptr;

		std::vector<Ptr<ImGui::CWindow>> Windows = {};
		std::vector<Ptr<ImGui::CToggleable>> MenuElements = {};
		std::vector<Ptr<SEditorAssetRepresentation>> AssetRepresentations = {};

		SEditorLayout EditorLayout;

		ETransformGizmo CurrentGizmo = ETransformGizmo::Translate;

		F32 ViewportPadding = 0.2f;
		bool IsEnabled = true;
		bool IsDebugInfoOpen = true;
		bool IsDemoOpen = false;
		bool IsFreeCamActive = false;
	};
}
