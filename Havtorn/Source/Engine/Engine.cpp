// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include <wincodec.h>
#include <document.h>

#include "Engine.h"
#include "FileSystem/FileSystem.h"
#include "Application/WindowHandler.h"
#include "Threading/ThreadManager.h"
#include "Graphics/GraphicsFramework.h"
#include "Graphics/TextureBank.h"

#include "Scene/Scene.h"
#include "ECS/ECSInclude.h"

#include "Graphics/RenderManager.h"
#include "Input/InputMapper.h"

#include "Timer.h"

namespace Havtorn
{
	GEngine* GEngine::Instance = nullptr;

	GEngine::GEngine()
	{
		Instance = this;

		FileSystem = new CFileSystem();
		Timer = new GTime();
		WindowHandler = new CWindowHandler();
		Framework = new CGraphicsFramework();
		TextureBank = new CTextureBank();
		RenderManager = new CRenderManager();
		InputMapper = new CInputMapper();
		Scene = new CScene();
		ThreadManager = new CThreadManager();
	}

	GEngine::~GEngine()
	{
		SAFE_DELETE(ThreadManager);
		SAFE_DELETE(Scene);
		SAFE_DELETE(InputMapper);
		SAFE_DELETE(RenderManager);
		SAFE_DELETE(TextureBank);
		SAFE_DELETE(Framework);
		SAFE_DELETE(WindowHandler);
		SAFE_DELETE(Timer);
		SAFE_DELETE(FileSystem);

		Instance = nullptr;
	}

	bool GEngine::Init(const CWindowHandler::SWindowData& windowData)
	{
		ENGINE_ERROR_BOOL_MESSAGE(WindowHandler->Init(windowData), "Window Handler could not be initialized.");
		WindowHandler->SetInternalResolution();
		ENGINE_ERROR_BOOL_MESSAGE(Framework->Init(WindowHandler), "Framework could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(TextureBank->Init(Framework), "TextureBank could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(RenderManager->Init(Framework, WindowHandler), "RenderManager could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(InputMapper->Init(), "Input Mapper could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(Scene->Init(RenderManager), "Scene could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(ThreadManager->Init(RenderManager), "Thread Manager could not be initialized.");

		InitWindowsImaging();

		return true;
	}

	float GEngine::BeginFrame()
	{
		GTime::BeginTracking(ETimerCategory::CPU);
		return GTime::Mark();
	}

	void GEngine::Update()
	{
		InputMapper->Update();
		Scene->Update();

		GTime::EndTracking(ETimerCategory::CPU);

		std::unique_lock<std::mutex> uniqueLock(CThreadManager::RenderMutex);
		CThreadManager::RenderCondition.wait(uniqueLock, []
			{
				return CThreadManager::RenderThreadStatus == ERenderThreadStatus::PostRender;
			});
	}

	void GEngine::RenderFrame()
	{
	}

	void GEngine::EndFrame()
	{
		RenderManager->SwapRenderCommandBuffers();
		RenderManager->SwapStaticMeshInstancedRenderLists();
		Framework->EndFrame();

		std::unique_lock<std::mutex> uniqueLock(CThreadManager::RenderMutex);
		CThreadManager::RenderThreadStatus = ERenderThreadStatus::ReadyToRender;
		uniqueLock.unlock();
		CThreadManager::RenderCondition.notify_one();
	}

	CWindowHandler* GEngine::GetWindowHandler()
	{
		return Instance->WindowHandler;
	}

	CFileSystem* GEngine::GetFileSystem()
	{
		return Instance->FileSystem;
	}

	CTextureBank* GEngine::GetTextureBank()
	{
		return Instance->TextureBank;
	}

	CThreadManager* GEngine::GetThreadManager()
	{
		return Instance->ThreadManager;
	}

	CInputMapper* GEngine::GetInput()
	{
		return Instance->InputMapper;
	}

	void GEngine::InitWindowsImaging()
	{
#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
		Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
		if (FAILED(initialize))
			// error
#else
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr))
			// error
#endif
			return;
	}

#include <DbgHelp.h>
#include <strsafe.h>

	void GEngine::CrashWithScreenShot(std::wstring& /*subPath*/)
	{
		//DL_Debug::CDebug::GetInstance()->CopyToCrashFolder(aSubPath);

		//aSubPath += L"\\screenshot.bmp";
		//HRESULT hr = CoInitialize(nullptr);
		//hr = SaveWICTextureToFile(Framework->GetContext(), Framework->GetBackbufferTexture(),
		//	GUID_ContainerFormatBmp, aSubPath.c_str(),
		//	&GUID_WICPixelFormat16bppBGR565);

		//if (FAILED(hr)) {
		//	return;
		//}
		//CoUninitialize();
	}

	void GEngine::SetResolution(SVector2<F32> resolution)
	{
		WindowHandler->SetResolution(resolution);
		RenderManager->Release();
		RenderManager->ReInit(Framework, WindowHandler);
	}

	void GEngine::ShowCursor(const bool& isInEditorMode)
	{
		WindowHandler->ShowAndUnlockCursor(isInEditorMode);
	}
	void GEngine::HideCursor(const bool& isInEditorMode)
	{
		WindowHandler->HideAndLockCursor(isInEditorMode);
	}

}