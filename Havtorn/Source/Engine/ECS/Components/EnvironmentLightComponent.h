// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SEnvironmentLightComponent : public SComponent
	{
		SEnvironmentLightComponent(Ref<SEntity> entity, EComponentType type)
			: SComponent(std::move(entity), type)
		{}

		U16 AmbientCubemapReference = 0;
	};
}
