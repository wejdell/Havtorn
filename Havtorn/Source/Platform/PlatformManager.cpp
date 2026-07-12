// Copyright 2022 Team Havtorn. All Rights Reserved.

#include <hvpch.h>

#include "PlatformManager.h"
#include "PlatformUtilities.h"

#include <string>
#include <vector>
#include <algorithm>

#include <Log.h>
#include <MetaCommand/MetaCommandRouter.h>
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
#ifdef HV_EDITOR_BUILD
		if (UFileSystem::Exists(UFileSystem::LastRunSettings))
		{
			CJsonDocument document = UFileSystem::OpenJson(UFileSystem::LastRunSettings);
			if (!document.HasMember("Window Width"))
				document.AddMember("Window Width", 0);
			if (!document.HasMember("Window Height"))
				document.AddMember("Window Height", 0);

			document.Set("Window Width", STATIC_I32(Resolution.X));
			document.Set("Window Height", STATIC_I32(Resolution.Y));
		}
#elif HV_GAME_BUILD
		// TODO.NW: Set up conditions for pure game build configurations
#endif

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

	SDL_HitTestResult WindowHitTest(SDL_Window* window, const SDL_Point* point, void* callbackData)
	{
		SHitTestData hitTestData = callbackData == nullptr ? SHitTestData() : *reinterpret_cast<SHitTestData*>(callbackData);
		if (hitTestData.BlockHitTest || hitTestData.IsFullscreen)
			return SDL_HITTEST_NORMAL;

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

		if (havtornPoint.Y < titlebarHeight)
			return SDL_HITTEST_DRAGGABLE;

		if (topBorder && leftBorder)
			return SDL_HITTEST_RESIZE_TOPLEFT;

		if (topBorder && rightBorder)
			return SDL_HITTEST_RESIZE_TOPRIGHT;

		if (bottomBorder && leftBorder)
			return SDL_HITTEST_RESIZE_BOTTOMLEFT;

		if (bottomBorder && rightBorder)
			return SDL_HITTEST_RESIZE_BOTTOMRIGHT;

		if (bottomBorder)
			return SDL_HITTEST_RESIZE_BOTTOM;

		if (leftBorder)
			return SDL_HITTEST_RESIZE_LEFT;

		if (rightBorder)
			return SDL_HITTEST_RESIZE_RIGHT;

		if (topBorder)
			return SDL_HITTEST_RESIZE_TOP;

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
			
			if (UCommandLine::HasDeepLinkCommand())
				UMetaCommandRouter::Push(SMetaCommand(UCommandLine::GetDeepLinkCommand()));
		} break;
		}

		return res;
	}
#endif // HV_PLATFORM_WINDOWS

	bool CPlatformManager::Init()
	{
		const CJsonDocument document = UFileSystem::OpenJson(UFileSystem::EngineConfig);
#ifdef HV_EDITOR_BUILD
		const std::string windowTitle = "Havtorn Editor";
#else
		const std::string windowTitle = document.GetString("Game Name", "Havtorn Editor");
#endif
		SDL_SetAppMetadata(windowTitle.c_str(), HAVTORN_VERSION, NULL);

		SDL_InitFlags initFlags = SDL_INIT_VIDEO | SDL_INIT_GAMEPAD;
#ifdef HV_AUDIO_BACKEND_SDL
		initFlags |= SDL_INIT_AUDIO;
#endif
		// NW: SteamAPI_InitEx goes here
		SDL_Init(initFlags);
		
		SplashSurface = SDL_LoadBMP(document.Get("Splash Path", "Resources/HavtornSplash.bmp").c_str());
		SplashWindow = SDL_CreateWindow(windowTitle.c_str(), SplashSurface->w, SplashSurface->h, SDL_WINDOW_BORDERLESS);
		const SDL_Rect defaultRect = { .x = 0, .y = 0, .w = SplashSurface->w, .h = SplashSurface->h };
		SDL_BlitSurface(SplashSurface, &defaultRect, SDL_GetWindowSurface(SplashWindow), &defaultRect);
		SDL_UpdateWindowSurface(SplashWindow);
		SDL_SetWindowIcon(SplashWindow, SplashSurface);
		SDL_SetWindowHitTest(SplashWindow, SplashHitTest, nullptr);

		SVector2<U16> windowResolution = { 1280, 720 };
#ifdef HV_EDITOR_BUILD
		if (!UFileSystem::Exists(UFileSystem::LastRunSettings))
			UFileSystem::AddFile(UFileSystem::LastRunSettings, "{}");

		const CJsonDocument lastRunSettings = UFileSystem::OpenJson(UFileSystem::LastRunSettings);
		const U16 lastRunWidth = STATIC_U16(lastRunSettings.Get("Window Width", 0));
		const U16 lastRunHeight = STATIC_U16(lastRunSettings.Get("Window Height", 0));
		if (lastRunWidth != 0)
			windowResolution.X = lastRunWidth;
		if (lastRunHeight != 0)
			windowResolution.Y = lastRunHeight;
#elif HV_GAME_BUILD
		// TODO.NW: Set up conditions for pure game build configurations
#endif
		// TODO.NW: Gather command line options together somewhere they can be viewed, make use of that list to implement a "help" command
		if (UCommandLine::IsOptionParameterValid("ResX"))
			windowResolution.X = STATIC_U16(std::stoi(UCommandLine::GetOptionParameter("ResX"))); // TODO.NW: Detect max screen res and clamp to that
		if (UCommandLine::IsOptionParameterValid("ResY"))
			windowResolution.X = STATIC_U16(std::stoi(UCommandLine::GetOptionParameter("ResY"))); // TODO.NW: Detect max screen res and clamp to that

		Window = SDL_CreateWindow(windowTitle.c_str(), windowResolution.X, windowResolution.Y, SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
		SDL_UpdateWindowSurface(Window);
		SDL_SetWindowHitTest(Window, WindowHitTest, &HitTestData);

#ifdef HV_PLATFORM_WINDOWS
		WindowHandle = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(Window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
		
		WNDPROC ogProc = (WNDPROC)SetWindowLongPtr(WindowHandle, GWLP_WNDPROC, (LONG_PTR)CustomWndProc);
		SetWindowLongPtr(WindowHandle, GWLP_USERDATA, (LONG_PTR)ogProc);
#endif // HV_PLATFORM_WINDOWS

		Resolution = windowResolution;
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

		if (UCommandLine::HasDeepLinkCommand())
			UMetaCommandRouter::Push(SMetaCommand(UCommandLine::GetDeepLinkCommand()));

		const std::vector<std::string> commandLineParams = UCommandLine::GetFreeParameters();
		for (auto param : commandLineParams)
			HV_LOG_INFO("PARAM: %s", param.c_str());
	}

	void CPlatformManager::SetBlockWindowHitTest(const bool shouldBlock)
	{
		HitTestData.BlockHitTest = shouldBlock;
	}

	void CPlatformManager::SetCursorLock(const bool shouldLock)
	{
		SDL_SetWindowRelativeMouseMode(Window, shouldLock);
		SDL_SetWindowMouseGrab(Window, shouldLock);
		if (shouldLock)
		{
			const SDL_Rect rect = { STATIC_U16(Resolution.X * 0.5f), STATIC_U16(Resolution.Y * 0.5f), 1, 1};
			SDL_SetWindowMouseRect(Window, &rect);
		}
		else
		{
			SDL_SetWindowMouseRect(Window, nullptr);
		}
	}

	bool CPlatformManager::IsCursorLocked() const
	{
		return SDL_GetWindowRelativeMouseMode(Window);
	}

	void CPlatformManager::BeginFrame()
	{
		EventLoop();
	}

	void CPlatformManager::EndFrame()
	{
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

			case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
			{
				HitTestData.IsFullscreen = true;
			}
			break;

			case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
			{
				HitTestData.IsFullscreen = false;
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
				DroppedFileBuffer.push_back(currentEvent.drop.data);
			}
			break;

			case SDL_EVENT_DROP_COMPLETE:
			{
				OnDragDropAccepted.Broadcast(DroppedFileBuffer);
				DroppedFileBuffer.clear();
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
