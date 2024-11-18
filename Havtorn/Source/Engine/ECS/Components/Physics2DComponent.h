// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "Core/BitSet.h"
#include "ECS/Component.h"

namespace Havtorn
{
	enum class EPhysics2DBodyType
	{
		Static, // No mass, no velocity. Can be moved manually
		Kinematic, // No mass. Velocity set by user. Moved by simulation
		Dynamic // Has mass. Velocity determined by forces. Moved by simulation
	};

	enum class EPhysics2DShapeType
	{
		Circle, // A circle with an offset
		Capsule, // A capsule is an extruded circle
		Segment, // A line segment
		Polygon, // A convex polygon
		ChainSegment // A line segment owned by a chain shape
	};

	struct SPhysics2DComponent : public SComponent
	{
		SPhysics2DComponent() = default;
		SPhysics2DComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		EPhysics2DBodyType BodyType = EPhysics2DBodyType::Static;
		// TODO.NR: Support multiple shapes on same body
		EPhysics2DShapeType ShapeType = EPhysics2DShapeType::Circle;
		SVector2<F32> ShapeLocalOffset = SVector2<F32>::Zero;
		// TODO.NR: Make union to name things clearer for every shape
		SVector2<F32> ShapeLocalExtents = SVector2<F32>::Zero;
		SVector2<F32> Velocity = SVector2<F32>::Zero;
		bool ConstrainRotation = false;
	};
}
