// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn
{
	class CDebugUtilitySystem final : public ISystem
	{
	public:
		CDebugUtilitySystem();
		~CDebugUtilitySystem() override;

		void Update(CScene* scene) override;

	private:
		void CheckDebugShapesForRendering(CScene* scene);

	};
}