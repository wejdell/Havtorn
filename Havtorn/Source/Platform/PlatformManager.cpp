// Copyright 2022 Team Havtorn. All Rights Reserved.

#include <hvpch.h>

#include "PlatformManager.h"

//#include "Input/Input.h"
#include <string>
#include <vector>

#include <Log.h>

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
				char file[MAX_PATH];
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
		UnregisterClass(L"HavtornWindowClass", GetModuleHandle(nullptr));
	}

	bool CPlatformManager::Init(CPlatformManager::SWindowData someWindowData)
	{
		WindowData = someWindowData;

		//rapidjson::Document document = CJsonReader::Get()->LoadDocument("Json/Settings/WindowSettings.json");

		//if (document.HasMember("Window Width"))
		//    WindowData.Width = document["Window Width"].GetInt();

		//if (document.HasMember("Window Height"))
		//    WindowData.Height = document["Window Height"].GetInt();

		//if (document.HasMember("Window Starting Pos X"))
		//    WindowData.X = document["Window Starting Pos X"].GetInt();

		//if (document.HasMember("Window Starting Pos Y"))
		//    WindowData.Y = document["Window Starting Pos Y"].GetInt();

		//HCURSOR customCursor = NULL;
		//if (document.HasMember("Cursor Path"))
		//    customCursor = LoadCursorFromFileA(document["Cursor Path"].GetString());

		//if (customCursor == NULL)
		//    customCursor = LoadCursor(nullptr, IDC_ARROW);

		//HICON customIcon = NULL;
		//if (document.HasMember("Icon Path"))
		//    customIcon = (HICON)LoadImageA(NULL, document["Icon Path"].GetString(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);

		//if (customIcon == NULL)
		//    customIcon = (HICON)LoadImageA(NULL, "ironwrought.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);

		WNDCLASSEX windowclass = {};
		windowclass.cbSize = sizeof(WNDCLASSEX);
		windowclass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
		windowclass.lpfnWndProc = CPlatformManager::WinProc;
		windowclass.cbClsExtra = 0;
		windowclass.cbWndExtra = 0;
		windowclass.hInstance = GetModuleHandle(nullptr);
		//windowclass.hIcon = customIcon;
		//windowclass.hCursor = customCursor;
		windowclass.lpszClassName = L"HavtornWindowClass";
		RegisterClassEx(&windowclass);

		std::string gameName = "Havtorn Editor";
		//if (document.HasMember("Game Name"))
		//{
		//    gameName = document["Game Name"].GetString();
		//}

		//bool borderless = false;
		//if (document.HasMember("Borderless Window"))
		//{
		//    borderless = document["Borderless Window"].GetBool();
		//}

		//MaxResX = GetSystemMetrics(SM_CXSCREEN);
		//MaxResY = GetSystemMetrics(SM_CYSCREEN);

		if (false/*borderless*/)
		{
#ifdef _DEBUG
			// Start in borderless
			WindowHandle = CreateWindowA("HavtornWindowClass", gameName.c_str(),
				WS_POPUP | WS_VISIBLE,
				0, 0, WindowData.Width, WindowData.Height,
				NULL, NULL, GetModuleHandle(nullptr), this);
#else
			// Start in borderless
			WindowHandle = CreateWindowA("HavtornWindowClass", gameName.c_str(),
				WS_POPUP | WS_VISIBLE,
				0, 0, MaxResX, MaxResY,
				NULL, NULL, GetModuleHandle(nullptr), this);
#endif
		}
		else
		{
			// Start in bordered window
			WindowHandle = CreateWindowA("HavtornWindowClass", gameName.c_str(),
				WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE,
				//WS_POPUP | WS_VISIBLE,
				WindowData.X, WindowData.Y, WindowData.Width, WindowData.Height,
				nullptr, nullptr, nullptr, this);
		}

		//::SetCursor(customCursor);

		//LockCursor(true);

		//HBRUSH hBrush = CreateSolidBrush(RGB(255, 153, 0));

		//MENUINFO mi = { 0 };
		//mi.cbSize = sizeof(mi);
		//mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
		//mi.hbrBack = hBrush;

		//HMENU hMenu = ::GetMenu(WindowHandle);
		//SetMenuInfo(hMenu, &mi);

		Resolution = { WindowData.Width, WindowData.Height };
		ResizeTarget = {};
		SetResolution(Resolution);
		/*SetInternalResolution();*/

		EnableDragDrop();

		InitWindowsImaging();

		return true;
	}

	const HWND CPlatformManager::GetWindowHandle() const
	{
		return WindowHandle;
	}

	SVector2<U16> CPlatformManager::GetCenterPosition() const
	{
		SVector2<U16> center = {};
		RECT rect = { 0 };
		if (GetWindowRect(WindowHandle, &rect))
		{
			center.X = STATIC_U16((rect.right - rect.left) / (U16)2);
			center.Y = STATIC_U16((rect.bottom - rect.top) / (U16)2);
		}
		return center;
	}

	SVector2<U16> CPlatformManager::GetResolution() const
	{
		return Resolution;
	}

	void CPlatformManager::SetResolution(SVector2<U16> resolution)
	{
		if ((I16)resolution.X <= MaxResX && (I16)resolution.Y <= MaxResY)
		{
			::SetWindowPos(WindowHandle, 0, 0, 0, resolution.X, resolution.Y, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
			SetInternalResolution();
		}
	}

	void CPlatformManager::InitWindowsImaging()
	{
//#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
//		Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
//		if (FAILED(initialize))
//			// error
//
//#else
//		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
//		if (FAILED(hr))
//			// error
//#endif
			return;
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
			SVector2<U16> center = GetCenterPosition();
			SetCursorPos(static_cast<I16>(center.X), static_cast<I16>(center.Y));
		}
		shouldLock ? static_cast<bool>(SetCapture(WindowHandle)) : static_cast<bool>(ReleaseCapture());
	}

	void CPlatformManager::HideAndLockCursor(const bool& isInEditorMode)
	{
		while (::ShowCursor(FALSE) >= 0);
		SetCapture(WindowHandle);

		CursorIsLocked = true;
		WindowIsInEditingMode = isInEditorMode;

		SVector2<U16> center = GetCenterPosition();
		SetCursorPos(static_cast<I16>(center.X), static_cast<I16>(center.Y));
	}

	void CPlatformManager::ShowAndUnlockCursor(const bool& isInEditorMode)
	{
		while (::ShowCursor(TRUE) < 0);
		ReleaseCapture();

		CursorIsLocked = false;
		WindowIsInEditingMode = isInEditorMode;
	}

	void CPlatformManager::SetInternalResolution()
	{
		if (ResizeTarget.LengthSquared() > 0)
		{
			SVector2<U16> newResolution = SVector2<U16>(STATIC_U16(ResizeTarget.X), STATIC_U16(ResizeTarget.Y));
			Resolution = newResolution;
			OnResolutionChanged.Broadcast(newResolution);
			ResizeTarget = { 0, 0 };
		}
	}

	PLATFORM_API void CPlatformManager::UpdateResolution()
	{
		SetInternalResolution();
	}

	void CPlatformManager::SetWindowTitle(const std::string& title)
	{
		SetWindowTextA(WindowHandle, title.c_str());
	}

	void CPlatformManager::EnableDragDrop() const
	{
		DragAcceptFiles(WindowHandle, TRUE);
	}

	void CPlatformManager::DisableDragDrop() const
	{
		DragAcceptFiles(WindowHandle, FALSE);
	}
}
