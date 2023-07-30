// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct STransform2DComponent : public SComponent
	{
		STransform2DComponent()
			: SComponent(EComponentType::Transform2DComponent)
		{}

		SVector2<F32> Position = SVector2<F32>::Zero;
		SVector2<F32> Scale = SVector2<F32>::Zero;
		F32 DegreesRoll = 0;
	};
}