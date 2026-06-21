// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "Sphere.h"

namespace Havtorn 
{
	SSphere::SSphere(const SVector& center, F32 radius)
		: Center(center)
		, Radius(radius)
	{
	}

	bool SSphere::IsInside(const SVector& position) const
	{
		const F32 distanceSquared = Center.DistanceSquared(position);
		return distanceSquared <= UMath::Square(Radius);
	}

	bool SSphere::Intersects(const SSphere& sphere) const
	{
		const F32 distanceSquared = Center.DistanceSquared(sphere.Center);
		return distanceSquared <= UMath::Square(Radius + sphere.Radius);
	}
}
