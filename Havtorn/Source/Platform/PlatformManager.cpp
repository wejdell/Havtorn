// Copyright 2022 Team Havtorn. All Rights Reserved.

#include <hvpch.h>

#include "PlatformManager.h"
#include "PlatformUtilities.h"

#include <string>
#include <vector>
#include <algorithm>

#include <Log.h>
#include <CommandLine.h>
#include <FileSystem.h>
#include <GeneralUtilities.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

namespace Havtorn
{
	CPlatformManager::CPlatformManager()
	{}

	CPlatformManager::~CPlatformManager()
	{
		WindowHandle = 0;

		if (SplashWindow != nullptr)
			SDL_DestroyWindow(SplashWindow);

		SDL_DestroySurface(SplashSurface);
		SDL_DestroyWindow(Window);
	}

	SDL_HitTestResult SplashHitTest(SDL_Window* /*window*/, const SDL_Point* /*point*/, void* /*callbackData*/)
	{
		return SDL_HITTEST_DRAGGABLE;
	}

	SDL_HitTestResult WindowHitTest(SDL_Window* window, const SDL_Point* point, void* /*callbackData*/)
	{
		I32 windowWidth = 0;
		I32 windowHeight = 0;
		SDL_GetWindowSize(window, &windowWidth, &windowHeight);
		
		const SVector2<F32> windowSize = SVector2<F32>(STATIC_F32(windowWidth), STATIC_F32(windowHeight));
		const SVector2<F32> havtornPoint = { STATIC_F32(point->x), STATIC_F32(point->y) };
		const F32 borderThickness = 4.0f;
		const F32 titlebarHeight = 18.0f;

		const bool leftBorder = havtornPoint.X <= borderThickness;
		const bool rightBorder = havtornPoint.X > (windowWidth - borderThickness);
		const bool topBorder = havtornPoint.Y <= borderThickness;
		const bool bottomBorder = havtornPoint.Y > (windowHeight - borderThickness);

		if (topBorder && leftBorder)
			return SDL_HITTEST_RESIZE_TOPLEFT;

		if (topBorder && rightBorder)
			return SDL_HITTEST_RESIZE_TOPRIGHT;

		if (bottomBorder && leftBorder)
			return SDL_HITTEST_RESIZE_BOTTOMLEFT;

		if (bottomBorder && rightBorder)
			return SDL_HITTEST_RESIZE_BOTTOMRIGHT;

		if (topBorder)
			return SDL_HITTEST_RESIZE_TOP;

		if (bottomBorder)
			return SDL_HITTEST_RESIZE_BOTTOM;

		if (leftBorder)
			return SDL_HITTEST_RESIZE_LEFT;

		if (rightBorder)
			return SDL_HITTEST_RESIZE_RIGHT;

		constexpr F32 leftMenuButtonsExtent = 260.0f;
		constexpr F32 rightMenuButtonsExtent = 100.0f;

		if (havtornPoint.Y < titlebarHeight && UMath::IsWithin(havtornPoint.X, leftMenuButtonsExtent, windowSize.X - rightMenuButtonsExtent))
			return SDL_HITTEST_DRAGGABLE;

		return SDL_HITTEST_NORMAL;
	}

#ifdef HV_PLATFORM_WINDOWS
	LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
	{
		WNDPROC ogProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		LRESULT res = CallWindowProc(ogProc, hwnd, msg, wParam, lParam);

		switch (msg) 
		{
		case WM_COPYDATA:
		{
			COPYDATASTRUCT* cds = reinterpret_cast<COPYDATASTRUCT*>(lParam);
			std::string stringData(reinterpret_cast<char*>(cds->lpData), cds->cbData / sizeof(char));
			UCommandLine::Parse(stringData);
			HV_LOG_INFO("DeepLink: %s", UCommandLine::GetDeepLinkCommand().c_str());
		} break;
		}

		return res;
	}
#endif // HV_PLATFORM_WINDOWS

	bool CPlatformManager::Init(CPlatformManager::SWindowData windowData)
	{
		const CJsonDocument document = UFileSystem::OpenJson("Config/EngineConfig.json");
#ifdef HV_EDITOR_BUILD
		const std::string windowTitle = "Havtorn Editor";
#else
		const std::string windowTitle = document.GetString("Game Name", "Havtorn Editor");
#endif

		SplashSurface = SDL_LoadBMP(document.GetString("Splash Path", "Resources/HavtornSplash.bmp").c_str());
		
		// NW: SteamAPI_InitEx goes here
		SDL_SetAppMetadata(windowTitle.c_str(), HAVTORN_VERSION, NULL);
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC);
		SplashWindow = SDL_CreateWindow(windowTitle.c_str(), SplashSurface->w, SplashSurface->h, SDL_WINDOW_BORDERLESS);
		const SDL_Rect defaultRect = { .x = 0, .y = 0, .w = SplashSurface->w, .h = SplashSurface->h };
		SDL_BlitSurface(SplashSurface, &defaultRect, SDL_GetWindowSurface(SplashWindow), &defaultRect);
		SDL_UpdateWindowSurface(SplashWindow);
		SDL_SetWindowIcon(SplashWindow, SplashSurface);
		SDL_SetWindowHitTest(SplashWindow, SplashHitTest, nullptr);

		Window = SDL_CreateWindow(windowTitle.c_str(), windowData.Width, windowData.Height, SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
		SDL_UpdateWindowSurface(Window);
		SDL_SetWindowHitTest(Window, WindowHitTest, nullptr);

#ifdef HV_PLATFORM_WINDOWS
		WindowHandle = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(Window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
		
		WNDPROC ogProc = (WNDPROC)SetWindowLongPtr(WindowHandle, GWLP_WNDPROC, (LONG_PTR)CustomWndProc);
		SetWindowLongPtr(WindowHandle, GWLP_USERDATA, (LONG_PTR)ogProc);
#endif // HV_PLATFORM_WINDOWS

		const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(Window));
		MaxResolution = SVector2<U16>(STATIC_U16(mode->w), STATIC_U16(mode->h));
		Resolution = { WindowData.Width, WindowData.Height };
		ResizeTarget = {};

		ShouldRun = true;

		return true;
	}

	const HWND CPlatformManager::GetWindowHandle() const
	{
		return WindowHandle;
	}

	SDL_Window* CPlatformManager::GetMainWindow() const
	{
		return Window;
	}

	void CPlatformManager::OnApplicationReady()
	{
		CloseSplashWindow();

		// NW: Not sure if it's an SDL bug or not, but the icon doesn't show correctly in the task manager unless we do this here
		SDL_SetWindowIcon(Window, SplashSurface);

#if HV_PLATFORM_WINDOWS
		// Create shortcut to executable
		const std::string rootPath = UFileSystem::GetExecutableRootPath();
		const std::string workingPath = UFileSystem::GetWorkingPath();
		std::string executableName;

#if HV_CONFIG_EDITOR_DEBUG
		executableName = "EditorDebug";
#elif HV_CONFIG_EDITOR_DEVELOPMENT
		executableName = "EditorDevelopment";
#elif HV_CONFIG_GAME_DEBUG
		executableName = "GameDebug";
#elif HV_CONFIG_GAME_RELEASE
		executableName = "GameRelease";
#endif

		const std::string objectPath = (rootPath + executableName + ".exe");
		const std::string linkPath = (workingPath + executableName + ".lnk");

		// TODO.NW: Add link description?
		CreateLink(objectPath.c_str(), linkPath.c_str(), 0);
		HV_LOG_INFO("Created link '%s' for target '%s'", linkPath.c_str(), objectPath.c_str());
#endif // HV_PLATFORM_WINDOWS

		HV_LOG_INFO("DeepLink command: %s", UCommandLine::GetDeepLinkCommand().c_str());

		const std::vector<std::string> commandLineParams = UCommandLine::GetFreeParameters();
		for (auto param : commandLineParams)
			HV_LOG_INFO("PARAM: %s", param.c_str());
	}

	void CPlatformManager::BeginFrame()
	{
		EventLoop();
	}

	void CPlatformManager::EventLoop()
	{
		SDL_Event currentEvent;
		while (SDL_PollEvent(&currentEvent))
		{
			OnProcessEvent.Broadcast(&currentEvent);

			switch (currentEvent.type)
			{
			case SDL_EVENT_QUIT:
			{
				ShouldRun = false;
			}
			break;

			case SDL_EVENT_WINDOW_RESIZED:
			{
				//AS: Setting Resize Width/Height to != 0 will trigger a Resize in-engine.
				ResizeTarget.X = STATIC_U16(currentEvent.window.data1);
				ResizeTarget.Y = STATIC_U16(currentEvent.window.data2);
				UpdateResolution();
				SDL_UpdateWindowSurface(Window);
			}
			break;

			case SDL_EVENT_WINDOW_FOCUS_LOST:
			{
			}
			break;

			case SDL_EVENT_WINDOW_FOCUS_GAINED:
			{
			}
			break;

			case SDL_EVENT_DROP_FILE:
			{
				const std::string droppedFilePath = currentEvent.drop.data;
				HV_LOG_INFO(droppedFilePath.c_str());
				OnDragDropAccepted.Broadcast({ droppedFilePath });
			}
			break;

			default:
				break;
			}
		}
	}

	SVector2<U16> CPlatformManager::GetResolution() const
	{
		return Resolution;
	}

	void CPlatformManager::UpdateResolution()
	{
		// NW: ResizeTarget is set through the message pump
		if (ResizeTarget.LengthSquared() > 0)
		{
			Resolution = ResizeTarget;
			OnResolutionChanged.Broadcast(ResizeTarget);
			ResizeTarget = SVector2<U16>::Zero;
		}
	}

	void CPlatformManager::MinimizeWindow() const
	{
		SDL_MinimizeWindow(Window);
	}

	void CPlatformManager::MaximizeWindow()
	{
		SDL_MaximizeWindow(Window);
	}

	void CPlatformManager::CloseWindow()
	{
		SDL_Event quitEvent;
		quitEvent.window.type = SDL_EVENT_QUIT;
		SDL_PushEvent(&quitEvent);
	}

	void CPlatformManager::CloseSplashWindow()
	{
		SDL_DestroyWindow(SplashWindow);
		SDL_RaiseWindow(Window);
	}
}
