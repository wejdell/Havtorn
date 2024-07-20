// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct STransformComponent : public SComponent
	{
		STransformComponent() = default;
		STransformComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		STransform Transform;

		// TODO.NR: Add Static/Dynamic modifiers
	};
}