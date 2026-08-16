// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "RHI/RHI.h"
#include "RHI/RHIEnums.h"

#include <Core.h>
#include <CoreTypes.h>

#include <d3d11.h>

namespace Havtorn
{
	class CShader;
	class CVertexInputLayout;
	class CBlendState;
	class CRasterizerState;
	class CDepthStencilState;
	class CRootSignature;

	struct SPSODescription
	{
		// TODO.NW: Deal with shaders becoming invalidated by recompilation
		CShader* VertexShader = nullptr;
		CShader* PixelShader = nullptr;
		CShader* GeometryShader = nullptr;
		CShader* ComputeShader = nullptr;
		CVertexInputLayout* InputLayout = nullptr;
		ETopologies Topology = ETopologies::TriangleList;
		CBlendState* BlendState = nullptr;
		CRasterizerState* RasterizerState = nullptr;
		CDepthStencilState* DepthStencilState = nullptr;
		CRootSignature* RootSignature = nullptr;

		U64 Hash() const;
	};

	class CPipelineStateObject
	{
	public:
		PLATFORM_API CPipelineStateObject(CRHI* rhi, const SPSODescription& description);

		// Returns internal hash if successfully set, 0 if not
		PLATFORM_API U64 TrySetPipelineState(const U64 currentHash);

	private:
		CRHI* RHI = nullptr;

		SPSODescription Description;
		U64 Hash = 0;

		//	- Content -
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
		//	Root Signature (specifies what shaders expect from CPU; buffers, textures, samplers)
	};
}
