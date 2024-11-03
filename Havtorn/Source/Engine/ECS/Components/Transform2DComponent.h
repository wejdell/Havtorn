// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct STransform2DComponent : public SComponent
	{
		STransform2DComponent() = default;
		STransform2DComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		SVector2<F32> Position = SVector2<F32>::Zero;
		SVector2<F32> Scale = { 1.f, 1.f };
		F32 DegreesRoll = 0;
	};
}