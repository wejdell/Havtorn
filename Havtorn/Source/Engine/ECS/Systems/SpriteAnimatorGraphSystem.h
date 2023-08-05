// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn
{
	//SSpriteAnimatorGraphComponent
	class CSpriteAnimatorGraphSystem : public ISystem
	{
		CSpriteAnimatorGraphSystem();
		~CSpriteAnimatorGraphSystem() override;

		void Update(CScene* scene) override;
	};
}