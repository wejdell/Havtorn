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
}
