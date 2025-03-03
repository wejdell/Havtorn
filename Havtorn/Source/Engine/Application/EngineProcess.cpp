// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "Engine.h"
//#include "WindowHandler.h"
#include "Graphics/GraphicsFramework.h"
#include "EngineProcess.h"

namespace Havtorn
{
	//CEngineProcess::WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	//CEngineProcess::WinProc = nullptr;
	CEngineProcess::CEngineProcess()
		: Engine(nullptr)
	{
	}

	CEngineProcess::~CEngineProcess()
	{
		SAFE_DELETE(Engine);
		HV_LOG_WARN("Engine shutdown!");
	}

	bool CEngineProcess::Init(CPlatformManager* platformManager)
	{
		Engine = new GEngine();
		bool isValid = Engine->Init(platformManager);
		return isValid;
	}

	void CEngineProcess::BeginFrame()
	{
		Engine->BeginFrame();
	}

	void CEngineProcess::PreUpdate()
	{

	}

	void CEngineProcess::Update()
	{
		Engine->Update();
	}

	void CEngineProcess::PostUpdate()
	{
		Engine->RenderFrame();
	}

	void CEngineProcess::EndFrame()
	{
		Engine->EndFrame();
	}

	SEngineRenderBackend CEngineProcess::GetRenderBackend()
	{
		return
		{
			0,
			Engine->GetGraphicsFramework()->GetDevice(),
			Engine->GetGraphicsFramework()->GetContext()
		};
	}
}
