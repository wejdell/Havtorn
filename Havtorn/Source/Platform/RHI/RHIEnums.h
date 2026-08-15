// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include <CoreTypes.h>

namespace Havtorn
{
#ifdef HV_RENDER_BACKEND_DIRECTX11
	enum class ETopologies : U8
	{
		TriangleList = 4, //D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		LineList = 2, //D3D11_PRIMITIVE_TOPOLOGY_LINELIST
		PointList = 1 //D3D11_PRIMITIVE_TOPOLOGY_POINTLIST
	};
#endif

	enum class EShaderType : U8
	{
		Vertex,
		Compute,
		Geometry,
		Pixel
	};

	enum class ERenderComparisonFunction : U8
	{
		Never = 1,
		Less = 2,
		Equal = 3,
		LessOrEqual = 4,
		Greater = 5,
		NotEqual = 6,
		GreaterOrEqual = 7,
		Always = 8
	};
}
