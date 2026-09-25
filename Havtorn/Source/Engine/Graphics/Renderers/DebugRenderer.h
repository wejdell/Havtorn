// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class CRenderStateManager;
	struct SRenderResource;

	struct SDebugPassData
	{
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

		// TODO.NW: Deal with depth/no depth
		void RenderDebugShape3D(const SDebugPassData& passData);
		void RenderDebugShape2D(const SDebugPassData& passData);
	
	private:
		U16 DebugPass3DPSOIndex = 0;
		U16 DebugPass2DPSOIndex = 0;

		CRenderStateManager* RenderStateManager = nullptr;
	};
}
