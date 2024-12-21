// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Vector.h"

namespace Havtorn 
{
	struct SRay 
	{
		// Default constructor: there is no ray, the origin and direction are the
        // zero vector.
		SRay();
		// Copy constructor.
		SRay(const SRay& ray);
		// Constructor that takes two points that define the ray. Note that the direction is
		// given by point - origin.
		SRay(const SVector& origin, const SVector& point);
		// For a given distance value from the origin, returns the corresponding point on the ray.
		SVector GetPointOnRay(F32 distanceFromOrigin) const;

		SVector Origin;
		SVector Direction;
	};

	inline SRay::SRay() : Origin(SVector()), Direction(SVector()) {}

	inline SRay::SRay(const SRay& ray) 
	{
		Origin = ray.Origin;
		Direction = ray.Direction;
	}

	inline SRay::SRay(const SVector& origin, const SVector& point) 
	{
		Origin = origin;
		Direction = point - origin;
	}

	inline SVector SRay::GetPointOnRay(F32 distanceFromOrigin) const
	{
		return Origin + Direction * distanceFromOrigin;
	}
}