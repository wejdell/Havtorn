// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ImGuiManager.h"

#include "Core/imgui.h"
#include "Core/imgui_impl_win32.h"
#include "Core/imgui_impl_dx11.h"
#include "Core/ImGuizmo/ImGuizmo.h"

//#include <d3d11.h>
//#pragma comment(lib, "d3d11.lib")

//#include "../Engine/Log.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Havtorn
{
    GImGuiManager* GImGuiManager::Instance = nullptr;

    GImGuiManager::GImGuiManager()
    {
        if (Instance)
        {
            HV_LOG_ERROR("Trying to create GImGuiManager but an instance already exists.");
            return;
        }

        Instance = this;
        ImGui::CreateContext();
        HV_LOG_INFO("GImGuiManager::Instance created.");
    }

    GImGuiManager::~GImGuiManager()
    {
        ImGui_ImplWin32_Shutdown();
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext();

        Instance = nullptr;
        HV_LOG_INFO("GImGuiManager::Instance destroyed.");
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

    void GImGuiManager::WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (HasInstance())
            ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
            //ImGui_ImplWin32_WndProcHandler((HWND)hWnd, msg, (WPARAM)wParam, (LPARAM)lParam);
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
            HV_LOG_ERROR("ImGui_ImplWin32_Init(hWnd) failed!");
            return false;
        }

        if (!ImGui_ImplDX11_Init(device, context))
        {
            HV_LOG_ERROR("ImGui_ImplDX11_Init(device, context) failed!");
            return false;
        }

        return true;
    }

    bool GImGuiManager::HasInstance()
    {
        if (!Instance)
        {
            HV_LOG_WARN("Trying to get GImGuiManager::Instance but none exists!");
            return false;
        }
        return true;
    }
}