// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "WindowsInclude.h"
#include <Core.h>
#include <MathTypes/Vector.h>
#include <HavtornDelegate.h>
#include <functional>

struct SDL_Window;
struct SDL_Surface;
union SDL_Event;

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

		void BeginFrame();
		void EventLoop();

		PLATFORM_API SVector2<U16> GetResolution() const;
		PLATFORM_API SVector2<I16> GetCenterPosition() const;
		PLATFORM_API SVector2<I16> GetScreenCursorPos() const;

		PLATFORM_API void UpdateResolution();

		// TODO.AG: reconsider access rights.
		PLATFORM_API void EnableDragDrop() const;
		PLATFORM_API void DisableDragDrop() const;

		PLATFORM_API void MinimizeWindow() const;
		PLATFORM_API void MaximizeWindow();
		PLATFORM_API void CloseWindow();
		PLATFORM_API void CloseSplashWindow();

	public:
		// TODO.NW: Try figure out if we can bind to and bool returns instead
		CMulticastDelegate<HWND, UINT, WPARAM, LPARAM> OnMessageHandled;
		CMulticastDelegate<const SDL_Event*> OnProcessEvent;
		CMulticastDelegate<std::vector<std::string>> OnDragDropAccepted;
		CMulticastDelegate<SVector2<U16>> OnResolutionChanged;

		PLATFORM_API const HWND GetWindowHandle() const;
		PLATFORM_API SDL_Window* GetMainWindow() const;

		PLATFORM_API void OnApplicationReady();

	private:
		CPlatformManager();
		~CPlatformManager();

		bool Init(SWindowData windowData);

	private:
		CPlatformManager::SWindowData WindowData = {};
		HWND WindowHandle = 0;

		SDL_Window* Window = nullptr;
		SDL_Window* SplashWindow = nullptr;
		SDL_Surface* SplashSurface = nullptr;

		SVector2<U16> ResizeTarget = SVector2<U16>::Zero;
		SVector2<U16> Resolution = SVector2<U16>::Zero;
		
		SVector2<U16> MaxResolution = SVector2<U16>::Zero;
		SVector2<U16> MinResolution = SVector2<U16>{ 128, 72 };

		bool ShouldRun = false;

	};
}
