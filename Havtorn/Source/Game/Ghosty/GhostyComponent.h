// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SGhostyState
	{
		SVector Input;
		bool IsInWalkingAnimationState;
	};

	struct SGhostyComponent : SComponent
	{
		SGhostyComponent() 
			: SComponent(EComponentType::GhostyComponent)
		{}
		~SGhostyComponent() override = default;

		SGhostyState State;
	};
}