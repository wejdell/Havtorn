// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"
#include "ECS/Entity.h"

namespace Havtorn
{
	class CScriptSystem : public ISystem
	{
	public:
		CScriptSystem();
		~CScriptSystem() override = default;
		ENGINE_API void Update(CScene* scene) override;
	};
}
