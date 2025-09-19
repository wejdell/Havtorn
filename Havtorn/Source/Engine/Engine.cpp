// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include <wincodec.h>

#include "Engine.h"
#include "FileSystem/FileWatcher.h"
#include "Threading/ThreadManager.h"
#include "Graphics/GraphicsFramework.h"
#include "Graphics/TextureBank.h"

#include "Scene/World.h"
#include "Scene/Scene.h"
#include "Assets/AssetRegistry.h"
#include "ECS/ECSInclude.h"

#include "Graphics/RenderManager.h"
#include "Input/InputMapper.h"

#include "Timer.h"
#include "Graphics/Debug/DebugDrawUtility.h"

#include "Application/EngineProcess.h"

#include <../Platform/PlatformManager.h>
#include <FileSystem.h>

namespace Havtorn
{
	GEngine* GEngine::Instance = nullptr;

	GEngine::GEngine()
	{
		Instance = this;

		Timer = new GTime();
		InputMapper = new CInputMapper();
		Framework = new CGraphicsFramework();
		RenderManager = new CRenderManager();
		AssetRegistry = new CAssetRegistry();
		World = new CWorld();
		ThreadManager = new CThreadManager();
		FileWatcher = new CFileWatcher();
		DebugDraw = new GDebugDraw();
	}

	GEngine::~GEngine()
	{
		SAFE_DELETE(DebugDraw);
		SAFE_DELETE(FileWatcher);
		SAFE_DELETE(ThreadManager);
		SAFE_DELETE(World);
		SAFE_DELETE(AssetRegistry);
		SAFE_DELETE(RenderManager);
		SAFE_DELETE(Framework);
		SAFE_DELETE(InputMapper);
		SAFE_DELETE(Timer);

		Instance = nullptr;
	}

	bool GEngine::Init(CPlatformManager* platformManager) 
	{
		ENGINE_ERROR_BOOL_MESSAGE(InputMapper->Init(platformManager), "Input Mapper could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(Framework->Init(platformManager), "Framework could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(RenderManager->Init(Framework, platformManager), "RenderManager could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(AssetRegistry->Init(RenderManager), "Asset Registry could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(World->Init(RenderManager), "World could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(ThreadManager->Init(RenderManager), "Thread Manager could not be initialized.");
		ENGINE_ERROR_BOOL_MESSAGE(FileWatcher->Init(ThreadManager), "File Watcher could not be initialized.");

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
		
		FileWatcher->FlushChanges();
		
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
			RenderManager->ReInit(Framework, WindowResizeTarget);
			WindowResizeTarget = {};
		}

		std::unique_lock<std::mutex> uniqueLock(CThreadManager::RenderMutex);
		CThreadManager::RenderThreadStatus = ERenderThreadStatus::ReadyToRender;
		uniqueLock.unlock();
		CThreadManager::RenderCondition.notify_one();
	}

	CFileWatcher* GEngine::GetFileWatcher()
	{
		return Instance->FileWatcher;
	}

	CThreadManager* GEngine::GetThreadManager()
	{
		return Instance->ThreadManager;
	}

	CAssetRegistry* GEngine::GetAssetRegistry()
	{
		return Instance->AssetRegistry;
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
}
