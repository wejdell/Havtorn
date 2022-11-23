// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core/imgui.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Havtorn
{
	class _declspec(dllexport) GImGuiManager
	{
		friend class CImGuiProcess;
		friend class CWindowHandler;
		friend class CRenderManager;//temp
	public:
		static ImGuiContext* GetContext();
		static void GetAllocatorFunctions(ImGuiMemAllocFunc* allocFunc, ImGuiMemFreeFunc* freeFunc, void** userData);

	private:
		//static void WindowsWindowProc(void* hWnd, unsigned int msg, void* wParam, void* lParam);
		static void WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void NewFrame();
		void EndFrame();

		GImGuiManager();
		~GImGuiManager();
		bool WindowsInitDX11(const HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* context);

		static bool HasInstance();

		static GImGuiManager* Instance;

	private:
		// Themes
	};
}