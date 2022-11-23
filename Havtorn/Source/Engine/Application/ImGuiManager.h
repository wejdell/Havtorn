// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core/imgui.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Havtorn
{
	// Havtorn Default == Struct Default Values
	struct SImGuiColorProfile
	{
		ImVec4 BackgroundBase		= ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		ImVec4 BackgroundMid		= ImVec4(0.198f, 0.198f, 0.198f, 1.00f);
		ImVec4 ElementBackground	= ImVec4(0.278f, 0.271f, 0.267f, 1.00f);
		ImVec4 ElementHovered		= ImVec4(0.478f, 0.361f, 0.188f, 1.00f);
		ImVec4 ElementActive		= ImVec4(0.814f, 0.532f, 0.00f, 1.00f);
		ImVec4 ElementHighlight		= ImVec4(1.00f, 0.659f, 0.00f, 1.00f);
		ImVec4 Text					= ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
		ImVec4 TextDisabled			= ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
		ImVec4 WindowBg				= BackgroundMid;
		ImVec4 ChildBg				= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		ImVec4 PopupBg				= ImVec4(0.13f, 0.13f, 0.13f, 0.94f);
		ImVec4 Border				= ImVec4(0.05f, 0.05f, 0.04f, 0.94f);
		ImVec4 BorderShadow			= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		ImVec4 FrameBg				= ElementBackground;
		ImVec4 FrameBgHovered		= ElementHovered;
		ImVec4 FrameBgActive		= ElementBackground;
		ImVec4 TitleBg				= ElementHovered;
		ImVec4 TitleBgActive		= ElementActive;
		ImVec4 TitleBgCollapsed		= ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		ImVec4 MenuBarBg			= BackgroundBase;
		ImVec4 ScrollbarBg			= BackgroundBase;
		ImVec4 ScrollbarGrab		= ElementBackground;
		ImVec4 ScrollbarGrabHovered = ElementHovered;
		ImVec4 ScrollbarGrabActive	= ElementActive;
		ImVec4 CheckMark			= ElementActive;
		ImVec4 SliderGrab			= ElementActive;
		ImVec4 SliderGrabActive		= ElementActive;
		ImVec4 Button				= ElementBackground;
		ImVec4 ButtonHovered		= ElementHovered;
		ImVec4 ButtonActive			= ElementActive;
		ImVec4 Header				= ElementBackground;
		ImVec4 HeaderHovered		= ElementHovered;
		ImVec4 HeaderActive			= ElementActive;
		ImVec4 Separator			= ElementBackground;
		ImVec4 SeparatorHovered		= ElementHovered;
		ImVec4 SeparatorActive		= ElementActive;
		ImVec4 ResizeGrip			= BackgroundBase;
		ImVec4 ResizeGripHovered	= ElementHovered;
		ImVec4 ResizeGripActive		= ElementActive;
		ImVec4 Tab					= ElementBackground;
		ImVec4 TabHovered			= ElementHovered;
		ImVec4 TabActive			= ElementActive;
		ImVec4 TabUnfocused			= ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		ImVec4 TabUnfocusedActive	= ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
		ImVec4 PlotLines			= ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		ImVec4 PlotLinesHovered		= ElementHighlight;
		ImVec4 PlotHistogram		= ElementHighlight;
		ImVec4 PlotHistogramHovered = ElementActive;
		ImVec4 TextSelectedBg		= ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		ImVec4 DragDropTarget		= ElementHighlight;
		ImVec4 NavHighlight			= ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		ImVec4 NavWindowHighlight	= ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		ImVec4 NavWindowDimBg		= ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		ImVec4 ModalWindowDimBg		= ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		SImGuiColorProfile() = default;

		// Constructs profile in the same way as default, params are reused for various elements.
		SImGuiColorProfile(const ImVec4& backgroundBase, const ImVec4& backgroundMid, const ImVec4& elementBackground, const ImVec4& elementHovered, const ImVec4& elementActive, const ImVec4& elementHightlight)
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
			ScrollbarGrabActive	= ElementActive;
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
			TabActive = ElementActive;
			PlotLinesHovered = ElementHighlight;
			PlotHistogram = ElementHighlight;
			PlotHistogramHovered = ElementActive;
			DragDropTarget = ElementHighlight;
		}
	};

	// Havtorn Default == Struct Default Values
	struct SImGuiStyleProfile
	{
		ImVec2 WindowPadding		= ImVec2(8.00f, 8.00f);
		ImVec2 FramePadding			= ImVec2(5.00f, 2.00f);
		ImVec2 CellPadding			= ImVec2(6.00f, 6.00f);
		ImVec2 ItemSpacing			= ImVec2(6.00f, 6.00f);
		ImVec2 ItemInnerSpacing		= ImVec2(6.00f, 6.00f);
		ImVec2 TouchExtraPadding	= ImVec2(0.00f, 0.00f);
		F32 IndentSpacing			= 25.f;
		F32 ScrollbarSize			= 15.f;
		F32 GrabMinSize				= 10.f;
		F32 WindowBorderSize		= 1.f;
		F32 ChildBorderSize			= 1.f;
		F32 PopupBorderSize			= 1.f;
		F32 FrameBorderSize			= 1.f;
		F32 TabBorderSize			= 1.f;
		F32 WindowRounding			= 1.f;
		F32 ChildRounding			= 1.f;
		F32 FrameRounding			= 1.f;
		F32 PopupRounding			= 1.f;
		F32 ScrollbarRounding		= 1.f;
		F32 GrabRounding			= 1.f;
		F32 LogSliderDeadzone		= 4.f;
		F32 TabRounding				= 1.f;

		SImGuiStyleProfile() = default;
	};

#define IMGUIMANAGER_USE_LOGS 0

	class HAVTORN_API GImGuiManager
	{
		// For construction and destruction, has ownership of:
		friend class CImGuiProcess;
		// For WindowProc access:
		friend class CWindowHandler;

	public:
		static const char* Version;
		static const char* DefaultFont;
		static const F32 DefaultFontSize;

		static ImGuiContext* GetContext();
		static void GetAllocatorFunctions(ImGuiMemAllocFunc* allocFunc, ImGuiMemFreeFunc* freeFunc, void** userData);
		static void SetImGuiColorProfile(const SImGuiColorProfile& colorProfile);
		static void SetImGuiStyleProfile(const SImGuiStyleProfile& styleProfile);
		
		static void SetFontTTF(const char* fontPath = DefaultFont, const F32 size = DefaultFontSize);	

	private:
		static void WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void NewFrame();
		void EndFrame();

		GImGuiManager();
		~GImGuiManager();
		bool WindowsInitDX11(const HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* context);

		static bool HasInstance();

		static GImGuiManager* Instance;
	};
}
