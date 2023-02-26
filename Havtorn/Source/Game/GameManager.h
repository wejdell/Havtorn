// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "Havtorn.h"

namespace Havtorn
{
	class CGameManager
	{
	public:
		_declspec(dllexport) CGameManager();
		_declspec(dllexport) ~CGameManager();

		_declspec(dllexport) bool Init();

		_declspec(dllexport) void BeginFrame();
		_declspec(dllexport) void PreUpdate();
		_declspec(dllexport) void Update();
		_declspec(dllexport) void PostUpdate();
		_declspec(dllexport) void EndFrame();

	public:
		static _declspec(dllexport) CGameManager* Instance;
	};
}