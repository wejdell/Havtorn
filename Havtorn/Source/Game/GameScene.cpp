// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GameScene.h"
#include "Engine.h"

namespace Havtorn
{
	bool CGameScene::Init(const std::string& sceneName)
	{
		if (!CScene::Init(sceneName))
			return false;

		// Register your components here

		return true;
	}

	void CGameScene::OpenDefault()
    {
		CScene::OpenDefault();
    }
}
