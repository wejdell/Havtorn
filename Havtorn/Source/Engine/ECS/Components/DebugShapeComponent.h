// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Component.h"

namespace Havtorn
{
	struct SDebugShapeComponent : public SComponent
	{
		SDebugShapeComponent(Ref<SEntity> entity, EComponentType type)
			: SComponent(std::move(entity), type) {}

		// TODO.AG: Might want to replace SVector4 with a unique color struct, to wrap 255.0f RGBA values
		SVector4 Color = { 0.0f, 0.0f, 0.0f, 0.0f };

		F32 LifeTime = 0.0f;
		F32 Thickness = 1.0f;
		U8 VertexBufferIndex = 0;
		U8 IndexCount = 0;
		U8 IndexBufferIndex = 0;
		bool IgnoreDepth = true;

		const bool operator<(const SDebugShapeComponent& rhs) const { return LifeTime < rhs.LifeTime; }
		const bool operator>(const SDebugShapeComponent& rhs) const { return LifeTime > rhs.LifeTime; }
	};
}
