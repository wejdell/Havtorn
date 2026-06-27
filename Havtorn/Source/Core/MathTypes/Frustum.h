// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Sphere.h"

namespace Havtorn
{
	struct CORE_API SFrustumPlane
	{
		SFrustumPlane() = default;
		SFrustumPlane(const SVector& pointOnPlane, const SVector& planeNormal);
		SFrustumPlane(const SVector& point0, const SVector& point1, const SVector& point2);
		SFrustumPlane(const SVector4& point0, const SVector4& point1, const SVector4& point2);

		// Calculates the perpendicular distance from the plane. 0 for points on the plane, positive for points outside the plane, negative for points inside the plane.
		F32 GetDistanceFromPlane(const SVector& point) const;
		
		SVector Normal = SVector::Zero;
		
		// Perpendicular distance from the plane to the origin
		F32 D = 0.0f;
	};

	struct CORE_API SFrustum
	{
		SFrustum() = delete;
		SFrustum(const SMatrix& viewMatrix, const SMatrix& projectionMatrix);
		bool Intersects(const SSphere& sphere) const;

		SFrustumPlane Near;
		SFrustumPlane Far;
		SFrustumPlane Top;
		SFrustumPlane Bottom;
		SFrustumPlane Left;
		SFrustumPlane Right;
	};
}
