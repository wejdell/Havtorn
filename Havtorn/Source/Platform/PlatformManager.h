// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "WindowsInclude.h"
#include <Core.h>
#include <MathTypes/Vector.h>
#include <HavtornDelegate.h>
#include <functional>

namespace Havtorn
{
	class CPlatformManager
	{
		friend class CPlatformProcess;

	public:
		struct SWindowData
		{
			U16 X = 0;
			U16 Y = 0;
			U16 Width = 1280;
			U16 Height = 720;
		};

		PLATFORM_API static LRESULT CALLBACK WinProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
		PLATFORM_API SVector2<U16> GetResolution() const;
		PLATFORM_API SVector2<I16> GetCenterPosition() const;
		PLATFORM_API SVector2<I16> GetScreenCursorPos() const;

		PLATFORM_API void UpdateResolution();

		// TODO.AG: reconsider access rights.
		PLATFORM_API void EnableDragDrop() const;
		PLATFORM_API void DisableDragDrop() const;

		PLATFORM_API void UpdateWindow(const SVector2<I16>& windowPos, const SVector2<U16>& resolution);
		PLATFORM_API void UpdateRelativeCursorToWindowPos();
		PLATFORM_API void UpdateWindowPos();
		PLATFORM_API void MinimizeWindow();
		PLATFORM_API void MaximizeWindow();
		PLATFORM_API void CloseWindow();

	public:
		// TODO.NW: Try figure out if we can bind to and bool returns instead
		CMulticastDelegate<HWND, UINT, WPARAM, LPARAM> OnMessageHandled;
		CMulticastDelegate<std::vector<std::string>> OnDragDropAccepted;
		CMulticastDelegate<SVector2<U16>> OnResolutionChanged;

		PLATFORM_API const HWND GetWindowHandle() const;

	private:
		CPlatformManager();
		~CPlatformManager();

		bool Init(SWindowData windowData);
		void SetWindowTitle(const std::string& title);

		const bool CursorLocked() const;

		PLATFORM_API void LockCursor(bool shouldLock);
		PLATFORM_API void HidLockCursor(bool shouldLock);

		PLATFORM_API void HideAndLockCursor(const bool& isInEditorMode = false);
		PLATFORM_API void ShowAndUnlockCursor(const bool& isInEditorMode = true);

	private:
		void InitWindowsImaging();

	private:
		CPlatformManager::SWindowData WindowData = {};
		HWND WindowHandle = 0;

		SVector2<U16> Resolution = {};
		SVector2<U16> PreviousResolution = {};
		SVector2<I16> WindowPos = {};
		SVector2<I16> PreviousWindowPos = {};

		SVector2<F32> NormalizedWindowRelativeCursorPos = {};
		SVector2<I16> WindowRelativeCursorPos = {};
		SVector2<I16> CursorPosPreDrag = {};
		
		SVector2<U16> MaxResolution = {};
		
		bool CursorIsLocked = false;
		bool WindowIsInEditingMode = false;
		bool IsFullscreen = false;

		SVector2<U16> ResizeTarget = {};
	};
}
