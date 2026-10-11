// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "SharedRendererData.h"

namespace Havtorn
{
	class CRenderStateManager;
	struct SRenderResource;

	struct SDebugShapeObjectBufferData
	{
		SMatrix ToWorldFromObject;
		SVector4 Color;
		F32 HalfThickness = 0.5f;
		F32 Padding[3] = {};
	} DebugShapeObjectBufferData;
	HV_ASSERT_BUFFER(SDebugShapeObjectBufferData)

	struct SDebugRenderData
	{
		// TODO.NW: See if this can feasibly be made instanced, might just need transform and color buffer?
		SRenderResource* ObjectBuffer = nullptr;
		SRenderResource* VertexBuffer = nullptr;
		SRenderResource* IndexBuffer = nullptr;
		U32 IndexCount = 0;
	};

	class CDebugRenderer 
	{
	public:
		CDebugRenderer(CRenderStateManager* stateManager);
		~CDebugRenderer() = default;

		void SetupDepthRender(const SGeometryConstantRenderData& viewData, const SMainTargetRenderData& targetData);
		void SetupNoDepthRender(const SGeometryConstantRenderData& viewData, const SMainTargetRenderData& targetData);

		void RenderDebugShape3D(const SDebugRenderData& passData);
		void RenderDebugShape2D(const SDebugRenderData& passData);
	
	private:
		U16 DebugPass3DPSOIndex = 0;
		U16 DebugPass2DPSOIndex = 0;

		CRenderStateManager* RenderStateManager = nullptr;
	};
}
