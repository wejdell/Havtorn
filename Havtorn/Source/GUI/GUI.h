// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include <string>
#include <memory>
#include <wtypes.h>
#include <vector>
#include <filesystem>
#include <functional>

#include <Core.h>
#include <CoreTypes.h>
#include <HavtornString.h>
#include <Color.h>
#include <MathTypes/Vector.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Havtorn
{
	struct SMatrix;
	class CPlatformManager;

	enum class GUI_API EWindowFlag
	{
		NoTitleBar = BIT(0),
		NoResize = BIT(1),
		NoMove = BIT(2),
		NoScrollbar = BIT(3),
		NoCollapse = BIT(5),
		AlwaysAutoResize = BIT(6),
		NoBackground = BIT(7),
		HorizontalScollbar = BIT(11),
		NoBringToFrontOnFocus = BIT(13),
	};

	enum class GUI_API EChildFlag
	{
		Borders = BIT(0),
		ResizeX = BIT(2),
		ResizeY = BIT(3),
		NavFlattened = BIT(8),
	};

	enum class GUI_API EDockNodeFlag
	{
		None = 0,
		KeepAliveOnly = BIT(0),   //       // Don't display the dockspace node but keep it alive. Windows docked into this dockspace node won't be undocked.
		NoDockingOverCentralNode = BIT(2),   //       // Disable docking over the Central Node, which will be always kept empty.
		PassthruCentralNode = BIT(3), //       // Enable passthru dockspace: 1) DockSpace() will render a ImGuiCol_WindowBg background covering everything excepted the Central Node when empty. Meaning the host window should probably use SetNextWindowBgAlpha(0.0f) prior to Begin() when using this. 2) When Central Node is empty: let inputs pass-through + won't display a DockingEmptyBg background. See demo for details.  
		NoDockingSplit = BIT(4),   //       // Disable other windows/nodes from splitting this node.
		NoResize = BIT(5),   // Saved // Disable resizing node using the splitter/separators. Useful with programmatically setup dockspaces.
		AutoHideTabBar = BIT(6),   //       // Tab bar will automatically hide when there is a single window in the dock node.
		NoUndocking = BIT(7),
		DockSpace = BIT(10)
	};

	enum class GUI_API EDragDropFlag
	{
		SourceNoPreviewToolTip = BIT(0),
		SourceNoDisableHover = BIT(1),
		SourceNoHoldToOpenOthers = BIT(2),
		SourceExtern = BIT(4),
		AcceptBeforeDelivery = BIT(10),
		AcceptNoDrawDefaultRect = BIT(11),
		AcceptNopreviewTooltip = BIT(12)
	};

	enum class GUI_API ESelectableFlag
	{
		NoAutoClosePopups = BIT(0),
		AllowDoubleClick = BIT(2),
		Disabled = BIT(3),
		AllowOverlap = BIT(4),
		Highlight = BIT(5)
	};

	enum class GUI_API ETreeNodeFlag
	{
		NoTreePushOnOpen = BIT(3),
		Leaf = BIT(8),
		Bullet = BIT(9),
	};

	enum class GUI_API EWindowCondition
	{
		None = 0,				// No condition (always set the variable), same as _Always
		Always = BIT(0),		// No condition (always set the variable), same as _None
		Once = BIT(1),			// Set the variable once per runtime session (only the first call will succeed)
		FirstUseEver = BIT(2),  // Set the variable if the object/window has no persistently saved data (no entry in .ini file)
		Appearing = BIT(3),
	};

	enum class GUI_API EDragMode
	{
		None = 0,
		Logarithmic = BIT(5),
	};

	enum class GUI_API EGUIDirection
	{
		None = -1,
		Left = 0,
		Right = 1,
		Up = 2,
		Down = 3,
		Count,
	};

	enum class GUI_API EStyleVar
	{
		WindowPadding = 2,
		WindowRounding = 3,
		WindowBorderSize = 4,
		FramePadding = 11,
		ItemSpacing = 14,
	};

	enum class GUI_API EStyleColor
	{
		Text,
		TextDisabled,
		WindowBg,              // Background of normal windows
		ChildBg,               // Background of child windows
		PopupBg,               // Background of popups, menus, tooltips windows
		Border,
		BorderShadow,
		FrameBg,               // Background of checkbox, radio button, plot, slider, text input
		FrameBgHovered,
		FrameBgActive,
		TitleBg,               // Title bar
		TitleBgActive,         // Title bar when focused
		TitleBgCollapsed,      // Title bar when collapsed
		MenuBarBg,
		ScrollbarBg,
		ScrollbarGrab,
		ScrollbarGrabHovered,
		ScrollbarGrabActive,
		CheckMark,             // Checkbox tick and RadioButton circle
		SliderGrab,
		SliderGrabActive,
		Button,
		ButtonHovered,
		ButtonActive,
		Header,                // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
		HeaderHovered,
		HeaderActive,
		Separator,
		SeparatorHovered,
		SeparatorActive,
		ResizeGrip,            // Resize grip in lower-right and lower-left corners of windows.
		ResizeGripHovered,
		ResizeGripActive,
		TabHovered,            // Tab background, when hovered
		Tab,                   // Tab background, when tab-bar is focused & tab is unselected
		TabSelected,           // Tab background, when tab-bar is focused & tab is selected
		TabSelectedOverline,   // Tab horizontal overline, when tab-bar is focused & tab is selected
		TabDimmed,             // Tab background, when tab-bar is unfocused & tab is unselected
		TabDimmedSelected,     // Tab background, when tab-bar is unfocused & tab is selected
		TabDimmedSelectedOverline,//..horizontal overline, when tab-bar is unfocused & tab is selected
		DockingPreview,        // Preview overlay color when about to docking something
		DockingEmptyBg,        // Background color for empty node (e.g. CentralNode with no window docked into it)
		PlotLines,
		PlotLinesHovered,
		PlotHistogram,
		PlotHistogramHovered,
		TableHeaderBg,         // Table header background
		TableBorderStrong,     // Table outer and header borders (prefer using Alpha=1.0 here)
		TableBorderLight,      // Table inner borders (prefer using Alpha=1.0 here)
		TableRowBg,            // Table row background (even rows)
		TableRowBgAlt,         // Table row background (odd rows)
		TextLink,              // Hyperlink color
		TextSelectedBg,
		DragDropTarget,        // Rectangle highlighting a drop target
		NavHighlight,          // Gamepad/keyboard: current highlighted item
		NavWindowingHighlight, // Highlight window when using CTRL+TAB
		NavWindowingDimBg,     // Darken/colorize entire screen behind the CTRL+TAB window list, when active
		ModalWindowDimBg,      // Darken/colorize entire screen behind a modal window, when one is active
		Count,
	};

	enum class GUI_API ETransformGizmo
	{
		Translate = 7,
		Rotate = 120,
		Scale = 896
	};

	enum class GUI_API ETransformGizmoSpace
	{
		Local,
		World
	};

	// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]". Rewrite from ImGui
	struct SGuiTextFilter
	{
		GUI_API           SGuiTextFilter(const char* default_filter = "");
		GUI_API bool      Draw(const char* label = "Filter (inc,-exc)", F32 width = 0.0f);  // Helper calling InputText+Build
		GUI_API bool      PassFilter(const char* text, const char* text_end = NULL) const;
		GUI_API void      Build();
		void                Clear() { InputBuf[0] = 0; Build(); }
		bool                IsActive() const { return !Filters.empty(); }

		static inline bool      CharIsBlankA(char c) { return c == ' ' || c == '\t'; }

		const char* Stristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end) const;  // Find a substring in a string range.
		void Strncpy(char* dst, const char* src, size_t count) const;

		// [Internal]
		struct SGuiTextRange
		{
			const char* b;
			const char* e;

			SGuiTextRange() { b = e = NULL; }
			SGuiTextRange(const char* _b, const char* _e) { b = _b; e = _e; }
			bool            empty() const { return b == e; }
			GUI_API void  split(char separator, std::vector<SGuiTextRange>* out) const;
		};
		char                    InputBuf[256];
		std::vector<SGuiTextRange> Filters;
		int                     CountGrep;
	};

	struct SGuiPayload
	{
		void* Data = nullptr;
		U64 Size = 0;

		U32 SourceID = 0;
		U32 SourceParentID = 0;
		I32 DataFrameCount = 0;
		std::string IDTag = "";
		bool IsPreview = false;
		bool IsDelivery = false;

		bool IsID(const std::string& id) { return IDTag == id; }
	};

	// Havtorn Default == Struct Default Values
	struct GUI_API SGuiColorProfile
	{
		SColor BackgroundBase = SColor(0.11f, 0.11f, 0.11f, 1.00f);
		SColor BackgroundMid = SColor(0.198f, 0.198f, 0.198f, 1.00f);
		SColor ElementBackground = SColor(0.278f, 0.271f, 0.267f, 1.00f);
		SColor ElementHovered = SColor(0.478f, 0.361f, 0.188f, 1.00f);
		SColor ElementActive = SColor(0.814f, 0.532f, 0.00f, 1.00f);
		SColor ElementHighlight = SColor(1.00f, 0.659f, 0.00f, 1.00f);
		SColor Text = SColor(0.92f, 0.92f, 0.92f, 1.00f);
		SColor TextDisabled = SColor(0.44f, 0.44f, 0.44f, 1.00f);
		SColor WindowBg = BackgroundMid;
		SColor ChildBg = SColor(0.00f, 0.00f, 0.00f, 0.00f);
		SColor PopupBg = SColor(0.13f, 0.13f, 0.13f, 0.94f);
		SColor Border = SColor(0.05f, 0.05f, 0.04f, 0.94f);
		SColor BorderShadow = SColor(0.00f, 0.00f, 0.00f, 0.00f);
		SColor FrameBg = ElementBackground;
		SColor FrameBgHovered = ElementHovered;
		SColor FrameBgActive = ElementBackground;
		SColor TitleBg = ElementHovered;
		SColor TitleBgActive = ElementActive;
		SColor TitleBgCollapsed = SColor(0.00f, 0.00f, 0.00f, 0.51f);
		SColor MenuBarBg = BackgroundBase;
		SColor ScrollbarBg = BackgroundBase;
		SColor ScrollbarGrab = ElementBackground;
		SColor ScrollbarGrabHovered = ElementHovered;
		SColor ScrollbarGrabActive = ElementActive;
		SColor CheckMark = ElementActive;
		SColor SliderGrab = ElementActive;
		SColor SliderGrabActive = ElementActive;
		SColor Button = ElementBackground;
		SColor ButtonHovered = ElementHovered;
		SColor ButtonActive = ElementActive;
		SColor Header = ElementBackground;
		SColor HeaderHovered = ElementHovered;
		SColor HeaderActive = ElementActive;
		SColor Separator = ElementBackground;
		SColor SeparatorHovered = ElementHovered;
		SColor SeparatorActive = ElementActive;
		SColor ResizeGrip = BackgroundBase;
		SColor ResizeGripHovered = ElementHovered;
		SColor ResizeGripActive = ElementActive;
		SColor Tab = ElementBackground;
		SColor TabHovered = ElementHovered;
		SColor TabSelected = BackgroundMid;
		SColor TabDimmed = ElementBackground;
		SColor TabDimmedSelected = BackgroundMid;
		SColor PlotLines = SColor(0.61f, 0.61f, 0.61f, 1.00f);
		SColor PlotLinesHovered = ElementHighlight;
		SColor PlotHistogram = ElementHighlight;
		SColor PlotHistogramHovered = ElementActive;
		SColor TextSelectedBg = SColor(0.26f, 0.59f, 0.98f, 0.35f);
		SColor DragDropTarget = ElementHighlight;
		SColor NavHighlight = SColor(0.26f, 0.59f, 0.98f, 1.00f);
		SColor NavWindowHighlight = SColor(1.00f, 1.00f, 1.00f, 0.70f);
		SColor NavWindowDimBg = SColor(0.80f, 0.80f, 0.80f, 0.20f);
		SColor ModalWindowDimBg = SColor(0.80f, 0.80f, 0.80f, 0.35f);

		SGuiColorProfile() = default;

		// Constructs profile in the same way as default, params are reused for various elements.
		SGuiColorProfile(const SColor& backgroundBase, const SColor& backgroundMid, const SColor& elementBackground, const SColor& elementHovered, const SColor& elementActive, const SColor& elementHightlight)
		{
			BackgroundBase = backgroundBase;
			BackgroundMid = backgroundMid;
			ElementBackground = elementBackground;
			ElementHovered = elementHovered;
			ElementActive = elementActive;
			ElementHighlight = elementHightlight;
			WindowBg = BackgroundMid;
			FrameBg = ElementBackground;
			FrameBgHovered = ElementHovered;
			FrameBgActive = ElementBackground;
			TitleBg = ElementHovered;
			TitleBgActive = ElementActive;
			MenuBarBg = BackgroundBase;
			ScrollbarBg = BackgroundBase;
			ScrollbarGrab = ElementBackground;
			ScrollbarGrabHovered = ElementHovered;
			ScrollbarGrabActive = ElementActive;
			CheckMark = ElementActive;
			SliderGrab = ElementActive;
			SliderGrabActive = ElementActive;
			Button = ElementBackground;
			ButtonHovered = ElementHovered;
			ButtonActive = ElementActive;
			Header = ElementBackground;
			HeaderHovered = ElementHovered;
			HeaderActive = ElementActive;
			Separator = ElementBackground;
			SeparatorHovered = ElementHovered;
			SeparatorActive = ElementActive;
			ResizeGrip = BackgroundBase;
			ResizeGripHovered = ElementHovered;
			ResizeGripActive = ElementActive;
			Tab = ElementBackground;
			TabHovered = ElementHovered;
			TabSelected = BackgroundMid;
			TabDimmed = ElementBackground;
			TabDimmedSelected = BackgroundMid;
			PlotLinesHovered = ElementHighlight;
			PlotHistogram = ElementHighlight;
			PlotHistogramHovered = ElementActive;
			DragDropTarget = ElementHighlight;
		}
	};

	// Havtorn Default == Struct Default Values
	struct SGuiStyleProfile
	{
		SVector2<F32> WindowPadding = SVector2<F32>(8.00f, 8.00f);
		SVector2<F32> FramePadding = SVector2<F32>(5.00f, 2.00f);
		SVector2<F32> CellPadding = SVector2<F32>(6.00f, 6.00f);
		SVector2<F32> ItemSpacing = SVector2<F32>(6.00f, 6.00f);
		SVector2<F32> ItemInnerSpacing = SVector2<F32>(6.00f, 6.00f);
		SVector2<F32> TouchExtraPadding = SVector2<F32>(0.00f, 0.00f);
		F32 IndentSpacing = 25.f;
		F32 ScrollbarSize = 15.f;
		F32 GrabMinSize = 10.f;
		F32 WindowBorderSize = 1.f;
		F32 ChildBorderSize = 1.f;
		F32 PopupBorderSize = 1.f;
		F32 FrameBorderSize = 1.f;
		F32 TabBorderSize = 1.f;
		F32 WindowRounding = 1.f;
		F32 ChildRounding = 1.f;
		F32 FrameRounding = 1.f;
		F32 PopupRounding = 1.f;
		F32 ScrollbarRounding = 1.f;
		F32 GrabRounding = 1.f;
		F32 LogSliderDeadzone = 4.f;
		F32 TabRounding = 1.f;
	};

	struct SAssetInspectionData
	{
		SAssetInspectionData(const std::string& name, const intptr_t textureRef)
			: Name(name)
			, TextureRef(textureRef)
		{}
		std::string Name = "";
		intptr_t TextureRef = 0;
	};

	enum class EAssetPickerState
	{
		Inactive,
		Active,
		AssetPicked,
		Cancelled,
	};

	struct SAssetPickResult
	{
		SAssetPickResult() = default;
		SAssetPickResult(EAssetPickerState state)
			: State(state)
		{}
		SAssetPickResult(const std::filesystem::directory_entry& entry)
			: State(EAssetPickerState::AssetPicked)
			, PickedEntry(entry)
		{}
		EAssetPickerState State = EAssetPickerState::Inactive;
		std::filesystem::directory_entry PickedEntry;
	};

	struct SRenderAssetCardResult
	{
		SRenderAssetCardResult() = default;
		
		bool IsDoubleClicked = false;
		bool IsHovered = false;
	};

	class GUI_API GUI
	{
	public:
		GUI();
		~GUI();
		void InitGUI(CPlatformManager* platformManager, ID3D11Device* device, ID3D11DeviceContext* context);

		void BeginFrame();
		void EndFrame();
		void WindowsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static void SetGUIContext();

	public:
		static const F32 SliderSpeed;
		static const F32 TexturePreviewSizeX;
		static const F32 TexturePreviewSizeY;
		static const F32 DummySizeX;
		static const F32 DummySizeY;
		static const F32 ThumbnailSizeX;
		static const F32 ThumbnailSizeY;
		static const F32 ThumbnailPadding;
		static const F32 PanelWidth;

		static const char* SelectTextureModalName;

		static bool TryOpenComponentView(const std::string& componentViewName);

	public:
		static bool Begin(const char* name, bool* open = 0, const std::vector<EWindowFlag>& flags = {});
		static void End();

		static void Text(const char* fmt, ...);
		static void TextWrapped(const char* fmt, ...);
		static void TextDisabled(const char* fmt, ...);
		static void TextUnformatted(const char* text);
		static void InputText(const char* label, CHavtornStaticString<255>* customString);

		static void SetTooltip(const char* fmt, ...);

		static bool InputFloat(const char* label, F32& value, F32 step = 0.0f, F32 stepFast = 0.0f, const char* format = "%.3f");
		static bool DragFloat(const char* label, F32& value, F32 speed = 0.1f, F32 min = 0.0f, F32 max = 0.0f, const char* format = "%.3f", EDragMode dragMode = EDragMode::None);
		static bool DragFloat2(const char* label, SVector2<F32>& value, F32 speed = 0.1f, F32 min = 0.0f, F32 max = 0.0f, const char* format = "%.3f", EDragMode dragMode = EDragMode::None);
		static bool DragFloat3(const char* label, SVector& value, F32 speed = 0.1f, F32 min = 0.0f, F32 max = 0.0f, const char* format = "%.3f", EDragMode dragMode = EDragMode::None);
		static bool DragFloat4(const char* label, SVector4& value, F32 speed = 0.1f, F32 min = 0.0f, F32 max = 0.0f, const char* format = "%.3f", EDragMode dragMode = EDragMode::None);
		static bool SliderFloat(const char* label, F32& value, F32 min = 0.0f, F32 max = 0.0f, const char* format = "%.3f", EDragMode dragMode = EDragMode::None);

		// TODO.NW: Make unsigned variants
		static bool DragInt2(const char* label, SVector2<I32>& value, F32 speed = 1.0f, int min = 0, int max = 0, const char* format = "%d", EDragMode dragMode = EDragMode::None);
		static bool SliderInt(const char* label, I32& value, int min = 0, int max = 1, const char* format = "%d", EDragMode dragMode = EDragMode::None);

		template<typename T>
		static bool SliderEnum(const char* label, T& value, const std::vector<std::string>& valueLabels)
		{
			I32 index = static_cast<I32>(value);
			I32 numberOfValues = STATIC_I32(valueLabels.size());
			std::string valueLabel = numberOfValues > index ? valueLabels[index] : "No Label";
			const bool returnValue = GUI::SliderInt(label, index, 0, numberOfValues > 0 ? numberOfValues - 1 : 1, valueLabel.c_str());
			value = static_cast<T>(index);
			return returnValue;
		}

		static bool ColorPicker3(const char* label, SColor& value);
		static bool ColorPicker4(const char* label, SColor& value);

		static void PushID(const char* label);
		static void PushID(I32 intID);
		static void PopID();
		static I32 GetID(const char* label);

		static bool BeginMainMenuBar();
		static void EndMainMenuBar();

		static bool BeginMenu(const char* label, bool enabled = true);
		static void EndMenu();

		static bool MenuItem(const char* label, const char* shortcut = (const char*)0, const bool selected = false, const bool enabled = true);

		static bool BeginPopup(const char* label);
		static void EndPopup();

		static bool BeginPopupModal(const char* label, bool* open = 0, const std::vector<EWindowFlag>& flags = {});
		static void CloseCurrentPopup();

		static void BeginGroup();
		static void EndGroup();

		static bool BeginChild(const char* label, const SVector2<F32>& size = SVector2<F32>(0.0f), const std::vector<EChildFlag>& childFlags = {}, const std::vector<EWindowFlag>& windowFlags = {});
		static void EndChild();

		static bool BeginDragDropSource(const std::vector<EDragDropFlag>& flags = {});
		static SGuiPayload GetDragDropPayload();
		static bool SetDragDropPayload(const char* type, const void* data, U64 dataSize);
		static void EndDragDropSource();

		static bool BeginDragDropTarget();
		static bool IsDragDropPayloadBeingAccepted();
		static SGuiPayload AcceptDragDropPayload(const char* type, const std::vector<EDragDropFlag>& flags = {});
		static void EndDragDropTarget();

		static bool BeginPopupContextWindow();

		static void OpenPopup(const char* label);

		static bool BeginTable(const char* label, const I32 columns);
		static void TableNextRow();
		static void TableNextColumn();
		static void EndTable();

		static bool TreeNode(const char* label);
		static bool TreeNodeEx(const char* label, const std::vector<ETreeNodeFlag>& treeNodeFlags);
		static void TreePop();

		static bool ArrowButton(const char* label, const EGUIDirection direction);
		static bool Button(const char* label, const SVector2<F32>& size = SVector2<F32>(0.0f));
		static bool SmallButton(const char* label);
		static bool RadioButton(const char* label, bool active);
		static bool ImageButton(const char* label, intptr_t image, const SVector2<F32>& size = SVector2<F32>(0.0f), const SVector2<F32>& uv0 = SVector2<F32>(0.0f), const SVector2<F32>& uv1 = SVector2<F32>(1.0f), const SColor& backgroundColor = SColor(0.0f, 0.0f, 0.0f, 0.0f), const SColor& tintColor = SColor::White);
		static bool Checkbox(const char* label, bool& value);

		static SAssetPickResult AssetPicker(const char* label, const char* modalLabel, intptr_t image, const std::string& directory, I32 columns, const std::function<SAssetInspectionData(std::filesystem::directory_entry)>& assetInspector);
		static SRenderAssetCardResult RenderAssetCard(const char* label, const intptr_t& thumbnailID, const char* typeName, const SColor& color, void* dragDropPayloadToSet, U64 payLoadSize);

		static bool Selectable(const char* label, const bool selected = false, const std::vector<ESelectableFlag>& flags = {}, const SVector2<F32>& size = SVector2<F32>(0.0f));

		static void Image(intptr_t image, const SVector2<F32>& size, const SVector2<F32>& uv0 = SVector2<F32>(0.0f), const SVector2<F32>& uv1 = SVector2<F32>(1.0f), const SColor& tintColor = SColor::White, const SColor& borderColor = SColor(0.0f, 0.0f, 0.0f, 0.0f));

		static void Separator();
		static void Dummy(const SVector2<F32>& size);
		static void SameLine(const F32 offsetFromX = 0.0f, const F32 spacing = -1.0f);
		static bool IsItemClicked();
		static bool IsItemHovered();

		static SVector2<F32> GetCursorPos();
		static void SetCursorPos(const SVector2<F32>& cursorPos);
		static F32 GetCursorPosX();
		static void SetCursorPosX(const F32 cursorPosX);
		static F32 GetScrollY();
		static F32 GetScrollMaxY();
		static void SetScrollHereY(const F32 centerYRatio);
		static SVector2<F32> CalculateTextSize(const char* text);

		static void SetItemDefaultFocus();
		static void SetKeyboardFocusHere(const I32 offset = 0);

		static void PushStyleVar(const EStyleVar styleVar, const SVector2<F32>& value);
		static void PushStyleVar(const EStyleVar styleVar, const F32 value);
		static void PopStyleVar(const I32 count = 1);
		static SVector2<F32> GetStyleVar(const EStyleVar styleVar);

		static std::vector<SColor> GetStyleColors();
		static void PushStyleColor(const EStyleColor styleColor, const SColor& color);
		static void PopStyleColor();

		static void DecomposeMatrixToComponents(const SMatrix& matrix, SVector& translation, SVector& rotation, SVector& scale);
		static void RecomposeMatrixFromComponents(SMatrix& matrix, const SVector& translation, const SVector& rotation, const SVector& scale);
		static void SetOrthographic(const bool enabled);
		
		static bool IsOverGizmo();
		static bool IsDoubleClick();

		static F32 GetTextLineHeight();
		static SVector2<F32> GetCursorScreenPos();

		static SVector2<F32> GetViewportWorkPos();
		static SVector2<F32> GetViewportCenter();

		static SVector2<F32> GetWindowContentRegionMin();
		static SVector2<F32> GetWindowContentRegionMax();

		static SVector2<F32> GetContentRegionAvail();

		static F32 GetFrameHeightWithSpacing();

		static void SetNextWindowPos(const SVector2<F32>& pos, const EWindowCondition condition = EWindowCondition::None, const SVector2<F32>& pivot = SVector2<F32>(0.0f));
		static void SetNextWindowSize(const SVector2<F32>& size);
		static void SetRect(const SVector2<F32>& position, const SVector2<F32>& dimensions);
		static void SetGizmoDrawList();

		static SVector2<F32> GetCurrentWindowSize();

		static void AddRectFilled(const SVector2<F32>& cursorPos, const SVector2<F32>& size, const SColor& color);

		static void SetGuiColorProfile(const SGuiColorProfile& profile);
		static void SetGuiStyleProfile(const SGuiStyleProfile& profile);

		static void GizmoManipulate(const F32* view, const F32* projection, ETransformGizmo operation, ETransformGizmoSpace mode, F32* matrix, F32* deltaMatrix = 0, const F32* snap = 0, const F32* localBounds = 0, const F32* boundsSnap = 0);
		static void ViewManipulate(F32* view, const F32 length, const SVector2<F32>& position, const SVector2<F32>& size, const SColor& color);

		static bool IsDockingEnabled();

		static void DockSpace(const U32 id, const SVector2<F32>& size, const EDockNodeFlag dockNodeFlag);
		static void DockBuilderAddNode(U32 id, const std::vector<EDockNodeFlag>& flags);
		static void DockBuilderRemoveNode(U32 id);
		static void DockBuilderSetNodeSize(U32 id, const SVector2<F32>& size);
		static void DockBuilderDockWindow(const char* label, U32 id);
		static void DockBuilderFinish(U32 id);

		static void LogToClipboard();
		static void LogFinish();

		static void MemFree(void* ptr);

		static void ShowDemoWindow(bool* open);

	private:
		class ImGuiImpl;
		ImGuiImpl* Impl;
		static GUI* Instance;
	};
}