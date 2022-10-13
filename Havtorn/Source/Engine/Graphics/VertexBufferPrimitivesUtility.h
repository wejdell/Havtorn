// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Graphics/RenderManager.h"
#include "Graphics/GeometryPrimitives.h"

namespace Havtorn
{
	namespace Utility
	{
		namespace VertexBufferPrimitives
		{
			template<class T>
			T GetVertexCount(const EVertexBufferPrimitives primitive)
			{
				switch (primitive)
				{
				case EVertexBufferPrimitives::DecalProjector:
					return static_cast<T>(GeometryPrimitives::DecalProjector.size());

				case EVertexBufferPrimitives::PointLightCube:
					return static_cast<T>(GeometryPrimitives::PointLightCube.size());

				case EVertexBufferPrimitives::LineShape:
					return static_cast<T>(GeometryPrimitives::Line.size());

				case EVertexBufferPrimitives::FlatArrow:
					return static_cast<T>(GeometryPrimitives::FlatArrow.size());

				case EVertexBufferPrimitives::DebugCube:
					return static_cast<T>(GeometryPrimitives::DebugCube.size());

				default:
					return 0;
				}
			}

			template<class T>
			T GetVertexBufferIndex(const EVertexBufferPrimitives primitive)
			{
				return static_cast<T>(primitive);
			}

			template<class T>
			T GetIndexCount(const EVertexBufferPrimitives primitive)
			{
				switch (primitive)
				{
				case EVertexBufferPrimitives::DecalProjector:
					return static_cast<T>(GeometryPrimitives::DecalProjectorIndices.size());

				case EVertexBufferPrimitives::PointLightCube:
					return static_cast<T>(GeometryPrimitives::PointLightCubeIndices.size());

				case EVertexBufferPrimitives::LineShape:
					return static_cast<T>(GeometryPrimitives::LineIndices.size());

				case EVertexBufferPrimitives::FlatArrow:
					return static_cast<T>(GeometryPrimitives::FlatArrowIndices.size());

				case EVertexBufferPrimitives::DebugCube:
					return static_cast<T>(GeometryPrimitives::DebugCubeIndices.size());

				default:
					return 0;
				}
			}

			template<class T>
			T GetIndexBufferIndex(const EDefaultIndexBuffers type)
			{
				return static_cast<T>(type);
			}
		}
	}
}
