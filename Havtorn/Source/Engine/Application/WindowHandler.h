// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/WindowsInclude.h"

namespace Havtorn
{
	class CWindowHandler
	{
		friend class GEngine;

	public:
		struct SWindowData
		{
			U16 X = 0;
			U16 Y = 0;
			U16 Width = 1280;
			U16 Height = 720;
		};

		HAVTORN_API static LRESULT CALLBACK WinProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

	private:
		CWindowHandler();
		~CWindowHandler();

		HAVTORN_API const HWND GetWindowHandle() const;
		SVector2<F32> GetCenterPosition();
		HAVTORN_API SVector2<F32> GetResolution();
		const float GetResolutionScale() const;
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
