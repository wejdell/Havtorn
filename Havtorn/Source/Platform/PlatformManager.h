// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "WindowsInclude.h"
#include <Core.h>
#include <MathTypes/Vector.h>
#include <HavtornDelegate.h>

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
		SVector2<U16> GetCenterPosition() const;
		const float GetResolutionScale() const;

		// TODO.AG: reconsider access rights.
		PLATFORM_API void EnableDragDrop() const;
		PLATFORM_API void DisableDragDrop() const;

	public:
		CMulticastDelegate<std::vector<std::string>> OnDragDropAccepted;
		CMulticastDelegate<SVector2<U16>> OnResolutionChanged;

		PLATFORM_API const HWND GetWindowHandle() const;

	private:
		CPlatformManager();
		~CPlatformManager();

		bool Init(SWindowData someWindowData);
		void SetWindowTitle(const std::string& title);

		const bool CursorLocked() const;

		PLATFORM_API void LockCursor(bool shouldLock);
		PLATFORM_API void HidLockCursor(bool shouldLock);

		PLATFORM_API void HideAndLockCursor(const bool& isInEditorMode = false);
		PLATFORM_API void ShowAndUnlockCursor(const bool& isInEditorMode = true);

	private:
		void SetInternalResolution();
		void SetResolution(SVector2<U16> resolution);

		void InitWindowsImaging();

	private:

		CPlatformManager::SWindowData WindowData = {};
		HWND WindowHandle = 0;
		SVector2<U16> Resolution;
		F32 ResolutionScale = 1.0f;
		U16 MaxResX = 1920;
		U16 MaxResY = 1080;
		bool CursorIsLocked = false;
		bool WindowIsInEditingMode = false;

	public:
		SVector2<U16> ResizeTarget = {};
		/*	UINT ResizeWidth = 0;
			UINT ResizeHeight = 0;*/

	};
}
