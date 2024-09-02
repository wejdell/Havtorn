// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SGhostyState
	{
		SVector Input;
		F32 MoveSpeed = 3.145f;
		bool IsInWalkingAnimationState = false;
	};

	struct SGhostyComponent : SComponent
	{
		SGhostyComponent() = default;
		SGhostyComponent(const SEntity& entityOwner) 
			: SComponent(entityOwner)
		{}
		~SGhostyComponent() override = default;

		SGhostyState State;
	};
}