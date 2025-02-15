// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "Engine.h"
#include "WindowHandler.h"
#include "Graphics/GraphicsFramework.h"
#include "EngineProcess.h"

namespace Havtorn
{
	//CEngineProcess::WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	//CEngineProcess::WinProc = nullptr;
	CEngineProcess::CEngineProcess(U16 windowPosX, U16 windowPosY, U16 windowWidth, U16 windowHeight)
		: Engine(nullptr)
		, WindowPositionX(windowPosX)
		, WindowPositionY(windowPosY)
		, WindowWidth(windowWidth)
		, WindowHeight(windowHeight)
	{
	}

	CEngineProcess::~CEngineProcess()
	{
		SAFE_DELETE(Engine);
		HV_LOG_WARN("Engine shutdown!");
	}

	bool CEngineProcess::Init()
	{
		CWindowHandler::SWindowData windowData;
		windowData.X = WindowPositionX;
		windowData.Y = WindowPositionY;
		windowData.Width = WindowWidth;
		windowData.Height = WindowHeight;

		Engine = new GEngine();
		bool isValid = Engine->Init(windowData);
		Engine->SetEngineProcess(this);
		return isValid;
	}

	void CEngineProcess::HavtornWindowProc(IProcess* otherProcess)
	{
		OtherProcess = otherProcess;
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
			Engine->GetWindowHandler()->GetWindowHandle(),
			Engine->GetGraphicsFramework()->GetDevice(),
			Engine->GetGraphicsFramework()->GetContext()
		};
	}

	void CEngineProcess::WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (OtherProcess == nullptr)
			return;

		OtherProcess->WindowsWindowProc(hWnd, msg, wParam, lParam);
		//WinProc(hWnd, msg, wParam, lParam);
	}
}
