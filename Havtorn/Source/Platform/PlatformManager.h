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
		PLATFORM_API void UpdateResolution();

		PLATFORM_API void MinimizeWindow() const;
		PLATFORM_API void MaximizeWindow();
		PLATFORM_API void CloseWindow();
		PLATFORM_API void CloseSplashWindow();

	public:
		CMulticastDelegate<const SDL_Event*> OnProcessEvent;
		CMulticastDelegate<std::vector<std::string>> OnDragDropAccepted;
		CMulticastDelegate<SVector2<U16>> OnResolutionChanged;

		PLATFORM_API const HWND GetWindowHandle() const;
		PLATFORM_API SDL_Window* GetMainWindow() const;

		PLATFORM_API void OnApplicationReady();

		PLATFORM_API void SetBlockWindowHitTest(const bool shouldBlock);

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

		bool ShouldRun = false;
		bool BlockHitTest = false;
	};
}
