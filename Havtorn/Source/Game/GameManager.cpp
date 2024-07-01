// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "GameManager.h"
#include "Ghosty\GhostySystem.h"

#include <Engine.h>

namespace Havtorn
{
	CGameManager* CGameManager::Instance = nullptr;

	CGameManager::CGameManager()
	{
		Instance = this;
	}

	CGameManager::~CGameManager()
	{
		Instance = nullptr;
	}

	bool CGameManager::Init()
	{
		HV_LOG_INFO("GameManager Initialized.");
		GEngine::GetWorld()->RegisterSystem(std::make_unique<Havtorn::CGhostySystem>());

		return true;
	}

	void CGameManager::BeginFrame()
	{
	}

	void CGameManager::PreUpdate()
	{
	}

	void CGameManager::Update()
	{
	}

	void CGameManager::PostUpdate()
	{
	}

	void CGameManager::EndFrame()
	{
	}
}