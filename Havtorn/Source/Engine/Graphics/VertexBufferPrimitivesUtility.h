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

				default:
					return 0;
				}
			}

			template<class T>
			T GetVertexBufferIndex(const EVertexBufferPrimitives primitive)
			{
				return static_cast<T>(primitive);
			}
		}
	}
}
