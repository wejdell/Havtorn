// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "Engine.h"
#include "WindowHandler.h"

#include "EngineProcess.h"

namespace Havtorn
{
	CEngineProcess::CEngineProcess(U16 windowPosX, U16 windowPosY, U16 windowWidth, U16 windowHeight)
		: Engine(nullptr)
		, WindowPositionX(windowPosX)
		, WindowPositionY(windowPosY)
		, WindowWidth(windowWidth)
		, WindowHeight(windowHeight)
	{}
	
	CEngineProcess::~CEngineProcess()
	{
		SAFE_DELETE(Engine);
	}
	
	bool CEngineProcess::Init()
	{
		CWindowHandler::SWindowData windowData;
		windowData.X = WindowPositionX;
		windowData.Y = WindowPositionY;
		windowData.Width = WindowWidth;
		windowData.Height = WindowHeight;

		Engine = new CEngine();
		return Engine->Init(windowData);
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
}
