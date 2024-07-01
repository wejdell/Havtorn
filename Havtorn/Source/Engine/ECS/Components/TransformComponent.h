// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct STransformComponent : public SComponent
	{
		STransformComponent()
			: SComponent(EComponentType::TransformComponent)
		{}

		STransform Transform;

		// TODO.NR: Add Static/Dynamic modifiers
	};
}