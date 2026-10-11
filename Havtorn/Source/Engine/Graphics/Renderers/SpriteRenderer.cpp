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

		// TODO.NW: Remove geometry shaders because they are not performant! Set up a super simple default plane vertex buffer instead

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

	void CSpriteRenderer::SetupWorldSpaceRender(const SGeometryConstantRenderData& constantData, const SGeometryTargetRenderData& targetData)
	{
		RenderStateManager->VSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);
		RenderStateManager->GSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);

		RenderStateManager->OMSetRenderTargets(targetData.GetRenderTargets(), targetData.DepthTexture);
	}

	void CSpriteRenderer::SetupWorldSpaceRender(const SGeometryConstantRenderData& constantData, const SGeometryTargetRenderData& targetData, const SEditorRenderData& editorPassData)
	{
		RenderStateManager->VSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);
		RenderStateManager->GSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);

		std::vector<SRenderResource*> renderTargets = targetData.GetRenderTargets();
		renderTargets.append_range(editorPassData.GetRenderTargets());
		RenderStateManager->OMSetRenderTargets(renderTargets, targetData.DepthTexture);
	}

	void CSpriteRenderer::SetupWidgetRender(const SGeometryConstantRenderData& constantData, const SMainTargetRenderData& targetData, const SEditorRenderData& editorPassData)
	{
		RenderStateManager->VSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);
		RenderStateManager->GSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);

		std::vector<SRenderResource*> renderTargets = { targetData.MainRenderTarget, editorPassData.WorldPositionRenderTarget, editorPassData.EditorDataRenderTarget };
		RenderStateManager->OMSetRenderTargets(renderTargets, targetData.DepthTexture);
	}

	void CSpriteRenderer::SetupScreenSpaceRender(const SMainTargetRenderData& targetData)
	{
		RenderStateManager->OMSetRenderTargets({ targetData.MainRenderTarget }, targetData.DepthTexture);
	}

	void CSpriteRenderer::RenderScreenSpaceSprite(const SSpriteRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(ScreenSpaceSpritePSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.TransformBuffer, passData.UVRectBuffer, passData.ColorBuffer });
		RenderStateManager->IASetIndexBuffer(CDataBuffer::Null);
		
		passData.SpriteTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		RenderStateManager->DrawInstanced(1, passData.InstanceCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CSpriteRenderer::RenderWorldSpaceSprite(const SSpriteRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(WorldSpaceSpritePSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.TransformBuffer, passData.UVRectBuffer, passData.ColorBuffer });
		RenderStateManager->IASetIndexBuffer(CDataBuffer::Null);

		passData.SpriteTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		RenderStateManager->DrawInstanced(1, passData.InstanceCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CSpriteRenderer::RenderEditorWorldSpaceSprite(const SSpriteRenderData& passData, const SEditorRenderData& editorPassData)
	{
		RenderStateManager->TrySetPipelineStateObject(WorldSpaceEditorSpritePSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.TransformBuffer, passData.UVRectBuffer, passData.ColorBuffer, editorPassData.EntityBuffer });
		RenderStateManager->IASetIndexBuffer(CDataBuffer::Null);

		passData.SpriteTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		RenderStateManager->DrawInstanced(1, passData.InstanceCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CSpriteRenderer::RenderEditorWorldSpaceWidget(const SSpriteRenderData& passData, const SEditorRenderData& editorPassData)
	{
		RenderStateManager->TrySetPipelineStateObject(WorldSpaceEditorWidgetPSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.TransformBuffer, passData.UVRectBuffer, passData.ColorBuffer, editorPassData.EntityBuffer });
		RenderStateManager->IASetIndexBuffer(CDataBuffer::Null);

		passData.SpriteTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		RenderStateManager->DrawInstanced(1, passData.InstanceCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}
}
