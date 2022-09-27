// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Component.h"
#include "Graphics/RenderManager.h"// Temp
#include "Debug/DebugShape.h"// Temp. TODO.AG: SVector4 as colors

namespace Havtorn
{
	struct SDebugShapeComponent : public SComponent
	{
		SDebugShapeComponent(Ref<SEntity> entity, EComponentType type)
			: SComponent(std::move(entity), type) {}

		U16 ShapeIndex = 0;//TODO.AG: Remove this
		U8 VertexBufferIndex = static_cast<U8>(EVertexBufferPrimitives::LineShape);// temp
		SVector4 Color = Debug::Color::White.AsVector4();
		F32 LifeTime = 0.0f;

		const bool operator<(const SDebugShapeComponent& rhs) const { return LifeTime < rhs.LifeTime; }
		const bool operator>(const SDebugShapeComponent& rhs) const { return LifeTime > rhs.LifeTime; }
	};
}