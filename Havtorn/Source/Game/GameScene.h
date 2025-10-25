// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include <Scene\Scene.h>

namespace Havtorn
{
    class CGameScene : public CScene
    {
    public:
		GAME_API virtual bool Init(const std::string& sceneName) override;

		GAME_API virtual bool Init3DDemoScene() override;
		GAME_API virtual bool Init2DDemoScene() override;
    };
}

