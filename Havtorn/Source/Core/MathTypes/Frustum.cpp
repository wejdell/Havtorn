// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Frustum.h"
#include "MathUtilities.h"

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

	bool SFrustum::Intersects(const SFrustum& frustum) const
	{
		// Using basic Separating Axes Theorem for frustum-frustum intersection, can
		// be used for any convex polygons
		// https://en.wikipedia.org/wiki/Hyperplane_separation_theorem#Use_in_collision_detection
		// https://www.geometrictools.com/Documentation/IntersectionBox3Frustum3.pdf
		
		// TODO.NW: We'd probably want to calculate these on construction if they are used often.
		// See if it's worth the increased memory footprint
		const SFrustumConstituents constituents = GetConstituents();
		const SFrustumConstituents otherConstituents = frustum.GetConstituents();

		std::vector<SVector> axes = constituents.PlaneNormals;
		axes.append_range(otherConstituents.PlaneNormals);

		// If we don't check edges, we miss cases where all vertices of both 
		// objects are outside of each other, but where edges intersect
		for (const SVector& edgeA : constituents.Edges) 
		{
			for (const SVector& edgeB : otherConstituents.Edges) 
			{
				const SVector cross = edgeA.Cross(edgeB);
				if (cross.LengthSquared() > PRETTY_SMALL_NUMBER) 
					axes.push_back(cross.GetNormalized());
			}
		}

		for (const SVector& axis : axes) 
		{
			const SVector2<F32> boundsA = UMathUtilities::FindProjectionBoundsOnAxis(constituents.Vertices, axis);
			const SVector2<F32> boundsB = UMathUtilities::FindProjectionBoundsOnAxis(otherConstituents.Vertices, axis);

			// Check for lack of projection overlap (separation)
			if (boundsA.Y < (boundsB.X - SMALL_NUMBER) || boundsB.Y < (boundsA.X - SMALL_NUMBER)) 
				return false;
		}

		// No separating axis found - meaning no disjointed projections found - objects intersect
		return true;

		// TODO.NW: Have a look at this somewhat different better performing version
		//https://mugen87.github.io/yuka/docs/math_SAT.js.html
		//// For each face normal: find the single support vertex of B in the
		//// opposite direction, then test distance to A's face plane. O(1) per axis.
		//bool checkFace(const Vector3& faceNormal, const Convex& A, const Convex& B,
		//               const Plane& facePlane) {
		//    // Find support vertex of B in direction -faceNormal
		//    Vector3 opp = faceNormal * -1.0f;
		//    float best = -FLT_MAX;
		//    Vector3 support;
		//    for (int i = 0; i < B.numVertices; i++) {
		//        float d = B.vertices[i].dot(opp);
		//        if (d > best) { best = d; support = B.vertices[i]; }
		//    }
		//    // Distance from support vertex to A's face plane
		//    return facePlane.distanceToPoint(support) > 0; // separated
		//}
		//
		//// For edge-edge axes: use the Minkowski face test to prune
		//// non-relevant edge pairs before computing the cross product.
		//bool minkowskiFace(const Edge& eA, const Edge& eB) {
		//    // Get the two face normals adjacent to each edge
		//    // Compute triple products to determine if the edge pair
		//    // contributes a face on the Minkowski difference
		//    float cba = eB.faceNormalA.dot(eA.dir);
		//    float dba = eB.faceNormalB.dot(eA.dir);
		//    float adc = eA.faceNormalA.dot(eB.dir);
		//    float bdc = eA.faceNormalB.dot(eB.dir);
		//    return (cba * dba < 0) && (adc * bdc < 0) && (cba * bdc > 0);
		//}
	}

	SFrustumConstituents SFrustum::GetConstituents() const
	{
		const SVector v0 = UMathUtilities::IntersectPlanes(Near, Left, Bottom);
		const SVector v1 = UMathUtilities::IntersectPlanes(Near, Left, Top);
		const SVector v2 = UMathUtilities::IntersectPlanes(Near, Right, Top);
		const SVector v3 = UMathUtilities::IntersectPlanes(Near, Right, Bottom);
		const SVector v4 = UMathUtilities::IntersectPlanes(Far, Left, Bottom);
		const SVector v5 = UMathUtilities::IntersectPlanes(Far, Left, Top);
		const SVector v6 = UMathUtilities::IntersectPlanes(Far, Right, Top);
		const SVector v7 = UMathUtilities::IntersectPlanes(Far, Right, Bottom);

		const SVector nearBottomTop = v1 - v0;
		const SVector nearLeftRight = v3 - v0;
		const SVector nearLeftFar = v4 - v0;
		const SVector farTopBottom = v7 - v6;
		const SVector farRightLeft = v5 - v6;
		const SVector farRightNear = v2 - v6;

		return 
		{ 
			.PlaneNormals = { Near.Normal, Top.Normal, Bottom.Normal, Left.Normal, Right.Normal },
			.Vertices = { v0, v1, v2, v3, v4, v5, v6, v7 },
			.Edges = { nearBottomTop, nearLeftRight, nearLeftFar, farTopBottom, farRightLeft, farRightNear }
		};
	}
}
