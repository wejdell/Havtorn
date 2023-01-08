// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SVolumetricLightComponent: public SComponent
	{
		SVolumetricLightComponent()
			: SComponent(EComponentType::VolumetricLightComponent)
		{}

		bool IsActive = false;
		F32 NumberOfSamples = 16.0f;
		F32 LightPower = 500000.0f;
		F32 ScatteringProbability = 0.0001f;
		F32 HenyeyGreensteinGValue = 0.0f;
	};
}