// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteRenderer.h"

#include "Graphics/RenderManager.h"
#include "Graphics/RenderStateManager.h" 
#include "Graphics/RenderResourceRegistry.h"

namespace Havtorn
{
	CSpriteRenderer::CSpriteRenderer(CRenderStateManager* stateManager)
	{
		RenderStateManager = stateManager;

		const SPSODescription screenSpaceSprite =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::SpriteInstanced)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::SpriteScreenSpace)],
			.GeometryShader = RenderStateManager->GeometryShaders[STATIC_U8(EGeometryShaders::SpriteScreenSpace)],
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::TransUVRectColor)],
			.Topology = ETopologies::PointList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::AlphaBlend)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		ScreenSpaceSpritePSOIndex = RenderStateManager->AddPipelineStateObject(screenSpaceSprite);

		const SPSODescription worldSpriteGame =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::SpriteInstanced)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::SpriteWorldSpace)],
			.GeometryShader = RenderStateManager->GeometryShaders[STATIC_U8(EGeometryShaders::SpriteWorldSpace)],
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::TransUVRectColor)],
			.Topology = ETopologies::PointList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::GBufferAlphaBlend)], // TODO.NW: Fix transparency
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		WorldSpaceSpritePSOIndex = RenderStateManager->AddPipelineStateObject(worldSpriteGame);

		SPSODescription worldSpriteEditor =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::SpriteInstancedEditor)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::SpriteWorldSpaceEditor)],
			.GeometryShader = RenderStateManager->GeometryShaders[STATIC_U8(EGeometryShaders::SpriteWorldSpaceEditor)],
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::TransUVRectColorEntity2)],
			.Topology = ETopologies::PointList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::GBufferAlphaBlend)], // TODO.NW: Fix transparency
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::Default)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		WorldSpaceEditorSpritePSOIndex = RenderStateManager->AddPipelineStateObject(worldSpriteEditor);

		worldSpriteEditor.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::Disable)];
		worldSpriteEditor.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::SpriteWorldSpaceEditorWidget)];
		WorldSpaceEditorWidgetPSOIndex = RenderStateManager->AddPipelineStateObject(worldSpriteEditor);
	}

	void CSpriteRenderer::RenderScreenSpaceSprite(const SSpriteRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(ScreenSpaceSpritePSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.TransformBuffer, passData.UVRectBuffer, passData.ColorBuffer });
		RenderStateManager->IASetIndexBuffer(CDataBuffer::Null);
		
		//RenderStateManager->PSSetSampler(0, ESamplers::DefaultWrap);

		passData.SpriteTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		RenderStateManager->DrawInstanced(1, passData.InstanceCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CSpriteRenderer::RenderWorldSpaceSprite(const SSpriteRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(WorldSpaceSpritePSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.TransformBuffer, passData.UVRectBuffer, passData.ColorBuffer });
		RenderStateManager->IASetIndexBuffer(CDataBuffer::Null);

		//RenderStateManager->PSSetSampler(0, ESamplers::DefaultWrap);

		passData.SpriteTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		RenderStateManager->DrawInstanced(1, passData.InstanceCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CSpriteRenderer::RenderEditorWorldSpaceSprite(const SSpriteRenderData& passData, const SEditorSpriteRenderData& editorPassData)
	{
		RenderStateManager->TrySetPipelineStateObject(WorldSpaceEditorSpritePSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.TransformBuffer, passData.UVRectBuffer, passData.ColorBuffer, editorPassData.EntityBuffer });
		RenderStateManager->IASetIndexBuffer(CDataBuffer::Null);

		//RenderStateManager->PSSetSampler(0, ESamplers::DefaultWrap);

		passData.SpriteTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		RenderStateManager->DrawInstanced(1, passData.InstanceCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CSpriteRenderer::RenderEditorWorldSpaceWidget(const SSpriteRenderData& passData, const SEditorSpriteRenderData& editorPassData)
	{
		RenderStateManager->TrySetPipelineStateObject(WorldSpaceEditorWidgetPSOIndex);

		// TODO.NW: Figure out what to do about this render target assignment
		//ID3D11RenderTargetView* renderTargets[3] = { TonemappedTexture.GetRenderTargetView(), GBuffer.GetEditorWorldPositionRenderTarget(), GBuffer.GetEditorDataRenderTarget() };
		//RenderStateManager->OMSetRenderTargets(3, renderTargets, EditorWidgetDepth.GetDepthStencilView());

		RenderStateManager->IASetVertexBuffers(0, { passData.TransformBuffer, passData.UVRectBuffer, passData.ColorBuffer, editorPassData.EntityBuffer });
		RenderStateManager->IASetIndexBuffer(CDataBuffer::Null);

		//RenderStateManager->PSSetSampler(0, ESamplers::DefaultWrap);

		passData.SpriteTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		RenderStateManager->DrawInstanced(1, passData.InstanceCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}
}
