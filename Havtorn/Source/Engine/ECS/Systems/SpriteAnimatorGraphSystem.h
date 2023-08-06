// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"
#include "Input/InputTypes.h"

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
		void HandleAxisInput(const SInputAxisPayload payload);
		void ResetInput();

		F32 horizontal = 0.0f;
	};
}