// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Component.h"
#include "Graphics/RenderManager.h"// Temp
#include "Core/ColorList.h"

namespace Havtorn
{
	struct SDebugShapeComponent : public SComponent
	{
		SDebugShapeComponent(Ref<SEntity> entity, EComponentType type)
			: SComponent(std::move(entity), type) {}

		SVector4 Color = Color::White;
		F32 LifeTime = 0.0f;
		U8 VertexBufferIndex = static_cast<U8>(EVertexBufferPrimitives::LineShape);// temp
		U8 VertexCount = 2;

		const bool operator<(const SDebugShapeComponent& rhs) const { return LifeTime < rhs.LifeTime; }
		const bool operator>(const SDebugShapeComponent& rhs) const { return LifeTime > rhs.LifeTime; }
	};
}