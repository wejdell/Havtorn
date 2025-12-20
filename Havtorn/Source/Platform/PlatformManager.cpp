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

#include <wincodec.h>
#include <roapi.h>
#pragma comment(lib, "runtimeobject")
#pragma comment(lib, "windowscodecs")

namespace Havtorn
{
	LRESULT CPlatformManager::WinProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
	{
		static CPlatformManager* platformManager = nullptr;
		CREATESTRUCT* createStruct;

		if (platformManager)
			platformManager->OnMessageHandled.Broadcast(hwnd, uMsg, wParam, lParam);

		switch (uMsg)
		{
		case WM_CLOSE:

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_CREATE:
			createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
			platformManager = reinterpret_cast<CPlatformManager*>(createStruct->lpCreateParams);
			break;

		case WM_MOVE:
		{
			const HMONITOR primaryMonitor = MonitorFromPoint({ 0 }, MONITOR_DEFAULTTOPRIMARY);
			MONITORINFO monitorInfo = { 0 };
			monitorInfo.cbSize = sizeof(monitorInfo);
			GetMonitorInfo(primaryMonitor, &monitorInfo);

			const RECT monitorRect = monitorInfo.rcWork;
			POINT point = { 0 };
			if (!GetCursorPos(&point))
				break;

			// TODO.NW: Add non-snapping shortcut, so it's possible to move from one monitor to the other
			if (point.x <= monitorRect.left)
				platformManager->SnapWindow(EWindowSnapPosition::Left);
			else if (point.x >= monitorRect.right - 1)
				platformManager->SnapWindow(EWindowSnapPosition::Right);
			else if (point.y <= monitorRect.top)
				platformManager->SnapWindow(EWindowSnapPosition::Maximized);
			else if (platformManager->SnapPosition != EWindowSnapPosition::Unsnapped)
				platformManager->SnapWindow(EWindowSnapPosition::Unsnapped);
		}
		break;

		case WM_COPYDATA:
		{
			COPYDATASTRUCT* cds = reinterpret_cast<COPYDATASTRUCT*>(lParam);
			std::string stringData(reinterpret_cast<char*>(cds->lpData), cds->cbData / sizeof(char));
			UCommandLine::Parse(stringData);
			HV_LOG_INFO("DeepLink: %s", UCommandLine::GetDeepLinkCommand().c_str());
		}
		break;
		case WM_KILLFOCUS:
			//           platformManager->LockCursor(false); // If we use this here the WindowIsInEditingMode bool will be preserved
			break;

		case WM_SETFOCUS:
#ifdef _DEBUG
			//if (false/*platformManager->GameIsInMenu*/)
			if (platformManager)
				platformManager->ShowAndUnlockCursor();
			//else
				//platformManager->HideAndLockCursor();
			//platformManager->WindowIsInEditingMode ? platformManager->LockCursor(false) : platformManager->LockCursor(true);
#else
			//if (platformManager->myGameIsInMenu)
			if (platformManager)
				platformManager->ShowAndUnlockCursor();
			//else
				//platformManager->HideAndLockCursor();
#endif
			break;

			// The following cases suppresses the following effects (by hindering DefWindowProc from running):
			// The windows help menu popup, which pauses the application when pressing the Alt key
			// The handling of the Alt key input in general (still picked up by WM_KEYDOWN etc. in our own loop)
			// The windows sound played when pressing Alt+(any other key)
			//
			// This means that default Alt+(other key) bindings won't be caught, such as Alt+F4, Alt+Space etc.
			// Alt+Enter still works but it might still be worth figuring out what to do about these.
			//
			// This needs more testing.
		case WM_ENTERIDLE:
			//case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_SYSCHAR:
			return true;

		case WM_DROPFILES:
		{
#pragma region Extract FilePaths

			// Here we cast the wParam as a HDROP handle to pass into the next functions
			HDROP hDrop = (HDROP)wParam;

			// DragQueryFile() takes a LPWSTR for the name so we need a TCHAR string
			// This functions has a couple functionalities.  If you pass in 0xFFFFFFFF in
			// the second parameter then it returns the count of how many filers were drag
			// and dropped.  Otherwise, the function fills in the szName string array with
			// the current file being queried.
			TCHAR szName[MAX_PATH];
			int count = DragQueryFile(hDrop, 0xFFFFFFFF, szName, MAX_PATH);
			// Here we go through all the files that were drag and dropped then display them
			std::vector<std::string> dragDropFilePaths;
			for (U64 i = 0; i < count; i++)
			{
				// Grab the name of the file associated with index "i" in the list of files dropped.
				// Be sure you know that the name is attached to the FULL path of the file.
				DragQueryFile(hDrop, static_cast<UINT>(i), szName, MAX_PATH);
				char file[MAX_PATH] = {};
				for (I64 index = 0; index < MAX_PATH; index++)
					file[index] = static_cast<char>(szName[index]);

				dragDropFilePaths.push_back(std::string(file));
			}
			// Finally, we destroy the HDROP handle so the extra memory
			// allocated by the application is released.
			DragFinish(hDrop);

			for (auto& path : dragDropFilePaths)
				HV_LOG_INFO(path.c_str());

#pragma endregion
			if (platformManager)
				platformManager->OnDragDropAccepted.Broadcast(dragDropFilePaths);

			return 0;
		}break;

		case WM_SIZE:
			//AS: Storing the new Window Size from the windows-loop. 
			if (wParam == SIZE_MINIMIZED)
				return 0;

			//AS: Setting Resize Width/Height to != 0 will trigger a Resize in-engine.
			if (platformManager)
			{
				platformManager->ResizeTarget.X = STATIC_U16((U32)LOWORD(lParam));
				platformManager->ResizeTarget.Y = STATIC_U16((U32)HIWORD(lParam));
			}
			break;

		default:
			// Send through engine pump
			//CInput::GetInstance()->UpdateEvents(uMsg, wParam, lParam);
			break;

			//if (CInput::GetInstance()->UpdateEvents(uMsg, wParam, lParam))
			//    break;

			//return true;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	CPlatformManager::CPlatformManager()
	{}

	CPlatformManager::~CPlatformManager()
	{
		LockCursor(false);
		CursorIsLocked = false;
		WindowIsInEditingMode = false;
		WindowHandle = 0;
		UnregisterClass(LHavtornWindowClass, GetModuleHandle(nullptr));
		UnregisterClass(SplashScreenWindowClass, GetModuleHandle(TEXT("Platform")));
	}

	bool CPlatformManager::Init(CPlatformManager::SWindowData windowData)
	{
		CJsonDocument document = UFileSystem::OpenJson("Config/EngineConfig.json");

		// TODO.NW: Use LoadImage instead?
		HICON customIcon = (HICON)LoadImageA(NULL, document.GetString("Icon Path", "Resources/HavtornIcon.ico").c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);

		InitWindowsImaging();
		RegisterSplashWindowClass(customIcon);
		SplashHandle = CreateSplashWindow();
		SetSplashImage(SplashHandle, LoadSplashImage(document.GetString("Splash Path", "Resources/HavtornSplash.bmp")));

		WindowData = windowData;

		//HCURSOR customCursor = NULL;
		//if (document.HasMember("Cursor Path"))
		//    customCursor = LoadCursorFromFileA(document["Cursor Path"].GetString());

		//if (customCursor == NULL)
		//    customCursor = LoadCursor(nullptr, IDC_ARROW);

		WNDCLASSEX windowclass = {};
		windowclass.cbSize = sizeof(WNDCLASSEX);
		windowclass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
		windowclass.lpfnWndProc = CPlatformManager::WinProc;
		windowclass.cbClsExtra = 0;
		windowclass.cbWndExtra = 0;
		windowclass.hInstance = GetModuleHandle(nullptr);
		windowclass.hIcon = customIcon;
		//windowclass.hCursor = customCursor;
		windowclass.lpszClassName = LHavtornWindowClass;
		RegisterClassEx(&windowclass);

		std::string gameName = document.GetString("Game Name", "Havtorn Editor");
		MaxResolution = SVector2<U16>(STATIC_U16(GetSystemMetrics(SM_CXSCREEN)), STATIC_U16(GetSystemMetrics(SM_CYSCREEN)));

		if (false/*borderless*/)
		{
#ifdef _DEBUG
			// Start in borderless
			WindowHandle = CreateWindowA(HavtornWindowClass, gameName.c_str(),
				WS_POPUP | WS_VISIBLE,
				0, 0, WindowData.Width, WindowData.Height,
				NULL, NULL, GetModuleHandle(nullptr), this);
#else
			// Start in borderless
			WindowHandle = CreateWindowA(HavtornWindowClass, gameName.c_str(),
				WS_POPUP | WS_VISIBLE,
				0, 0, MaxResolution.X, MaxResolution.Y,
				NULL, NULL, GetModuleHandle(nullptr), this);
#endif
		}
		else
		{
			// Start in bordered window
			WindowHandle = CreateWindowA(HavtornWindowClass, gameName.c_str(),
				/*WS_OVERLAPPEDWINDOW | */WS_POPUP | WS_VISIBLE,
				WindowData.X, WindowData.Y, WindowData.Width, WindowData.Height,
				nullptr, nullptr, nullptr, this);
		}

		//::SetCursor(customCursor);

		Resolution = { WindowData.Width, WindowData.Height };
		ResizeTarget = {};

		EnableDragDrop();

		return true;
	}

	const HWND CPlatformManager::GetWindowHandle() const
	{
		return WindowHandle;
	}

	void CPlatformManager::OnApplicationReady()
	{
		CloseSplashWindow();

		// Create shortcut to executable
		std::string rootPath = UFileSystem::GetExecutableRootPath();
		std::string workingPath = UFileSystem::GetWorkingPath();
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

		std::string objectPath = (rootPath + executableName + ".exe");
		std::string linkPath = (workingPath + executableName + ".lnk");

		// TODO.NW: Add link description?
		CreateLink(objectPath.c_str(), linkPath.c_str(), 0);
		HV_LOG_INFO("Created link '%s' for target '%s'", linkPath.c_str(), objectPath.c_str());

		HV_LOG_INFO("DeepLink command: %s", UCommandLine::GetDeepLinkCommand().c_str());

		std::vector<std::string> commandLineParams = UCommandLine::GetFreeParameters();
		for (auto param : commandLineParams)
			HV_LOG_INFO("PARAM: %s", param.c_str());
	}

	SVector2<I16> CPlatformManager::GetCenterPosition() const
	{
		SVector2<I16> center = {};
		RECT rect = { 0 };
		if (GetWindowRect(WindowHandle, &rect))
		{
			center.X = STATIC_U16((rect.right - rect.left) / (I16)2);
			center.Y = STATIC_U16((rect.bottom - rect.top) / (I16)2);
		}
		return center;
	}

	SVector2<I16> CPlatformManager::GetScreenCursorPos() const
	{
		POINT point = { 0 };
		if (!GetCursorPos(&point))
			return SVector2<I16>(0);

		return { STATIC_I16(point.x), STATIC_I16(point.y) };
	}

	SVector2<U16> CPlatformManager::GetResolution() const
	{
		return Resolution;
	}

	HBITMAP CPlatformManager::LoadSplashImage(const std::string& filePath)
	{ 
		std::string workingPath = UFileSystem::GetWorkingPath();
		HANDLE handle = LoadImageA(GetModuleHandle(NULL), (workingPath + filePath).c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (handle == NULL)
			HV_LOG_ERROR("Splash image could not be found. Make sure that EngineConfig.json points to a BMP splash image in 8-bit (or 24-bit?) format.");

		return (HBITMAP)handle;
	}

	void CPlatformManager::InitWindowsImaging()
	{
//#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
//		Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
//		if (FAILED(initialize))
//			HV_LOG_ERROR("Windows Imaging could not be initialized");
//
//#else
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr))
			HV_LOG_ERROR("Windows Imaging could not be initialized");
//#endif
	}

	IStream* CPlatformManager::CreateStreamOnResource(LPCSTR resourceName, LPCSTR resourceType)
	{
		HMODULE platformModule = GetModuleHandle(TEXT("Platform"));
		HRSRC resource = FindResourceA(platformModule, resourceName, resourceType);
		if (resource == NULL)
			return NULL;

		DWORD resourceSize = SizeofResource(platformModule, resource);
		HGLOBAL imageGlobal = LoadResource(platformModule, resource);
		if (imageGlobal == NULL)
			return NULL;

		LPVOID sourceResourcePointer = LockResource(imageGlobal);
		if (sourceResourcePointer == NULL)
			return NULL;

		HGLOBAL resourceData = GlobalAlloc(GMEM_MOVEABLE, resourceSize);
		if (resourceData == NULL)
			return NULL;

		LPVOID resourceDataPointer = GlobalLock(resourceData);
		if (resourceDataPointer == NULL)
		{
			GlobalFree(resourceData);
			return NULL;
		}

		CopyMemory(resourceDataPointer, sourceResourcePointer, resourceSize);
		GlobalUnlock(resourceData);
		
		IStream* imageStream = NULL;
		if (SUCCEEDED(CreateStreamOnHGlobal(resourceData, TRUE, &imageStream)))
			return imageStream;

		return imageStream;
	}

	IWICBitmapSource* CPlatformManager::LoadBitmapFromStream(IStream* imageStream)
	{
		IWICBitmapDecoder* decoder = NULL;
		HRESULT result = CoCreateInstance(CLSID_WICPngDecoder, NULL, CLSCTX_INPROC_SERVER, __uuidof(decoder), reinterpret_cast<void**>(&decoder));
		if (FAILED(result))
			return NULL;

		if (FAILED(decoder->Initialize(imageStream, WICDecodeMetadataCacheOnLoad)))
		{
			decoder->Release();
			return NULL;
		}

		UINT frameCount = 0;
		if (FAILED(decoder->GetFrameCount(&frameCount)) || frameCount != 1)
		{
			decoder->Release();
			return NULL;
		}

		IWICBitmapFrameDecode* bitmapFrame = NULL;
		if (FAILED(decoder->GetFrame(0, &bitmapFrame)))
		{
			decoder->Release();
			return NULL;
		}

		IWICBitmapSource* bitmapSource = NULL;
		WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, bitmapFrame, &bitmapSource);
		bitmapFrame->Release();
		decoder->Release();
		return bitmapSource;
	}

	HBITMAP CPlatformManager::CreateBitmap(IWICBitmapSource* bitmapSource)
	{
		UINT width = 0;
		UINT height = 0;
		if (FAILED(bitmapSource->GetSize(&width, &height)) || width == 0 || height == 0)
			return NULL;

		BITMAPINFO bitmapInfo;
		ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));
		bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfo.bmiHeader.biWidth = width;
		bitmapInfo.bmiHeader.biHeight = -((LONG)height);
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biBitCount = 32;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;

		HBITMAP bitmap = NULL;
		void* imageBits = NULL;
		HDC hdcScreen = GetDC(NULL);
		bitmap = CreateDIBSection(hdcScreen, &bitmapInfo, DIB_RGB_COLORS, &imageBits, NULL, 0);
		ReleaseDC(NULL, hdcScreen);
		if (bitmap == NULL)
			return NULL;

		const UINT stride = width * 4;
		const UINT image = stride * height;
		if (FAILED(bitmapSource->CopyPixels(NULL, stride, image, static_cast<BYTE*>(imageBits))))
		{
			DeleteObject(bitmap);
			bitmap = NULL;
		}

		return bitmap;
	}

	void CPlatformManager::RegisterSplashWindowClass(HICON icon)
	{
		HINSTANCE instance = GetModuleHandle(nullptr);
		WNDCLASS windowClass = { 0 };
		windowClass.lpfnWndProc = DefWindowProc;
		windowClass.hInstance = instance;
		windowClass.hIcon = icon;
		windowClass.hCursor = LoadCursor(instance, IDC_ARROW);
		windowClass.lpszClassName = SplashScreenWindowClass;
		RegisterClass(&windowClass);
	}

	HWND CPlatformManager::CreateSplashWindow()
	{
		// NW: Invisible owner window so the splash shows up when alt+tabbing, but not in the taskbar. 
		// To make it appear in the taskbar, we can drop the owner window. 
		// If we don't want it anywhere, we could use the WS_EX_TOOLWINDOW extended window style.
		
		HINSTANCE instance = GetModuleHandle(nullptr);
		HWND ownerHandle = CreateWindow(SplashScreenWindowClass, NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, instance, NULL);
		return CreateWindowEx(WS_EX_LAYERED, SplashScreenWindowClass, NULL, WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, ownerHandle, NULL, instance, NULL);
	}

	void CPlatformManager::SetSplashImage(HWND hwndSplash, HBITMAP splashBitmap)
	{
		BITMAP bitmapData;
		GetObject(splashBitmap, sizeof(bitmapData), &bitmapData);
		SIZE sizeSplash = { bitmapData.bmWidth, bitmapData.bmHeight };

		POINT zeroPoint = { 0 };
		HMONITOR primaryMonitor = MonitorFromPoint(zeroPoint, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO monitorInfo = { 0 };
		monitorInfo.cbSize = sizeof(monitorInfo);
		GetMonitorInfo(primaryMonitor, &monitorInfo);

		RECT monitorRect = monitorInfo.rcWork;
		POINT splashOrigin;
		splashOrigin.x = monitorRect.left + (monitorRect.right - monitorRect.left - sizeSplash.cx) / 2;
		splashOrigin.y = monitorRect.top + (monitorRect.bottom - monitorRect.top - sizeSplash.cy) / 2;

		HDC hdcScreen = GetDC(NULL);
		HDC hdcMem = CreateCompatibleDC(hdcScreen);
		HBITMAP bitmapOld = (HBITMAP)SelectObject(hdcMem, splashBitmap);

		BLENDFUNCTION blend = { 0 };
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat = AC_SRC_ALPHA;

		RECT splashRect = { 0, 0, sizeSplash.cx, sizeSplash.cy };
		SetTextColor(hdcMem, RGB(20, 20, 20));
		SetBkMode(hdcMem, TRANSPARENT);
		DrawText(hdcMem, TEXT("Getting things ready..."), -1, &splashRect, DT_BOTTOM | DT_LEFT | DT_SINGLELINE);

		UpdateLayeredWindow(hwndSplash, hdcScreen, &splashOrigin, &sizeSplash, hdcMem, &zeroPoint, RGB(0, 0, 0), &blend, ULW_ALPHA);

		SelectObject(hdcMem, bitmapOld);
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdcScreen);
	}

	const bool CPlatformManager::CursorLocked() const
	{
		return CursorIsLocked;
	}

	void CPlatformManager::LockCursor(bool shouldLock)
	{
		CursorIsLocked = shouldLock;
		if (shouldLock)
		{
			while (::ShowCursor(FALSE) >= 0);
		}
		else {
			while (::ShowCursor(TRUE) < 0);
		}
		shouldLock ? static_cast<bool>(SetCapture(WindowHandle)) : static_cast<bool>(ReleaseCapture());
	}

	void CPlatformManager::HidLockCursor(bool shouldLock)
	{
		CursorIsLocked = shouldLock;
		if (shouldLock)
		{
			SVector2<I16> center = GetCenterPosition();
			SetCursorPos(center.X, center.Y);
		}
		shouldLock ? static_cast<bool>(SetCapture(WindowHandle)) : static_cast<bool>(ReleaseCapture());
	}

	void CPlatformManager::HideAndLockCursor(const bool& isInEditorMode)
	{
		while (::ShowCursor(FALSE) >= 0);
		SetCapture(WindowHandle);

		CursorIsLocked = true;
		WindowIsInEditingMode = isInEditorMode;

		SVector2<I16> center = GetCenterPosition();
		SetCursorPos(center.X, center.Y);
	}

	void CPlatformManager::ShowAndUnlockCursor(const bool& isInEditorMode)
	{
		while (::ShowCursor(TRUE) < 0);
		ReleaseCapture();

		CursorIsLocked = false;
		WindowIsInEditingMode = isInEditorMode;
	}

	void CPlatformManager::UpdateResolution()
	{
		// NW: ResizeTarget is set through the message pump
		if (ResizeTarget.LengthSquared() > 0)
		{
			OnResolutionChanged.Broadcast(ResizeTarget);
			ResizeTarget = { 0, 0 };
		}
	}

	void CPlatformManager::EnableDragDrop() const
	{
		DragAcceptFiles(WindowHandle, TRUE);
	}

	void CPlatformManager::DisableDragDrop() const
	{
		DragAcceptFiles(WindowHandle, FALSE);
	}

	void CPlatformManager::UpdateWindow(const SVector2<I16>& windowPos, const SVector2<U16>& resolution)
	{
		if (resolution.X > MaxResolution.X || resolution.Y > MaxResolution.Y)
			return;

		if (resolution.X <= MinResolution.X || resolution.Y <= MinResolution.Y)
		{
			HV_LOG_WARN("Tried to set a resolution lower than the minimum (minimizing does not count). May be good to investigate.");
			return;
		}

		IsFullscreen = resolution.IsEqual(MaxResolution);
		if (IsFullscreen && !Resolution.IsEqual(MaxResolution))
		{
			PreviousResolution = Resolution;
			PreviousWindowPos = WindowPos;
		}

		WindowPos = windowPos;
		Resolution = resolution;
		::MoveWindow(WindowHandle, WindowPos.X, WindowPos.Y, Resolution.X, Resolution.Y, true);
	}

	void CPlatformManager::UpdateRelativeCursorToWindowPos()
	{
		if (ResizeTarget.SizeSquared() > 0)
			return;

		RECT rect = { 0 };
		if (!GetWindowRect(WindowHandle, &rect))
			return;

		const SVector2<I16> windowPos = SVector2<I16>(STATIC_U16(rect.left), STATIC_U16(rect.top));
		CursorPosPreDrag = GetScreenCursorPos();
		WindowRelativeCursorPos = CursorPosPreDrag - windowPos;
		NormalizedWindowRelativeCursorPos = SVector2<F32>(STATIC_F32(GetScreenCursorPos().X - windowPos.X), STATIC_F32(GetScreenCursorPos().Y - windowPos.Y)) / SVector2<F32>(STATIC_F32(Resolution.X), STATIC_F32(Resolution.Y));
	}

	void CPlatformManager::UpdateWindowPos()
	{
		const SVector2<I16> currentCursorPos = GetScreenCursorPos();
		const SVector2<I16> delta = CursorPosPreDrag - currentCursorPos;

		constexpr U16 menuBarHeight = 18;
		if (WindowRelativeCursorPos.Y > menuBarHeight || delta.SizeSquared() == 0 || ResizeTarget.SizeSquared() > 0)
			return;

		// TODO.NW: Finally make mapping functions from one space to the other, also useful in "picking" logic
		const SVector2<U16> newResolution = IsFullscreen ? PreviousResolution : Resolution;
		const SVector2<I16> newWindowRelativeCursorPos = SVector2<I16>(STATIC_I16(NormalizedWindowRelativeCursorPos.X * STATIC_F32(newResolution.X)), STATIC_I16(NormalizedWindowRelativeCursorPos.Y * STATIC_F32(newResolution.Y)));
		const SVector2<I16> newPos = (currentCursorPos - newWindowRelativeCursorPos);

		UpdateWindow(newPos, newResolution);
	}

	void CPlatformManager::MinimizeWindow() const
	{
		PostMessage(WindowHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}

	void CPlatformManager::MaximizeWindow()
	{
		if (!IsFullscreen)
			UpdateWindow(SVector2<I16>(0), MaxResolution);
		else
			UpdateWindow(PreviousWindowPos, PreviousResolution);
	}

	void CPlatformManager::CloseWindow()
	{
		PostMessage(WindowHandle, WM_SYSCOMMAND, SC_CLOSE, 0);
	}

	void CPlatformManager::CloseSplashWindow()
	{
		PostMessage(SplashHandle, WM_SYSCOMMAND, SC_CLOSE, 0);
	}

	// TODO.NW: It doesn't seem like there's a windows API for snapping windows yet?
	void CPlatformManager::SnapWindow(const EWindowSnapPosition position)
	{
		SVector2<I16> windowPos = WindowPos;
		SVector2<U16> resolution = Resolution;

		switch (position)
		{
		case EWindowSnapPosition::Unsnapped:
			windowPos = PreviousWindowPos;
			resolution = PreviousResolution;
			SnapPosition = EWindowSnapPosition::Unsnapped;
			break;
		case EWindowSnapPosition::Left:
			windowPos = { 0 };
			resolution = SVector2<U16>(MaxResolution.X / 2, MaxResolution.Y);
			SnapPosition = EWindowSnapPosition::Left;
			break;
		case EWindowSnapPosition::Right:
			windowPos = SVector2<I16>(STATIC_I16(MaxResolution.X / 2), 0);
			resolution = SVector2<U16>(MaxResolution.X / 2, MaxResolution.Y);
			SnapPosition = EWindowSnapPosition::Right;
			break;
		case EWindowSnapPosition::Maximized:
			windowPos = { 0 };
			resolution = MaxResolution;
			SnapPosition = EWindowSnapPosition::Maximized;
			break;
		default:
			break;
		}

		UpdateWindow(windowPos, resolution);
	}
}
