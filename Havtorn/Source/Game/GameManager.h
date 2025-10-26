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
		GAME_API void OnApplicationReady();

		GAME_API void BeginFrame();
		GAME_API void PreUpdate();
		GAME_API void Update();
		GAME_API void PostUpdate();
		GAME_API void EndFrame();

		void OnBeginPlay(std::vector<Ptr<CScene>>& scenes);
		void OnPausePlay(std::vector<Ptr<CScene>>& scenes);
		void OnEndPlay(std::vector<Ptr<CScene>>& scenes);

	public:
		static GAME_API CGameManager* Instance;
		CWorld* World = nullptr;
	};
}