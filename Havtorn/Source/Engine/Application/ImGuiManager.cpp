// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ImGuiManager.h"

#include "Core/imgui.h"
#include "Core/imgui_impl_win32.h"
#include "Core/imgui_impl_dx11.h"
#include "Core/ImGuizmo/ImGuizmo.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Havtorn
{
    GImGuiManager* GImGuiManager::Instance = nullptr;

    const char* GImGuiManager::Version = "1.86 WIP";
    const char* GImGuiManager::DefaultFont = "../External/imgui/misc/fonts/Roboto-Medium.ttf";
    const F32 GImGuiManager::DefaultFontSize = 15.0f;

    GImGuiManager::GImGuiManager()
    {
        if (Instance)
        {
            HV_LOG_ERROR("Trying to create GImGuiManager but an instance already exists.");
            return;
        }

        Instance = this;
        ImGui::CreateContext();
        SetImGuiColorProfile(SImGuiColorProfile());
        SetImGuiStyleProfile(SImGuiStyleProfile());

        ImGui::DebugCheckVersionAndDataLayout(Version, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(unsigned int));
        ImGui::GetIO().Fonts->AddFontFromFileTTF(DefaultFont, DefaultFontSize);

#if IMGUIMANAGER_USE_LOGS
        HV_LOG_INFO("GImGuiManager::Instance created.");
#endif
    }

    GImGuiManager::~GImGuiManager()
    {
        ImGui_ImplWin32_Shutdown();
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext();

        Instance = nullptr;

#if IMGUIMANAGER_USE_LOGS
        HV_LOG_INFO("GImGuiManager::Instance destroyed.");
#endif
    }

    ImGuiContext* GImGuiManager::GetContext()
    {
        if (HasInstance())
            return ImGui::GetCurrentContext();
        else
            return nullptr;
    }

    void GImGuiManager::GetAllocatorFunctions(ImGuiMemAllocFunc* allocFunc, ImGuiMemFreeFunc* freeFunc, void** userData)
    {
        if (HasInstance())
            ImGui::GetAllocatorFunctions(allocFunc, freeFunc, userData);
    }

    void GImGuiManager::SetImGuiColorProfile(const SImGuiColorProfile& colorProfile)
    {
        if (!HasInstance())
            return;

        ImVec4* colors = (&ImGui::GetStyle())->Colors;

        colors[ImGuiCol_Text] = colorProfile.Text;
        colors[ImGuiCol_TextDisabled] = colorProfile.TextDisabled;
        colors[ImGuiCol_WindowBg] = colorProfile.WindowBg;
        colors[ImGuiCol_ChildBg] = colorProfile.ChildBg;
        colors[ImGuiCol_PopupBg] = colorProfile.PopupBg;
        colors[ImGuiCol_Border] = colorProfile.Border;
        colors[ImGuiCol_BorderShadow] = colorProfile.BorderShadow;
        colors[ImGuiCol_FrameBg] = colorProfile.FrameBg;
        colors[ImGuiCol_FrameBgHovered] = colorProfile.FrameBgHovered;
        colors[ImGuiCol_FrameBgActive] = colorProfile.FrameBgActive;
        colors[ImGuiCol_TitleBg] = colorProfile.TitleBg;
        colors[ImGuiCol_TitleBgActive] = colorProfile.TitleBgActive;
        colors[ImGuiCol_TitleBgCollapsed] = colorProfile.TitleBgCollapsed;
        colors[ImGuiCol_MenuBarBg] = colorProfile.MenuBarBg;
        colors[ImGuiCol_ScrollbarBg] = colorProfile.ScrollbarBg;
        colors[ImGuiCol_ScrollbarGrab] = colorProfile.ScrollbarGrab;
        colors[ImGuiCol_ScrollbarGrabHovered] = colorProfile.ScrollbarGrabHovered;
        colors[ImGuiCol_ScrollbarGrabActive] = colorProfile.ScrollbarGrabActive;
        colors[ImGuiCol_CheckMark] = colorProfile.CheckMark;
        colors[ImGuiCol_SliderGrab] = colorProfile.SliderGrab;
        colors[ImGuiCol_SliderGrabActive] = colorProfile.SliderGrabActive;
        colors[ImGuiCol_Button] = colorProfile.Button;
        colors[ImGuiCol_ButtonHovered] = colorProfile.ButtonHovered;
        colors[ImGuiCol_ButtonActive] = colorProfile.ButtonActive;
        colors[ImGuiCol_Header] = colorProfile.Header;
        colors[ImGuiCol_HeaderHovered] = colorProfile.HeaderHovered;
        colors[ImGuiCol_HeaderActive] = colorProfile.HeaderActive;
        colors[ImGuiCol_Separator] = colorProfile.Separator;
        colors[ImGuiCol_SeparatorHovered] = colorProfile.SeparatorHovered;
        colors[ImGuiCol_SeparatorActive] = colorProfile.ScrollbarGrabActive;
        colors[ImGuiCol_ResizeGrip] = colorProfile.ResizeGrip;
        colors[ImGuiCol_ResizeGripHovered] = colorProfile.ResizeGripHovered;
        colors[ImGuiCol_ResizeGripActive] = colorProfile.ResizeGripActive;
        colors[ImGuiCol_Tab] = colorProfile.Tab;
        colors[ImGuiCol_TabHovered] = colorProfile.TabHovered;
        colors[ImGuiCol_TabActive] = colorProfile.TabActive;
        colors[ImGuiCol_TabUnfocused] = colorProfile.TabUnfocused;
        colors[ImGuiCol_TabUnfocusedActive] = colorProfile.TabUnfocusedActive;
        colors[ImGuiCol_PlotLines] = colorProfile.PlotLines;
        colors[ImGuiCol_PlotLinesHovered] = colorProfile.PlotLinesHovered;
        colors[ImGuiCol_PlotHistogram] = colorProfile.PlotHistogram;
        colors[ImGuiCol_PlotHistogramHovered] = colorProfile.PlotHistogramHovered;
        colors[ImGuiCol_TextSelectedBg] = colorProfile.TextSelectedBg;
        colors[ImGuiCol_DragDropTarget] = colorProfile.DragDropTarget;
        colors[ImGuiCol_NavHighlight] = colorProfile.NavHighlight;
        colors[ImGuiCol_NavWindowingHighlight] = colorProfile.NavWindowHighlight;
        colors[ImGuiCol_NavWindowingDimBg] = colorProfile.NavWindowDimBg;
        colors[ImGuiCol_ModalWindowDimBg] = colorProfile.ModalWindowDimBg;
    }

    void GImGuiManager::SetImGuiStyleProfile(const SImGuiStyleProfile& styleProfile)
    {
        if (!HasInstance())
            return;

        ImGuiStyle* style = &ImGui::GetStyle();

        style->WindowPadding = styleProfile.WindowPadding;
        style->FramePadding = styleProfile.FramePadding;
        style->CellPadding = styleProfile.CellPadding;
        style->ItemSpacing = styleProfile.ItemSpacing;
        style->ItemInnerSpacing = styleProfile.ItemInnerSpacing;
        style->TouchExtraPadding = styleProfile.TouchExtraPadding;
        style->IndentSpacing = styleProfile.IndentSpacing;
        style->ScrollbarSize = styleProfile.ScrollbarSize;
        style->GrabMinSize = styleProfile.GrabMinSize;
        style->WindowBorderSize = styleProfile.WindowBorderSize;
        style->ChildBorderSize = styleProfile.ChildBorderSize;
        style->PopupBorderSize = styleProfile.PopupBorderSize;
        style->FrameBorderSize = styleProfile.FrameBorderSize;
        style->TabBorderSize = styleProfile.TabBorderSize;
        style->WindowRounding = styleProfile.WindowRounding;
        style->ChildRounding = styleProfile.ChildRounding;
        style->FrameRounding = styleProfile.FrameRounding;
        style->PopupRounding = styleProfile.PopupRounding;
        style->ScrollbarRounding = styleProfile.ScrollbarRounding;
        style->GrabRounding = styleProfile.GrabRounding;
        style->LogSliderDeadzone = styleProfile.LogSliderDeadzone;
        style->TabRounding = styleProfile.TabRounding;
    }

    void GImGuiManager::SetFontTTF(const char* fontPath, const F32 size)
    {
        if (HasInstance())
            ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath, size);
    }

    void GImGuiManager::WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (HasInstance())
            ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    }

    void GImGuiManager::NewFrame()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void GImGuiManager::EndFrame()
    {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    bool GImGuiManager::WindowsInitDX11(const HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        if (!ImGui_ImplWin32_Init(hWnd))
        {
#if IMGUIMANAGER_USE_LOGS
            HV_LOG_ERROR("ImGui_ImplWin32_Init(hWnd) failed!");
#endif
            return false;
        }

        if (!ImGui_ImplDX11_Init(device, context))
        {
#if IMGUIMANAGER_USE_LOGS
            HV_LOG_ERROR("ImGui_ImplDX11_Init(device, context) failed!");
#endif
            return false;
        }

        return true;
    }

    bool GImGuiManager::HasInstance()
    {
        if (!Instance)
        {
#if IMGUIMANAGER_USE_LOGS
            HV_LOG_WARN("Trying to get GImGuiManager::Instance but none exists!");
#endif
            return false;
        }
        return true;
    }
}
