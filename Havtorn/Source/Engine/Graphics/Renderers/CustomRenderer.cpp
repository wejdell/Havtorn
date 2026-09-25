// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CustomRenderer.h"

#include "Graphics/RenderManager.h"
#include "Graphics/RenderStateManager.h" 
#include "Graphics/RenderResourceRegistry.h"

namespace Havtorn
{
	CCustomRenderer::CCustomRenderer(CRenderStateManager* stateManager)
	{
		RenderStateManager = stateManager;

		//const SPSODescription customPSO =
		//{
		//	.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::StaticMeshInstanced)],
		//	.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::GBuffer)],
		//	.GeometryShader = nullptr,
		//	.ComputeShader = nullptr,
		//	.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)],
		//	.Topology = ETopologies::TriangleList,
		//	.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::Disable)],
		//	.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
		//	.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
		//	.RootSignature = nullptr
		//};
		//CustomPassPSOIndex = RenderStateManager->AddPipelineStateObject(customPSO);
	}

	void CCustomRenderer::RenderCustomPass(const SCustomPassData& /*passData*/)
	{
		//RenderStateManager->TrySetPipelineStateObject(CustomPassPSOIndex);
		//
		//passData.Resource->RenderTexture->SetAsPSResourceOnSlot(1);

		//RenderStateManager->DrawIndexed(3, 0, 0);
		//CRenderManager::NumberOfDrawCallsThisFrame++;
	}
}
