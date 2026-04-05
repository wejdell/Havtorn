// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn
{
	class CAbilitySystem final : public ISystem
	{
	public:
		CAbilitySystem() = default;
		~CAbilitySystem() override = default;

		ENGINE_API void Update(std::vector<Ptr<CScene>>& scenes) override;
	};
}
