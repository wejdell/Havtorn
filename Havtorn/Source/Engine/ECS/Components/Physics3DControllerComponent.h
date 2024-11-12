// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	enum class EPhysics3DControllerType : U8
	{
		Box,
		Capsule
	};

	struct SPhysics3DControllerComponent : public SComponent
	{
		SPhysics3DControllerComponent() = default;
		SPhysics3DControllerComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		EPhysics3DControllerType ControllerType = EPhysics3DControllerType::Capsule;
		SVector ShapeLocalExtents = SVector::Zero;
		SVector Velocity = SVector::Zero;
	};
}
