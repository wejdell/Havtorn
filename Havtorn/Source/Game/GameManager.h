// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "Havtorn.h"

namespace Havtorn
{
	class CGameManager
	{
	public:
		GAME_API CGameManager();
		GAME_API ~CGameManager();

		GAME_API bool Init();

		GAME_API void BeginFrame();
		GAME_API void PreUpdate();
		GAME_API void Update();
		GAME_API void PostUpdate();
		GAME_API void EndFrame();

		void OnBeginPlay(CScene* scene);
		void OnPausePlay(CScene* scene);
		void OnEndPlay(CScene* scene);

	public:
		static GAME_API CGameManager* Instance;
		CWorld* World;
	};
}