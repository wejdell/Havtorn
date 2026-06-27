// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Vector.h"

namespace Havtorn 
{
	struct CORE_API SSphere 
	{
		SSphere() = default;
		SSphere(const SVector& center, const F32 radius);

		bool IsInside(const SVector& position) const;
		bool Intersects(const SSphere& sphere) const;

		SVector Center = SVector::Zero;
		F32 Radius = 0.0f;
	};
}
