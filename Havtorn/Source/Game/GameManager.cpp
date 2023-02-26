// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "GameManager.h"

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
