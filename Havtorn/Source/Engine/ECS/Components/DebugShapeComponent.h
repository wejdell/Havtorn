// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Component.h"

#include "Core/Color.h"

namespace Havtorn
{
	struct SDebugShapeComponent : public SComponent
	{
		SDebugShapeComponent()
			: SComponent(EComponentType::DebugShapeComponent) 
		{}

		SColor Color;
		F32 LifeTime = 0.0f;
		F32 Thickness = 1.0f;
		U16 IndexCount = 0;
		U8 VertexBufferIndex = 0;
		U8 IndexBufferIndex = 0;
		bool IgnoreDepth = true;

		const bool operator<(const SDebugShapeComponent& rhs) const { return LifeTime < rhs.LifeTime; }
		const bool operator>(const SDebugShapeComponent& rhs) const { return LifeTime > rhs.LifeTime; }
	};
}
