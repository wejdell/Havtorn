// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Component.h"

#include "Debug/DebugShape.h"

namespace Havtorn
{
	struct SDebugShapeComponent : public SComponent
	{
		SDebugShapeComponent(Ref<SEntity> entity, EComponentType type)
			: SComponent(std::move(entity), type) {}

		//std::vector<const Debug::SDebugShape*> shapes;
		U16 ShapeIndex;
	};
}