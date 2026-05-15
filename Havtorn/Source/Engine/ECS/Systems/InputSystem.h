// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn
{
	class CInputSystem final : public ISystem
	{
	public:
		ENGINE_API CInputSystem();
		ENGINE_API ~CInputSystem() override;

		ENGINE_API void Update(std::vector<Ptr<CScene>>& scenes) override;
	};
}
