// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SEnvironmentLightComponent : public SComponent
	{
		SEnvironmentLightComponent() = default;
		SEnvironmentLightComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		U16 AmbientCubemapReference = 0;
		U64 AssetRegistryKey = 0;
	};
}
