// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	enum class ECameraProjectionType
	{
		Perspective,
		Orthographic,
	};

	struct SCameraComponent : public SComponent
	{
		SCameraComponent()
			: SComponent(EComponentType::CameraComponent)
		{}

		SMatrix ViewMatrix;
		SMatrix ProjectionMatrix;
		F32 NearClip = 0.1f;
		F32 FarClip = 1000.0f;
		
		// Perspective
		F32 FOV = 70.0f;
		F32 AspectRatio = (16.0f / 9.0f);

		// Orthographic
		F32 ViewWidth = 10.0f;
		F32 ViewHeight = 10.0f;

		ECameraProjectionType ProjectionType = ECameraProjectionType::Perspective;
	};
}
