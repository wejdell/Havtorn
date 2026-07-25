// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "PlatformProcess.h"
#include "PlatformManager.h"

namespace Havtorn
{
	CPlatformProcess::CPlatformProcess()
		: PlatformManager(nullptr)
	{}

	CPlatformProcess::~CPlatformProcess()
	{
		SAFE_DELETE(PlatformManager);
	}

	bool CPlatformProcess::Init(CPlatformManager* /*platformManager*/)
	{
		if (PlatformManager != nullptr)
			return true;

		PlatformManager = new CPlatformManager();
		return PlatformManager->Init();
	}

	void CPlatformProcess::OnApplicationReady()
	{
		PlatformManager->OnApplicationReady();
	}

	bool CPlatformProcess::ShouldRun()
	{
		return PlatformManager->ShouldRun;
	}

	void CPlatformProcess::BeginFrame()
	{
		PlatformManager->BeginFrame();
	}

	void CPlatformProcess::EndFrame()
	{
		PlatformManager->EndFrame();
	}
}
