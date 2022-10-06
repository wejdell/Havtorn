// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Component.h"

namespace Havtorn
{
	struct SDebugShapeComponent : public SComponent
	{
		SDebugShapeComponent(Ref<SEntity> entity, EComponentType type)
			: SComponent(std::move(entity), type) {}

		SVector4 Color = { 0.0f, 0.0f, 0.0f, 0.0f };
		F32 LifeTime = 0.0f;
		U8 VertexBufferIndex = 0;
		U8 VertexCount = 0;

		const bool operator<(const SDebugShapeComponent& rhs) const { return LifeTime < rhs.LifeTime; }
		const bool operator>(const SDebugShapeComponent& rhs) const { return LifeTime > rhs.LifeTime; }
	};
}