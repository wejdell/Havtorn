// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsMaterial.h"

namespace Havtorn
{
	struct SMaterialComponent : public SComponent
	{
		SMaterialComponent(Ref<SEntity> entity, EComponentType type)
			: SComponent(std::move(entity), type)
		{}

		std::vector<SEngineGraphicsMaterial> Materials;
	};
}