// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "RHI/RHI.h"

#include <Core.h>
#include <CoreTypes.h>

#include <d3d11.h>

namespace Havtorn
{
	class CShader;

	struct SPSODescription
	{
		CShader* VertexShader = nullptr;
		CShader* PixelShader = nullptr;
		CShader* GeometryShader = nullptr;
		//ETopologies Topology = ETopologies::TriangleList;
	};

	class CPipelineStateObject
	{
	public:
		PLATFORM_API CPipelineStateObject(SPSODescription& description, const CRHI* rhi);

	private:
		ID3D11DeviceContext* Context = nullptr;

		//	Content
		//	Shader Bytecode
		//	Vertex Format Input Layout
		//	Primitive Topology TYPE
		//	Blend State
		//	Rasterizer State
		//	Depth - Stencil State
		//	Num Render Targets, Render Target Formats
		//	Depth - Stencil Format
		//	Multisample Desc
		//	Stream Output Buffer Desc
		//	Root Signature (specifies what shaders expect from CPU)
	};
}
