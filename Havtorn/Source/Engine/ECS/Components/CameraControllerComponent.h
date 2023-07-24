// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	enum class ECameraControllerType
	{
		FreeCam,
		FirstPerson,
		ThirdPerson,
		Cinematic
	};

	struct SCameraControllerComponent : public SComponent
	{
		SCameraControllerComponent()
			: SComponent(EComponentType::CameraControllerComponent)
		{}

		static constexpr F32 MaxPitchDegrees = 90.0f;

		F32 MaxMoveSpeed = 3.0f;
		F32 RotationSpeed = 2.0f;
		F32 AccelerationDuration = 0.2f;

		SVector AccelerationDirection = SVector::Zero;
		F32 CurrentPitch = 0.0f;
		F32 CurrentYaw = 0.0f;
		F32 CurrentAccelerationFactor = 0.0f;

		ECameraControllerType ControllerType = ECameraControllerType::FreeCam;
	};
}
