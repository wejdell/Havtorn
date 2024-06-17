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
}

namespace Havtorn
{
	struct SEntity;
	class CGraphicsFramework;
	class CWindowHandler;
	class CRenderManager;
	class CEditorResourceManager;
	class CScene;
	class CSequencerSystem;

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

		bool __declspec(dllexport) Init(const CGraphicsFramework* framework, const CWindowHandler* windowHandler, CRenderManager* renderManager, CSequencerSystem* sequencerSystem);
		void __declspec(dllexport) BeginFrame();
		void __declspec(dllexport) Render();
		void __declspec(dllexport) EndFrame();
		void __declspec(dllexport) DebugWindow();

	public:
		void SetCurrentScene(CScene* scene);
		CScene* GetCurrentScene() const;

		void SetSelectedEntity(SEntity* entity);
		SEntity* GetSelectedEntity() const;

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
	
		// AS: We're returning at ' T* const ' In contrast to ' const T* ' 
		// This means that the Pointer itself is Const, meaning the user cannot re-point it to something else.
		template<class TEditorWindowType>
		inline TEditorWindowType* const GetEditorWindow() const;

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

		CScene* CurrentScene = nullptr;

		// TODO.NR: Should be a weak ptr
		SEntity* SelectedEntity = nullptr;

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

	template<class TEditorWindowType>
	inline TEditorWindowType* const CEditorManager::GetEditorWindow() const
	{
		U64 targetHashCode = typeid(TEditorWindowType).hash_code();
		for (U32 i = 0; i < Windows.size(); i++)
		{
			U64 hashCode = typeid(*Windows[i].get()).hash_code();
			if (hashCode == targetHashCode)
				return static_cast<TEditorWindowType*>(Windows[i].get());
		}
		return nullptr;
	}
}
