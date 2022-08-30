// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/WindowsInclude.h"

#include <Core/imgui.h>

//struct ImGuiContext;
//typedef void*	  (*ImGuiMemAllocFunc)(size_t sz, void* user_data);               // Function signature for ImGui::SetAllocatorFunctions()
//typedef void    (*ImGuiMemFreeFunc)(void* ptr, void* user_data);                // Function signature for ImGui::SetAllocatorFunctions()

namespace Havtorn
{
	class CWindowHandler
	{
		friend class GEngine;
		friend class CEditorManager;
		friend class CGraphicsFramework;

	public:
		struct SWindowData
		{
			U16 X = 0;
			U16 Y = 0;
			U16 Width = 1280;
			U16 Height = 720;
		};

		HAVTORN_API static LRESULT CALLBACK WinProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
		HAVTORN_API SVector2<F32> GetResolution();
		SVector2<F32> GetCenterPosition();
		const float GetResolutionScale() const;

	private:
		CWindowHandler();
		~CWindowHandler();

		HAVTORN_API ImGuiContext* GetImGuiContext() const;
		HAVTORN_API void GetImGuiAllocatorFunctions(ImGuiMemAllocFunc* allocFunc, ImGuiMemFreeFunc* freeFunc, void** userData) const;

		HAVTORN_API const HWND GetWindowHandle() const;
		bool Init(SWindowData someWindowData);
		void SetWindowTitle(const std::string& title);

		const bool CursorLocked() const;

		HAVTORN_API void LockCursor(bool shouldLock);
		HAVTORN_API void HidLockCursor(bool shouldLock);

		HAVTORN_API void HideAndLockCursor(const bool& isInEditorMode = false);
		HAVTORN_API void ShowAndUnlockCursor(const bool& isInEditorMode = true);

	private:
		void SetInternalResolution();
		void SetResolution(SVector2<F32> resolution);

	private:
		CWindowHandler::SWindowData WindowData = {};
		HWND WindowHandle = 0;
		SVector2<F32>* Resolution = nullptr;
		F32 ResolutionScale = 1.0f;
		U16 MaxResX = 1920;
		U16 MaxResY = 1080;
		bool CursorIsLocked = false;
		bool WindowIsInEditingMode = false;
	};
}
