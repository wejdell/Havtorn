// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Plane.h"
#include "Sphere.h"
#include "AABB3D.h"
#include "Frustum.h"
#include "Ray.h"
#include <math.h>

namespace Havtorn 
{
	// If the ray is parallel to the plane, aOutIntersectionPoint remains unchanged. If
	// the ray is in the plane, true is returned, if not, false is returned. If the ray
	// isn't parallel to the plane, the intersection point is stored in
	// aOutIntersectionPoint and true returned.
	bool IntersectionPlaneLine(const SPlane &plane, const SRay &ray, SVector &out_intersection_point) 
	{
		F32 d = plane.GetPoint().Dot(plane.GetNormal());
		F32 dn = ray.Direction.Dot(plane.GetNormal());
		if (dn == 0) 
		{
			return false;
		}

		F32 t = (d - (ray.Origin.Dot(plane.GetNormal())) / dn);

		if (t > 0) 
		{
			out_intersection_point = ray.Origin + t * ray.Direction;
		}

		return t > 0;
	}

	// If the ray intersects the AABB, true is returned, if not, false is returned.
	// A ray in one of the AABB's sides is counted as intersecting it.
	bool IntersectionAABBLine(const AABB3D &AABB, const SRay &ray) 
	{
		SPlane plane_x_max = SPlane(AABB.Max, SVector(1, 0, 0));
		SPlane plane_y_max = SPlane(AABB.Max, SVector(0, 1, 0));
		SPlane plane_z_max = SPlane(AABB.Max, SVector(0, 0, 1));
		SPlane plane_x_min = SPlane(AABB.Min, SVector(1, 0, 0));
		SPlane plane_y_min = SPlane(AABB.Min, SVector(0, 1, 0));
		SPlane plane_z_min = SPlane(AABB.Min, SVector(0, 0, 1));

		SVector intersection_plane_x_max; 
		SVector intersection_plane_y_max;
		SVector intersection_plane_z_max;
		SVector intersection_plane_x_min;
		SVector intersection_plane_y_min;
		SVector intersection_plane_z_min;

		if (IntersectionPlaneLine(plane_x_max, ray, intersection_plane_x_max)) 
		{
			if (AABB.IsInside(intersection_plane_x_max)) 
			{
				return true;
			}
		}
		if (IntersectionPlaneLine(plane_y_max, ray, intersection_plane_y_max)) 
		{
			if (AABB.IsInside(intersection_plane_y_max)) 
			{
				return true;
			}
		}
		if (IntersectionPlaneLine(plane_z_max, ray, intersection_plane_z_max)) 
		{
			if (AABB.IsInside(intersection_plane_z_max)) 
			{
				return true;
			}
		}
		if (IntersectionPlaneLine(plane_x_min, ray, intersection_plane_x_min)) 
		{
			if (AABB.IsInside(intersection_plane_x_min)) 
			{
				return true;
			}
		}
		if (IntersectionPlaneLine(plane_y_min, ray, intersection_plane_y_min)) 
		{
			if (AABB.IsInside(intersection_plane_y_min)) 
			{
				return true;
			}
		}
		if (IntersectionPlaneLine(plane_z_min, ray, intersection_plane_z_min)) 
		{
			if (AABB.IsInside(intersection_plane_z_min)) 
			{
				return true;
			}
		}
		return false;
	}
	
	// If the ray intersects the sphere, true is returned, if not, false is returned.
	// A ray intersecting the surface of the sphere is considered as intersecting it.
	bool IntersectionSphereLine(const SSphere &sphere, const SRay &ray) 
	{
		SVector e = sphere.Center - ray.Origin;
		F32 a = e.Dot(ray.Direction);
		F32 t = a - sqrt((sphere.Radius * sphere.Radius) - e.LengthSquared() + (a * a));
		return t > 0;
		{
		//SVector ray_to_sphere = sphere.Center - ray.Origin;
		//F32 projection = ray_to_sphere.Dot(ray.Direction);
		//
		////if (projection < 0) {
		////	return false;
		////}

		//F32 perpendicular_component_squared = ray_to_sphere.LengthSqr() - (projection * projection);
		////
		////if (perpendicular_component_squared > sphere.Radius *sphere.Radius) {
		////	return false;
		////}

		////if (perpendicular_component_squared < 0) {
		////	return false;
		////}
		////
		//F32 intersection_point_squared = sphere.Radius * sphere.Radius - perpendicular_component_squared;
		//F32 t0 = sqrt(projection) - sqrt(intersection_point_squared);
		//F32 t1 = sqrt(projection) + sqrt(intersection_point_squared);
		//
		//if (t0 > t1) { std::swap(t0, t1); }

		//return t0 > 0;
		}
	}
}