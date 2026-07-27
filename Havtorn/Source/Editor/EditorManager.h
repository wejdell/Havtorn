// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <Havtorn.h>
#include <Input/InputTypes.h>

#include <filesystem>

#include <GUI.h>
#include <Graphics/RenderingPrimitives/RenderTexture.h>

#include "EditHistory.h"
#include "EditorDeepLinkParser.h"
#include "ComponentView.h"

#include "NodeView.h"
#include "NodeViews/CoreNodeViews.h"

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

	struct SSnappingOption
	{
		SVector Snapping = SVector::Zero;
		std::string Label = "No Snapping";
		auto operator<=>(const SSnappingOption& other) const = default;
	};
	
	enum class EEditorColorTheme
	{
		HavtornDefault,
		HavtornYellow,
		HavtornRed,
		HavtornGreen,
		HavtornDarkBlue,
		HavtornLightBlue,
		HavtornPurple,
		HavtornPink,
		Count
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
		SVector2<I16> DockSpacePosition		= SVector2<I16>::Zero;
		SVector2<U16> DockSpaceSize			= SVector2<U16>::Zero;
		SVector2<I16> HierarchyViewPosition	= SVector2<I16>::Zero;
		SVector2<U16> HierarchyViewSize		= SVector2<U16>::Zero;
		SVector2<I16> InspectorPosition		= SVector2<I16>::Zero;
		SVector2<U16> InspectorSize			= SVector2<U16>::Zero;

		bool ViewportChanged = false;
		bool DockSpaceChanged = false;
		bool HierarchyViewChanged = false;
		bool InspectorChanged = false;

		bool HasLayoutChanged() const
		{
			return ViewportChanged || DockSpaceChanged || HierarchyViewChanged || InspectorChanged;
		}
	};

	struct SEditorAssetRepresentation
	{
		EAssetType AssetType = EAssetType::None;
		std::filesystem::directory_entry DirectoryEntry = {};
		CRenderTexture TextureRef;
		// TODO.NW: Make static string, figure out relationship to engine asset. 
		// We might want to be able to make more shared ptrs of these, so authoring tools can use them by making a shared ptr to the one we're opening.
		std::string Name = "";
		bool UsingEditorTexture = false;
		bool IsSourceWatched = false;
		bool IsBeingNamed = false;
	};
	
	struct SEditorPreferences
	{
		F32 Sensitivity = 0.5f;
		EEditorColorTheme EditorColorTheme = EEditorColorTheme::HavtornDefault;
		EEditorColorTheme PlayColorTheme = EEditorColorTheme::HavtornDefault;
		EEditorColorTheme PauseColorTheme = EEditorColorTheme::HavtornDefault;
	};

	template<typename T>
	concept ComponentViewType = std::derived_from<T, SComponentView>;

	template<typename T>
	concept ComponentType = std::derived_from<T, SComponent>;

	template<typename T>
	concept NodeViewType = std::derived_from<T, SNodeView>;

	template<typename T>
	concept NodeType = std::derived_from<T, HexRune::SNode>;

	class CEditorManager
	{
	public:
		EDITOR_API CEditorManager();
		EDITOR_API ~CEditorManager();

		virtual bool EDITOR_API Init(CPlatformManager* platformManager, CRenderManager* renderManager);
		void EDITOR_API BeginFrame();
		void EDITOR_API Render();
		void EDITOR_API EndFrame();
		void EDITOR_API DebugWindow();

	public:
		void SetCurrentWorkingScene(const I64 sceneIndex);
		CScene* GetCurrentWorkingScene() const;
		std::vector<Ptr<CScene>>& GetScenes() const;
		CScene* GetContainingScene(const SEntity& entity) const;

		const std::map<U64, Ptr<SComponentView>>& GetComponentViewsMap() const;
		const std::vector<Ptr<SComponentView>>& GetComponentViewsVector() const;
		const std::vector<U64> GetComponentDepencies(const U64 componentRuntimeHash) const;

		const std::unordered_map<U64, Ptr<SNodeView>>& GetNodeViewsMap() const;
		const std::vector<Ptr<SNodeView>>& GetNodeViewsVector() const;

		void SetSelectedEntity(const SEntity& entity);
		void AddSelectedEntity(const SEntity& entity);
		void RemoveSelectedEntity(const SEntity& entity);
		
		bool IsEntitySelected(const SEntity& entity);
		void ClearSelectedEntities();
		
		const SEntity& GetSelectedEntity() const;
		const SEntity& GetLastSelectedEntity() const;
		std::vector<SEntity> GetSelectedEntities() const;

		// NW: Packed prefabs can be attached to other entities but their children can't be modified or extended outside of the asset level. Excludes the prefab entity itself.
		bool IsEntityInsidePackedPrefab(const SEntity& entity) const;
		// NW: Excludes the entity going in as parameter itself.
		SEntity GetPackedPrefabParent(const SEntity& entity) const;

		std::string GetEntityFocusLink(const SEntity& entity) const;
		std::string GetCameraFocusLink() const;
		std::string GetAssetFocusLink(SEditorAssetRepresentation* assetRep) const;

		// TODO.NW: I'd much rather figure out how to manage non-owned resources similar to how unreal does it. Those weak ptrs are managed and 
		// reset when things are garbage collected and so can be checked for validity, but by default, c++ weak ptrs must be converted into shared
		// ptrs in order to be used. Can we make our own version?
		void SetSelectedAsset(SEditorAssetRepresentation* asset);
		void AddSelectedAsset(SEditorAssetRepresentation* asset);
		void RemoveSelectedAsset(SEditorAssetRepresentation* asset);
		void SetSelectedFolder(const std::optional<std::filesystem::directory_entry>& folder);

		bool IsAssetSelected(SEditorAssetRepresentation* asset) const;
		void ClearSelectedAssets();

		SEditorAssetRepresentation* GetSelectedAsset() const;
		SEditorAssetRepresentation* GetLastSelectedAsset() const;
		std::vector<SEditorAssetRepresentation*> GetSelectedAssets() const;
		std::optional<std::filesystem::directory_entry> GetSelectedFolder() const;

		const Ptr<SEditorAssetRepresentation>& GetAssetRepFromDirEntry(const std::filesystem::directory_entry& dirEntry) const;
		const Ptr<SEditorAssetRepresentation>& GetAssetRepFromName(const std::string& assetName) const;
		const intptr_t GetTextureResourceFromAssetRep(SEditorAssetRepresentation* assetRepresentation) const;
		DirEntryFunc GetAssetInspectFunction() const;
		DirEntryEAssetTypeFunc GetAssetFilteredInspectFunction() const;

		void CreateAssetRep(const std::filesystem::path& destinationPath);
		void RemoveAssetRep(const std::filesystem::directory_entry& sourceEntry);

		void OpenAssetTool(SEditorAssetRepresentation* asset);

		void FocusEntity(const SEntity& entity);

		void SetEditorTheme(EEditorColorTheme colorTheme = EEditorColorTheme::HavtornYellow, EEditorStyleTheme styleTheme = EEditorStyleTheme::Havtorn, F32 darknessOffset = 1.0f);
		std::string GetEditorColorThemeName(const EEditorColorTheme colorTheme);
		SColor GetEditorColorThemeRepColor(const EEditorColorTheme colorTheme);
		[[nodiscard]] SEditorLayout& GetEditorLayout();

		[[nodiscard]] ETransformGizmo GetCurrentGizmo() const;
		[[nodiscard]] ETransformGizmoSpace GetCurrentGizmoSpace() const;
		[[nodiscard]] const SSnappingOption& GetCurrentGizmoSnapping() const;
		[[nodiscard]] bool GetIsFreeCamActive() const;
		[[nodiscard]] bool GetIsOverGizmo() const;
		[[nodiscard]] bool GetIsModalOpen() const;
		[[nodiscard]] bool GetIsDragCopyActive() const;
		[[nodiscard]] bool GetIsPivotOffsetSet() const;
		[[nodiscard]] bool GetIsPivotMovingActive() const;
		[[nodiscard]] bool GetIsVertexSnappingActive() const;
		[[nodiscard]] bool GetIsGridSnappingActive() const;

		void SetCurrentGizmo(const ETransformGizmo gizmo);
		void SetGizmoSpace(const ETransformGizmoSpace space);
		void SetGizmoSnapping(const SSnappingOption& snapping);
		void SetPivotMoving(const bool active);
		void SetVertexSnapping(const bool active);
		void SetGridSnapping(const bool active);

		void SetIsModalOpen(const bool isModalOpen);

		[[nodiscard]] F32 GetViewportPadding() const;
		void SetViewportPadding(const F32 padding);
		[[nodiscard]] F32 GetEditorSensitivity() const;
		void SetEditorSensitivity(const F32 sensitivity);
		
		void SetEditorModeColorTheme(const EWorldPlayState dedicatedPlayState, const EEditorColorTheme newColorTheme);
	
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
		void TogglePreferences();
		void ToggleGamePreferences();
		void ToggleEditHistory();

		[[nodiscard]] std::string_view GetProjectName() const;

		static std::string PreviewMaterial;
	
		template<typename TNodeView, typename TNode>
		void RegisterDataBindingNodeView(HexRune::SScript* script, U64 dataBindingID)
		{
			const U64 runtimeNodeTypeHash = typeid(TNode).hash_code();
			if (RegisteredNodeViewsMap.contains(runtimeNodeTypeHash))
				return;

			RegisteredNodeViewsMap.emplace(runtimeNodeTypeHash, std::make_unique<TNodeView>(script, dataBindingID));
			RegisteredNodeViewsMap.at(runtimeNodeTypeHash)->RuntimeHash = runtimeNodeTypeHash;

			RegisteredNodeViewsVector.emplace_back(std::make_unique<TNodeView>(script, dataBindingID));
			RegisteredNodeViewsVector.back()->RuntimeHash = runtimeNodeTypeHash;
			std::sort(RegisteredNodeViewsVector.begin(), RegisteredNodeViewsVector.end(), [](const Ptr<SNodeView>& a, const Ptr<SNodeView>& b) { return a->GetSortingPriority() < b->GetSortingPriority(); });
		}

		template<typename TNodeView, typename TNode>
		void RemoveDataBindingNodeView(U64 dataBindingID)
		{
			const U64 runtimeNodeTypeHash = typeid(TNode).hash_code();
			if (!RegisteredNodeViewsMap.contains(runtimeNodeTypeHash))
				return;

			std::erase_if(RegisteredNodeViewsMap, [dataBindingID](const auto& mapItem)
				{
					auto const& [hash, view] = mapItem;

					HexRune::SDataBindingGetNodeEditorContext* getterContext = static_cast<HexRune::SDataBindingGetNodeEditorContext*>(view.get());
					if (getterContext != nullptr)
						return getterContext->DataBindingID == dataBindingID;

					HexRune::SDataBindingSetNodeEditorContext* setterContext = static_cast<HexRune::SDataBindingSetNodeEditorContext*>(view.get());
					if (setterContext != nullptr)
						return setterContext->DataBindingID == dataBindingID;

					return false;
				});

			std::erase_if(RegisteredNodeViewsVector, [dataBindingID](const Ptr<SNodeView>& view)
				{
					HexRune::SDataBindingGetNodeEditorContext* getterContext = static_cast<HexRune::SDataBindingGetNodeEditorContext*>(view.get());
					if (getterContext != nullptr)
						return getterContext->DataBindingID == dataBindingID;

					HexRune::SDataBindingSetNodeEditorContext* setterContext = static_cast<HexRune::SDataBindingSetNodeEditorContext*>(view.get());
					if (setterContext != nullptr)
						return setterContext->DataBindingID == dataBindingID;

					return false;
				});
		}

	protected:
		template<ComponentViewType TComponentView, ComponentType TComponent>
		void RegisterComponentView()
		{
			const U64 runtimeComponentTypeHash = typeid(TComponent).hash_code();
			if (RegisteredComponentViewsMap.contains(runtimeComponentTypeHash))
				return;

			RegisteredComponentViewsMap.emplace(runtimeComponentTypeHash, std::make_unique<TComponentView>());
			RegisteredComponentViewsMap.at(runtimeComponentTypeHash)->RuntimeHash = runtimeComponentTypeHash;

			RegisteredComponentViewsVector.emplace_back(std::make_unique<TComponentView>());
			RegisteredComponentViewsVector.back()->RuntimeHash = runtimeComponentTypeHash;
			std::sort(RegisteredComponentViewsVector.begin(), RegisteredComponentViewsVector.end(), [](const Ptr<SComponentView>& a, const Ptr<SComponentView>& b) { return a->GetSortingPriority() < b->GetSortingPriority(); });
		}

		template<ComponentType TComponent, ComponentType... TComponents>
		void SetupComponentDependencies()
		{
			const U64 runtimeComponentTypeHash = typeid(TComponent).hash_code();
			if (!ComponentDependencies.contains(runtimeComponentTypeHash))
				ComponentDependencies.emplace(runtimeComponentTypeHash, std::vector<U64>{});

			std::vector<U64>& dependencies = ComponentDependencies.at(runtimeComponentTypeHash);
			([&dependencies] { dependencies.push_back(typeid(TComponents).hash_code()); } (), ...);
		}

		template<NodeViewType TNodeView, NodeType TNode>
		void RegisterNodeView()
		{
			const U64 runtimeNodeTypeHash = typeid(TNode).hash_code();
			if (RegisteredNodeViewsMap.contains(runtimeNodeTypeHash))
				return;

			RegisteredNodeViewsMap.emplace(runtimeNodeTypeHash, std::make_unique<TNodeView>());
			RegisteredNodeViewsMap.at(runtimeNodeTypeHash)->RuntimeHash = runtimeNodeTypeHash;

			RegisteredNodeViewsVector.emplace_back(std::make_unique<TNodeView>());
			RegisteredNodeViewsVector.back()->RuntimeHash = runtimeNodeTypeHash;
			std::sort(RegisteredNodeViewsVector.begin(), RegisteredNodeViewsVector.end(), [](const Ptr<SNodeView>& a, const Ptr<SNodeView>& b) { return a->GetSortingPriority() < b->GetSortingPriority(); });
		}	

	private:
		void InitEditorLayout(); 
		void ReinitEditorLayout();
		void InitAssetRepresentations();
		void InitEditorPreferences();

		void OnInputSetTransformGizmo(const SInputActionPayload payload);
		void OnInputToggleFreeCam(const SInputActionPayload payload);
		void OnInputFocusSelection(const SInputActionPayload payload);
		void OnDeleteEvent(const SInputActionPayload payload);
		void OnToggleFullscreen(const SInputActionPayload payload);
		void OnCopyEvent(const SInputActionPayload payload);
		void OnDragCopyEvent(const SInputActionPayload payload);
		void OnEditorActionTreeEvent(const SInputActionPayload payload);
		void OnPlayStateEvent(const SInputActionPayload payload);
		void OnToggleCursor(const SInputActionPayload payload);
		void OnPivotMoving(const SInputActionPayload payload);
		void OnVertexSnapping(const SInputActionPayload payload);
		void OnGridSnapping(const SInputActionPayload payload);

		void OnResolutionChanged(SVector2<U16> newResolution);
		void OnBeginPlay(std::vector<Ptr<CScene>>& scenes);
		void OnPausePlay(std::vector<Ptr<CScene>>& scenes);
		void OnEndPlay(std::vector<Ptr<CScene>>& scenes);

		[[nodiscard]] std::string GetFrameRate() const;
		[[nodiscard]] std::string GetSystemMemory() const;
		[[nodiscard]] std::string GetRenderInfo() const;

	private:
		CRenderManager* RenderManager = nullptr;
		CPlatformManager* PlatformManager = nullptr;
		CEditorResourceManager* ResourceManager = nullptr;

		CWorld* World = nullptr;

		// NW: The current working scene is the one used when dragging entities into the viewport, and currently the one we save when "saving current scene".
		// I see no reason currently we shouldn't minimize usage of this, for the benefit of working in a multi-scene workflow with a bunch of open "containers".
		CScene* CurrentWorkingScene = nullptr;
		
		// Components
		std::map<U64, Ptr<SComponentView>> RegisteredComponentViewsMap;
		std::vector<Ptr<SComponentView>> RegisteredComponentViewsVector;
		std::map<U64, std::vector<U64>> ComponentDependencies;

		// Nodes
		std::unordered_map<U64, Ptr<SNodeView>> RegisteredNodeViewsMap;
		std::vector<Ptr<SNodeView>> RegisteredNodeViewsVector;

		std::vector<SEntity> SelectedEntities = {};

		CEditHistory EditHistory;
		CEditorDeepLinkParser DeepLinkParser;

		std::vector<Ptr<CWindow>> Windows;
		std::vector<Ptr<CToggleable>> MenuElements;
		std::vector<Ptr<SEditorAssetRepresentation>> AssetRepresentations = {};
		
		std::vector<SEditorAssetRepresentation*> SelectedAssets = {};
		std::optional<std::filesystem::directory_entry> SelectedFolder;

		// TODO.NR: Save these in .ini file
		SEditorLayout EditorLayout;

		ETransformGizmo CurrentGizmo = ETransformGizmo::Translate;
		ETransformGizmoSpace CurrentGizmoSpace = ETransformGizmoSpace::World;
		SSnappingOption CurrentGizmoSnapping = {};
		SEditorPreferences EditorPreferences;
		CJsonDocument EditorPreferencesDocument;
		
		F32 ViewportPadding = 0.2f;
		bool IsEnabled = true;
		bool IsDebugInfoOpen = true;
		bool IsDemoOpen = false;
		bool IsPreferencesOpen = false;
		bool IsGamePreferencesOpen = false;
		bool IsEditHistoryOpen = false;
		bool IsFreeCamActive = false;
		bool IsModalOpen = false;
		bool IsFullscreen = false;
		bool IsDragCopyActive = false;
		bool IsPivotOffsetSet = false;
		bool IsPivotMovingActive = false;
		bool IsVertexSnappingActive = false;
		bool IsGridSnappingActive = false;
		// TODO.NW: Think about whether the above states should be exclusive, 
		// in which case an enum representation for all of them would be nicer

		std::string EntityCopyBuffer;

		inline static const std::string DefaultEditorSettingsPath = "Config/EditorPreferences.json";
		inline static const std::string UserEditorSettingsPath = "Config/EditorPreferences.user.json";

		std::string ProjectName = "Project Name";
		
		inline static const std::string EditorColorThemeKey = "Editor Color Theme";
		inline static const std::string PauseColorThemeKey = "Pause Color Theme";
		inline static const std::string PlayColorThemeKey = "Play Color Theme";
		
		const F32 DarknessOffsetStoppedTheme = 1.0f;
		const F32 DarknessOffsetPlayTheme = 0.6f;
		const F32 DarknessOffsetPausedTheme = 0.8f;
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
