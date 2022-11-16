// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "GeometryPrimitivesUtility.h"
#include "Graphics/GeometryPrimitives.h"

namespace Havtorn
{
	namespace UGraphicsUtils
	{
		const size_t GetVertexCount(const EVertexBufferPrimitives primitive)
		{
			switch (primitive)
			{
				case EVertexBufferPrimitives::DecalProjector:
					return GeometryPrimitives::DecalProjector.size();

				case EVertexBufferPrimitives::PointLightCube:
					return GeometryPrimitives::PointLightCube.size();

				case EVertexBufferPrimitives::LineShape:
					return GeometryPrimitives::LineShape.size();

				case EVertexBufferPrimitives::FlatArrow:
					return GeometryPrimitives::FlatArrow.size();

				case EVertexBufferPrimitives::DebugCube:
					return GeometryPrimitives::DebugCube.size();

				case EVertexBufferPrimitives::Camera:
					return GeometryPrimitives::Camera.size();

				case EVertexBufferPrimitives::CircleXY8:
					return GeometryPrimitives::CircleXY8.size();

				case EVertexBufferPrimitives::CircleXY16:
					return GeometryPrimitives::CircleXY16.size();

				case EVertexBufferPrimitives::CircleXY32:
					return GeometryPrimitives::CircleXY32.size();

				default:
					return 0;
			}
		}

		const size_t GetIndexCount(const EVertexBufferPrimitives primitive)
		{
			switch (primitive)
			{
				case EVertexBufferPrimitives::DecalProjector:
					return GeometryPrimitives::DecalProjectorIndices.size();

				case EVertexBufferPrimitives::PointLightCube:
					return GeometryPrimitives::PointLightCubeIndices.size();

				case EVertexBufferPrimitives::LineShape:
					return GeometryPrimitives::LineShapeIndices.size();

				case EVertexBufferPrimitives::FlatArrow:
					return GeometryPrimitives::FlatArrowIndices.size();

				case EVertexBufferPrimitives::DebugCube:
					return GeometryPrimitives::DebugCubeIndices.size();

				case EVertexBufferPrimitives::Camera:
					return GeometryPrimitives::CameraIndices.size();

				case EVertexBufferPrimitives::CircleXY8:
					return GeometryPrimitives::CircleXY8Indices.size();

				case EVertexBufferPrimitives::CircleXY16:
					return GeometryPrimitives::CircleXY16Indices.size();

				case EVertexBufferPrimitives::CircleXY32:
					return GeometryPrimitives::CircleXY32Indices.size();

				default:
					return 0;
			}
		}
	}
}
