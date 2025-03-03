// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "GameProcess.h"

#include "GameManager.h"
#include "Havtorn.h"

namespace Havtorn
{
	CGameProcess::CGameProcess()
		: GameManager(nullptr)
	{}

	CGameProcess::~CGameProcess()
	{
		SAFE_DELETE(GameManager);
		HV_LOG_INFO("Game shutdown!");
	}

	bool CGameProcess::Init(CPlatformManager* /*platformManager*/)
	{
		GameManager = new CGameManager();

		auto havtornEngine = GEngine::Instance;
		if (havtornEngine == nullptr)
		{
			HV_ASSERT(havtornEngine != nullptr, "Couldn't find Havtorn Engine!");
			return false;
		}

		return GameManager->Init();
	}

	void CGameProcess::BeginFrame()
	{
		GameManager->BeginFrame();
	}

	void CGameProcess::PreUpdate()
	{
		GameManager->PreUpdate();
	}

	void CGameProcess::Update()
	{
		GameManager->Update();
	}

	void CGameProcess::PostUpdate()
	{
		GameManager->PostUpdate();
	}

	void CGameProcess::EndFrame()
	{
		GameManager->EndFrame();
	}
}
