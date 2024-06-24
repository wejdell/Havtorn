// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn
{
	struct SSpriteAnimatorGraphComponent;
	class CSpriteAnimatorGraphSystem : public ISystem
	{
	public:
		CSpriteAnimatorGraphSystem();
		~CSpriteAnimatorGraphSystem() override;
		void Update(CScene* scene) override;

	private:
		SVector4 TickAnimationClip(SSpriteAnimatorGraphComponent& component, const F32 deltaTime);
	};
}