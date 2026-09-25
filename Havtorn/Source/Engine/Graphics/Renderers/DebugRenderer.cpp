// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugRenderer.h"

#include "Graphics/RenderManager.h"
#include "Graphics/RenderStateManager.h" 
#include "Graphics/RenderResourceRegistry.h"

namespace Havtorn
{
	CDebugRenderer::CDebugRenderer(CRenderStateManager* stateManager)
	{
		RenderStateManager = stateManager;

		SPSODescription debugShapes =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::Line)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::Line)],
			.GeometryShader = RenderStateManager->GeometryShaders[STATIC_U8(EGeometryShaders::Line)],
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Position4)],
			.Topology = ETopologies::LineList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::AlphaBlend)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::Default)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::OnlyRead)],
			.RootSignature = nullptr
		};
		DebugPass3DPSOIndex = RenderStateManager->AddPipelineStateObject(debugShapes);

		debugShapes.GeometryShader = RenderStateManager->PixelShaders[STATIC_U8(EGeometryShaders::Line2D)];
		DebugPass2DPSOIndex = RenderStateManager->AddPipelineStateObject(debugShapes);
	}

	void CDebugRenderer::RenderDebugShape3D(const SDebugPassData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(DebugPass3DPSOIndex);

		RenderStateManager->IASetVertexBuffer(0, *passData.VertexBuffer->DataBuffer);
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		RenderStateManager->VSSetConstantBuffer(1, *passData.ObjectBuffer->DataBuffer);
		RenderStateManager->GSSetConstantBuffer(1, *passData.ObjectBuffer->DataBuffer);

		RenderStateManager->DrawIndexed(passData.IndexCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CDebugRenderer::RenderDebugShape2D(const SDebugPassData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(DebugPass2DPSOIndex);

		RenderStateManager->IASetVertexBuffer(0, *passData.VertexBuffer->DataBuffer);
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		RenderStateManager->VSSetConstantBuffer(1, *passData.ObjectBuffer->DataBuffer);
		RenderStateManager->GSSetConstantBuffer(1, *passData.ObjectBuffer->DataBuffer);

		RenderStateManager->DrawIndexed(passData.IndexCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}
}
