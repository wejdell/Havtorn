// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ImGuiProcess.h"

#include "ImGuiManager.h"
#include "Engine.h"
#include "Application/WindowHandler.h"
#include "Graphics/GraphicsFramework.h"

namespace Havtorn
{
	CImGuiProcess::CImGuiProcess()
	{
		ImGuiManager = new GImGuiManager();
	}

	CImGuiProcess::~CImGuiProcess()
	{
		SAFE_DELETE(ImGuiManager);
	}

	bool CImGuiProcess::Init()
	{
		GEngine* engineInstance = GEngine::Instance;
		HV_ASSERT(engineInstance != nullptr, "GEngine::Instance is nullptr! Cannot init ImGuiProcess.");
		if (!engineInstance)
		{
			return false;
		}
		
		return ImGuiManager->WindowsInitDX11(engineInstance->WindowHandler->GetWindowHandle(), engineInstance->Framework->GetDevice(), engineInstance->Framework->GetContext());
	}

	void CImGuiProcess::BeginFrame()
	{
		ImGuiManager->NewFrame();
	}
	
	void CImGuiProcess::PreUpdate()
	{}
	
	void CImGuiProcess::Update()
	{}
	
	void CImGuiProcess::PostUpdate()
	{
		ImGuiManager->EndFrame();
	}
	
	void CImGuiProcess::EndFrame()
	{
		
	}
}
