// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SEnvironmentLightComponent : public SComponent
	{
		SEnvironmentLightComponent()
			: SComponent(EComponentType::EnvironmentLightComponent)
		{}

		U16 AmbientCubemapReference = 0;
	};
}
