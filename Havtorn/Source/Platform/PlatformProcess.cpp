// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "PlatformProcess.h"
#include "PlatformManager.h"

namespace Havtorn
{
	CPlatformProcess::CPlatformProcess(U16 windowPosX, U16 windowPosY, U16 windowWidth, U16 windowHeight)
		: PlatformManager(nullptr)
		, WindowPositionX(windowPosX)
		, WindowPositionY(windowPosY)
		, WindowWidth(windowWidth)
		, WindowHeight(windowHeight)
	{}

	CPlatformProcess::~CPlatformProcess()
	{
		SAFE_DELETE(PlatformManager);
	}

	bool CPlatformProcess::Init(CPlatformManager* /*platformManager*/)
	{
		if (PlatformManager != nullptr)
			return true;

		CPlatformManager::SWindowData windowData;
		windowData.X = WindowPositionX;
		windowData.Y = WindowPositionY;
		windowData.Width = WindowWidth;
		windowData.Height = WindowHeight;

		PlatformManager = new CPlatformManager();
		return PlatformManager->Init(windowData);
	}

	void CPlatformProcess::BeginFrame()
	{
		//PlatformManager->BeginFrame();
	}

	void CPlatformProcess::PreUpdate()
	{
		//PlatformManager->PreUpdate();
	}

	void CPlatformProcess::Update()
	{
		//PlatformManager->Update();
	}

	void CPlatformProcess::PostUpdate()
	{
		//PlatformManager->PostUpdate();
	}

	void CPlatformProcess::EndFrame()
	{
		//PlatformManager->EndFrame();
	}
}
