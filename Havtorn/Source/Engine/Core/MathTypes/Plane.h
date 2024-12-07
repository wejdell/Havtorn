// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Vector.h"

namespace Havtorn 
{
	class SPlane 
	{
	public:
		// Default constructor.
		SPlane();
		// Constructor taking three points where the normal is (point1 - point0) x (point2 -point0).
		SPlane(const SVector& point0, const SVector& point1, const SVector& point2);
		// Constructor taking a point and a normal.
		SPlane(const SVector& point0, const SVector& normal);
		// Init the plane with three points, the same as the constructor above.
		void Init(const SVector& point0, const SVector& point1, const SVector& point2);
		// Init the plane with a point and a normal, the same as the constructor above.
		void Init(const SVector& point, const SVector& normal);
		// Returns whether a point is inside the plane: it is inside when the point is on the plane or on the side the normal is pointing away from.
		bool IsInside(const SVector&  position) const;
		// Returns the normal of the plane.
		const SVector& GetNormal() const;
		const SVector& GetPoint() const;
	private:
		SVector Point;
		SVector Normal;
	};

	SPlane::SPlane() : Point(SVector()), Normal(SVector()) {}

	SPlane::SPlane(const SVector& point0, const SVector& point1, const SVector& point2) 
		: Point(point0) 
	{
		Normal = SVector(point1 - point0).Cross(SVector(point2 - point0)).GetNormalized();
	}

	SPlane::SPlane(const SVector& point0, const SVector& normal) : Point(point0), Normal(normal) {}

	void SPlane::Init(const SVector& point0, const SVector& point1, const SVector& point2) 
	{
		Point = point0;
		Normal = SVector(point1 - point0).Cross(SVector(point2 - point0)).GetNormalized();
	}

	void SPlane::Init(const SVector& point0, const SVector& normal) 
	{
		Point = point0;
		Normal = normal;
	}

	bool SPlane::IsInside(const SVector& position) const 
	{
		return SVector(position - Point).Dot(Normal) <= 0;
	}

	const SVector& SPlane::GetNormal() const 
	{
		return Normal;
	}
	
	const SVector& SPlane::GetPoint() const 
	{
		return Point;
	}
}