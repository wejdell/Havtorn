// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	enum class EPhysics3DBodyType : U8
	{
		Static,
		Kinematic,
		Dynamic
	};

	enum class EPhysics3DShapeType : U8
	{
		Sphere,
		InfinitePlane, // Infinite plane
		Capsule,
		Box,
		Convex,
	};

	struct SPhysicsMaterial
	{
		F32 DynamicFriction = 0.5f;
		F32 StaticFriction = 0.5f;
		F32 Restitution = 0.0f;

		auto operator<=>(const SPhysicsMaterial& other) const = default;
	};

	struct SPhysics3DComponent : public SComponent
	{
		SPhysics3DComponent() = default;
		SPhysics3DComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		EPhysics3DBodyType BodyType = EPhysics3DBodyType::Static;
		// TODO.NR: Support multiple shapes on same body
		EPhysics3DShapeType ShapeType = EPhysics3DShapeType::Sphere;
		SVector ShapeLocalOffset = SVector::Zero;

		union
		{
			SVector ShapeLocalExtents = SVector::Zero;
			SVector2<F32> ShapeLocalRadiusAndHeight;
			F32 ShapeLocalRadius;
		};

		SVector Velocity = SVector::Zero;
		SPhysicsMaterial Material = {};
		F32 Density = 10.f;
		bool IsTrigger = false;
	};
}
