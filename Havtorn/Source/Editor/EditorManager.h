// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <Havtorn.h>
#include <Input/InputTypes.h>

#include <filesystem>

#include <GUI.h>
#include <Graphics/RenderingPrimitives/FullscreenTexture.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Havtorn
{
	struct SEntity;
	struct SColor;
	class CGraphicsFramework;
	class CWindowHandler;
	class CRenderManager;
	class CEditorResourceManager;
	class CScene;
	class CSequencerSystem;
	class CPlatformManager;
	class CWindow;
	class CToggleable;

	enum class EEditorColorTheme
	{
		DefaultDark,
		HavtornDark,
		HavtornRed,
		HavtornGreen,
		Count,
		PlayMode,
		PauseMode
	};

	enum class EEditorStyleTheme
	{
		Default,
		Havtorn,
		Count
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
		CRenderTexture TextureRef;
		// TODO.NW: Make static string, figure out relationship to engine asset
		std::string Name = "";
		bool UsingEditorTexture = false;
	};

	class CEditorManager
	{
	public:
		EDITOR_API CEditorManager();
		EDITOR_API ~CEditorManager();

		bool EDITOR_API Init(CPlatformManager* platformManager, const CGraphicsFramework* framework, CRenderManager* renderManager);
		void EDITOR_API BeginFrame();
		void EDITOR_API Render();
		void EDITOR_API EndFrame();
		void EDITOR_API DebugWindow();

	public:
		void SetCurrentScene(CScene* scene);
		CScene* GetCurrentScene() const;

		void SetSelectedEntity(const SEntity& entity);
		void AddSelectedEntity(const SEntity& entity);
		void RemoveSelectedEntity(const SEntity& entity);
		
		bool IsEntitySelected(const SEntity& entity);
		void ClearSelectedEntities();
		
		const SEntity& GetSelectedEntity() const;
		const SEntity& GetLastSelectedEntity() const;
		std::vector<SEntity> GetSelectedEntities() const;

		const Ptr<SEditorAssetRepresentation>& GetAssetRepFromDirEntry(const std::filesystem::directory_entry& dirEntry) const;
		const Ptr<SEditorAssetRepresentation>& GetAssetRepFromName(const std::string& assetName) const;
		//const Ptr<SEditorAssetRepresentation>& GetAssetRepFromImageRef(void* imageRef) const;
		DirEntryEAssetTypeFunc GetAssetInspectFunction() const;

		void CreateAssetRep(const std::filesystem::path& destinationPath);
		void RemoveAssetRep(const std::filesystem::directory_entry& sourceEntry);

		void OpenAssetTool(SEditorAssetRepresentation* asset);

		void SetEditorTheme(EEditorColorTheme colorTheme = EEditorColorTheme::HavtornDark, EEditorStyleTheme styleTheme = EEditorStyleTheme::Havtorn);
		std::string GetEditorColorThemeName(const EEditorColorTheme colorTheme);
		SColor GetEditorColorThemeRepColor(const EEditorColorTheme colorTheme);
		[[nodiscard]] const SEditorLayout& GetEditorLayout() const;

		[[nodiscard]] ETransformGizmo GetCurrentGizmo() const;
		[[nodiscard]] bool GetIsFreeCamActive() const;
		[[nodiscard]] bool GetIsOverGizmo() const;
		[[nodiscard]] bool GetIsModalOpen() const;

		void SetIsModalOpen(const bool isModalOpen);

		[[nodiscard]] F32 GetViewportPadding() const;
		void SetViewportPadding(const F32 padding);
	
		bool GetIsWorldPlaying() const;

		// AS: We're returning at ' T* const ' In contrast to ' const T* ' 
		// This means that the Pointer itself is Const, meaning the user cannot re-point it to something else.
		template<class TEditorWindowType>
		inline TEditorWindowType* const GetEditorWindow() const;

		[[nodiscard]] CRenderManager* GetRenderManager() const;
		[[nodiscard]] const CEditorResourceManager* GetResourceManager() const;
		[[nodiscard]] CPlatformManager* GetPlatformManager() const;

		void ToggleDebugInfo();
		void ToggleDemo();

		static std::string PreviewMaterial;

	private:
		void InitEditorLayout(); 
		void InitAssetRepresentations();
		void PreProcessAssets();

		void OnInputSetTransformGizmo(const SInputActionPayload payload);
		void OnInputToggleFreeCam(const SInputActionPayload payload);
		void OnInputFocusSelection(const SInputActionPayload payload);
		void OnDeleteEvent(const SInputActionPayload payload);

		void OnResolutionChanged(SVector2<U16> newResolution);
		void OnBeginPlay(CScene* scene);
		void OnPausePlay(CScene* scene);
		void OnStopPlay(CScene* scene);

		[[nodiscard]] std::string GetFrameRate() const;
		[[nodiscard]] std::string GetSystemMemory() const;
		[[nodiscard]] std::string GetDrawCalls() const;

	private:
		CRenderManager* RenderManager = nullptr;
		CPlatformManager* PlatformManager = nullptr;
		CEditorResourceManager* ResourceManager = nullptr;

		CWorld* World = nullptr;
		CScene* CurrentScene = nullptr;
		std::vector<SEntity> SelectedEntities = {};

		// TODO.NR: Figure out why we can't use unique ptrs with these namespaced classes
		std::vector<Ptr<CWindow>> Windows;
		std::vector<Ptr<CToggleable>> MenuElements;
		std::vector<Ptr<SEditorAssetRepresentation>> AssetRepresentations = {};

		// TODO.NR: Save these in .ini file
		SEditorLayout EditorLayout;
		EEditorColorTheme CurrentColorTheme = EEditorColorTheme::HavtornDark;

		EEditorColorTheme CachedColorTheme = EEditorColorTheme::HavtornDark;

		ETransformGizmo CurrentGizmo = ETransformGizmo::Translate;

		F32 ViewportPadding = 0.2f;
		bool IsEnabled = true;
		bool IsDebugInfoOpen = true;
		bool IsDemoOpen = false;
		bool IsFreeCamActive = false;
		bool IsModalOpen = false;
	};

	template<class TEditorWindowType>
	inline TEditorWindowType* const CEditorManager::GetEditorWindow() const
	{
		// TODO.NR: Figure out why we can't use unique ptrs with these namespaced imgui classes
		U64 targetHashCode = typeid(TEditorWindowType).hash_code();
		for (U32 i = 0; i < Windows.size(); i++)
		{
			U64 hashCode = typeid(*(Windows[i])).hash_code();
			if (hashCode == targetHashCode)
				return static_cast<TEditorWindowType*>(Windows[i].get());
		}
		return nullptr;
	}
}
