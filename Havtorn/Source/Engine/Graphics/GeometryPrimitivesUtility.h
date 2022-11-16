// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Graphics/GraphicsEnums.h"
#include "Graphics/GeometryPrimitives.h"

namespace Havtorn
{
	namespace UGraphicsUtils
	{
		const static std::map<EVertexBufferPrimitives, const std::vector<SPositionVertex>> PrimitivesVertices = {
			{ EVertexBufferPrimitives::PointLightCube, GeometryPrimitives::PointLightCube},
			{ EVertexBufferPrimitives::LineShape, GeometryPrimitives::LineShape},
			{ EVertexBufferPrimitives::FlatArrow, GeometryPrimitives::FlatArrow},
			{ EVertexBufferPrimitives::DebugCube, GeometryPrimitives::DebugCube},
			{ EVertexBufferPrimitives::Camera, GeometryPrimitives::Camera},
			{ EVertexBufferPrimitives::CircleXY8, GeometryPrimitives::CircleXY8},
			{ EVertexBufferPrimitives::CircleXY16, GeometryPrimitives::CircleXY16},
			{ EVertexBufferPrimitives::CircleXY32, GeometryPrimitives::CircleXY32},
		};

		const static std::map<EVertexBufferPrimitives, const std::vector<U32>> PrimitivesIndices = {
			{ EVertexBufferPrimitives::DecalProjector, GeometryPrimitives::DecalProjectorIndices},
			{ EVertexBufferPrimitives::PointLightCube, GeometryPrimitives::PointLightCubeIndices},
			{ EVertexBufferPrimitives::LineShape, GeometryPrimitives::LineShapeIndices},
			{ EVertexBufferPrimitives::FlatArrow, GeometryPrimitives::FlatArrowIndices},
			{ EVertexBufferPrimitives::DebugCube, GeometryPrimitives::DebugCubeIndices},
			{ EVertexBufferPrimitives::Camera, GeometryPrimitives::CameraIndices},
			{ EVertexBufferPrimitives::CircleXY8, GeometryPrimitives::CircleXY8Indices},
			{ EVertexBufferPrimitives::CircleXY16, GeometryPrimitives::CircleXY16Indices},
			{ EVertexBufferPrimitives::CircleXY32, GeometryPrimitives::CircleXY32Indices},
		};

		template<class T>
		HAVTORN_API const T GetVertexCount(const EVertexBufferPrimitives primitive)
		{
			return static_cast<T>(PrimitivesVertices.at(primitive).size());
		}

		template<class T>
		HAVTORN_API const T GetIndexCount(const EVertexBufferPrimitives primitive)
		{
			return static_cast<T>(PrimitivesIndices.at(primitive).size());
		}

	}
}
