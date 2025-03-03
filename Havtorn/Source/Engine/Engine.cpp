// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include <wincodec.h>
#include <document.h>

#include "Engine.h"
#include "FileSystem/FileSystem.h"
#include "Threading/ThreadManager.h"
#include "Graphics/GraphicsFramework.h"
#include "Graphics/TextureBank.h"

#include "Scene/World.h"
#include "Scene/Scene.h"
#include "Scene/AssetRegistry.h"
#include "ECS/ECSInclude.h"

#include "Graphics/RenderManager.h"
#include "Input/InputMapper.h"

#include "Timer.h"
#include "Graphics/Debug/DebugDrawUtility.h"

#include "Application/EngineProcess.h"

#include <../Platform/PlatformManager.h>

namespace Havtorn
{
	GEngine* GEngine::Instance = nullptr;

	GEngine::GEngine()
	{
		Instance = this;

		FileSystem = new CFileSystem();
		Timer = new GTime();
		InputMapper = new CInputMapper();
		Framework = new CGraphicsFramework();
		TextureBank = new CTextureBank();
		RenderManager = new CRenderManager();
		World = new CWorld();
		ThreadManager = new CThreadManager();
		DebugDraw = new GDebugDraw();
	}

	GEngine::~GEngine()
	{
		SAFE_DELETE(ThreadManager);
		SAFE_DELETE(World);
		SAFE_DELETE(DebugDraw);
		SAFE_DELETE(RenderManager);
		SAFE_DELETE(TextureBank);
		SAFE_DELETE(Framework);
		SAFE_DELETE(InputMapper);
		SAFE_DELETE(Timer);
		SAFE_DELETE(FileSystem);

		Instance = nullptr;
	}

	bool GEngine::Init(CPlatformManager* platformManager) 
	{
		ENGINE_ERROR_BOOL_MESSAGE(InputMapper->Init(platformManager), "Input Mapper could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(Framework->Init(platformManager), "Framework could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(TextureBank->Init(Framework), "TextureBank could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(RenderManager->Init(Framework, platformManager), "RenderManager could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(World->Init(RenderManager), "World could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(ThreadManager->Init(RenderManager), "Thread Manager could not be initialized.");

		SequencerSystem = World->GetSystem<CSequencerSystem>();

		platformManager->OnResolutionChanged.AddMember(this, &GEngine::OnWindowResolutionChanged);

		if (!DebugDraw->Init(RenderManager))
		{
			HV_LOG_WARN("Debug Draw Utility could not be initialized.");
		}

		return true;
	}

	float GEngine::BeginFrame()
	{
		// TODO.AG/AS: Do the empty the editor-changes-queue here
		GTime::BeginTracking(ETimerCategory::CPU);
		return GTime::Mark();
	}

	void GEngine::Update()
	{
		InputMapper->Update();
		World->Update();
		DebugDraw->Update();
		
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
		RenderManager->SyncCrossThreadResources(World);
		Framework->EndFrame();

		if (WindowResizeTarget.LengthSquared() > 0)
		{
			RenderManager->Release(WindowResizeTarget);
			
			//replace w/ Resize To ResizeTarget
			//WindowHandler->SetResolution(WindowHandler->ResizeTarget);
			//WindowHandler->SetInternalResolution();

			//WindowHandler->ResizeTarget = { };
			RenderManager->ReInit(Framework, WindowResizeTarget);
			WindowResizeTarget = {};
		}

		std::unique_lock<std::mutex> uniqueLock(CThreadManager::RenderMutex);
		CThreadManager::RenderThreadStatus = ERenderThreadStatus::ReadyToRender;
		uniqueLock.unlock();
		CThreadManager::RenderCondition.notify_one();
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

	CWorld* GEngine::GetWorld()
	{
		return Instance->World;
	}

	CInputMapper* GEngine::GetInput()
	{
		return Instance->InputMapper;
	}

	CGraphicsFramework* GEngine::GetGraphicsFramework()
	{
		return Instance->Framework;
	}

	void GEngine::OnWindowResolutionChanged(SVector2<U16> newResolution)
	{
		WindowResizeTarget = newResolution;
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

	//void GEngine::SetResolution(SVector2<F32> resolution)
	//{
	//	WindowHandler->SetResolution(resolution);
	//	RenderManager->Release();
	//	RenderManager->ReInit(Framework, WindowHandler);
	//}

	//void GEngine::ShowCursor(const bool& isInEditorMode)
	//{
	//	WindowHandler->ShowAndUnlockCursor(isInEditorMode);
	//}
	//void GEngine::HideCursor(const bool& isInEditorMode)
	//{
	//	WindowHandler->HideAndLockCursor(isInEditorMode);
	//}

}