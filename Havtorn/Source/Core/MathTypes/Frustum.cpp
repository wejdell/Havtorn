// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Frustum.h"

namespace Havtorn
{
	SFrustumPlane::SFrustumPlane(const SVector& pointOnPlane, const SVector& planeNormal)
		: Normal(planeNormal.GetNormalized())
	{
		D = -Normal.Dot(pointOnPlane);
	}

	SFrustumPlane::SFrustumPlane(const SVector& point0, const SVector& point1, const SVector& point2)
	{
		Normal = SVector(point1 - point0).Cross(SVector(point2 - point0)).GetNormalized();
		D = -Normal.Dot(point0);
	}

	SFrustumPlane::SFrustumPlane(const SVector4& point0, const SVector4& point1, const SVector4& point2)
		: SFrustumPlane(point0.ToVector3(), point1.ToVector3(), point2.ToVector3())
	{}

	F32 SFrustumPlane::GetDistanceFromPlane(const SVector& point) const
	{
		return Normal.Dot(point) + D;
	}

	SFrustum::SFrustum(const SMatrix& viewMatrix, const SMatrix& projectionMatrix)
	{
		// Near: a, b, c, d
		// Far: e, f, g, h
		// Top: e, f, a, b
		// Bottom: g, h, c, d
		// Left: e, a, g, c
		// Right: b, f, d, h

		// NW: Idea from: https://gamedev.stackexchange.com/questions/29999/how-do-i-create-a-bounding-frustum-from-a-view-projection-matrix
		// Do we need to change this between D3D and Vulkan? Supposedly NDC Z coordinate runs from 0->1 in D3D and from -1->1 in OpenGL
		constexpr F32 xMin = -1.0f;
		constexpr F32 xMax = 1.0f;
		constexpr F32 yMin = -1.0f;
		constexpr F32 yMax = 1.0f;
		constexpr F32 zMin = 0.0f; // Potentially change between -1.0f and 0.0f based on rendering API?
		constexpr F32 zMax = 1.0f;

		SVector4 a = SVector4(xMin, yMax, zMin, 1.0f);
		SVector4 b = SVector4(xMax, yMax, zMin, 1.0f);
		SVector4 c = SVector4(xMin, yMin, zMin, 1.0f);
		SVector4 d = SVector4(xMax, yMin, zMin, 1.0f);
		SVector4 e = SVector4(xMin, yMax, zMax, 1.0f);
		SVector4 f = SVector4(xMax, yMax, zMax, 1.0f);
		SVector4 g = SVector4(xMin, yMin, zMax, 1.0f);
		//SVector4 h = SVector4(xMax, yMin, zMax, 1.0f);

		// NW: Go from NDC to projection space, then to world space
		const SMatrix invViewProj = (projectionMatrix.Inverse() * viewMatrix);

		a *= invViewProj;
		b *= invViewProj;
		c *= invViewProj;
		d *= invViewProj;
		e *= invViewProj;
		f *= invViewProj;
		g *= invViewProj;
		
		a /= a.W;
		b /= b.W;
		c /= c.W;
		d /= d.W;
		e /= e.W;
		f /= f.W;
		g /= g.W;

		Near = SFrustumPlane(a, b, c);
		Far = SFrustumPlane(e, g, f);
		Top = SFrustumPlane(e, f, a);
		Bottom = SFrustumPlane(c, d, g);
		Left = SFrustumPlane(g, e, c);
		Right = SFrustumPlane(b, f, d);
	}

	bool SFrustum::Intersects(const SSphere& sphere) const
	{
		if (Near.GetDistanceFromPlane(sphere.Center) > sphere.Radius)
			return false;

		if (Far.GetDistanceFromPlane(sphere.Center) > sphere.Radius)
			return false;

		if (Top.GetDistanceFromPlane(sphere.Center) > sphere.Radius)
			return false;

		if (Bottom.GetDistanceFromPlane(sphere.Center) > sphere.Radius)
			return false;

		if (Left.GetDistanceFromPlane(sphere.Center) > sphere.Radius)
			return false;

		if (Right.GetDistanceFromPlane(sphere.Center) > sphere.Radius)
			return false;

		return true;
	}
}
