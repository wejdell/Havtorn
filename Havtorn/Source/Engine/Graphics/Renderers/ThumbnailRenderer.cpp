// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ThumbnailRenderer.h"

#include "Graphics/RenderManager.h"
#include "Graphics/RenderStateManager.h" 
#include "Graphics/RenderResourceRegistry.h"

namespace Havtorn
{
	CThumbnailRenderer::CThumbnailRenderer(CRenderStateManager* stateManager)
	{
		RenderStateManager = stateManager;

		const SPSODescription staticMeshThumbnail =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::EditorPreviewStaticMesh)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::EditorPreview)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Pos3Nor3Tan3Bit3UV2)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::Disable)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		StaticMeshThumbnailPSOIndex = RenderStateManager->AddPipelineStateObject(staticMeshThumbnail);

		const SPSODescription skeletalMeshThumbnail =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::EditorPreviewSkeletalMesh)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::EditorPreview)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4AnimDataTrans)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::Disable)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		SkeletalMeshThumbnailPSOIndex = RenderStateManager->AddPipelineStateObject(skeletalMeshThumbnail);

		const SPSODescription textureCubeThumbnail =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::Skybox)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::Skybox)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Position4)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::Disable)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::FrontfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::DepthFirst)],
			.RootSignature = nullptr
		};
		CubemapTextureThumbnailPSOIndex = RenderStateManager->AddPipelineStateObject(textureCubeThumbnail);
	}

	void CThumbnailRenderer::RenderStaticMeshThumbnail(const SStaticMeshThumbnailPassData& passData)
	{
		// TODO.NW: Figure out why the depth doesn't work
		
		// TODO.NW: Figure out if we need to set this
		//renderTarget.GetViewport().SetViewport();

		RenderStateManager->TrySetPipelineStateObject(StaticMeshThumbnailPSOIndex);

		RenderStateManager->IASetVertexBuffer(0, *passData.VertexBuffer->DataBuffer);
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		RenderStateManager->VSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);
		RenderStateManager->VSSetConstantBuffer(1, *passData.ObjectBuffer->DataBuffer);

		RenderStateManager->PSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);
	
		RenderStateManager->DrawIndexed(passData.IndexCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CThumbnailRenderer::RenderSkeletalMeshThumbnail(const SSkeletalMeshThumbnailPassData& passData)
	{
		// TODO.NW: Figure out why the depth doesn't work

		// TODO.NW: Figure out if we need to set this
		//renderTarget.GetViewport().SetViewport();

		RenderStateManager->TrySetPipelineStateObject(SkeletalMeshThumbnailPSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.VertexBuffer, passData.AnimDataBuffer, passData.TransformBuffer });
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		RenderStateManager->VSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);
		RenderStateManager->VSSetConstantBuffer(1, *passData.ObjectBuffer->DataBuffer);
		RenderStateManager->VSSetConstantBuffer(2, *passData.BoneBuffer->DataBuffer);

		RenderStateManager->PSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);

		RenderStateManager->DrawIndexed(passData.IndexCount, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CThumbnailRenderer::RenderCubemapTextureThumbnail(const SCubemapTexturePassData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(CubemapTextureThumbnailPSOIndex);

		RenderStateManager->IASetVertexBuffer(0, RenderStateManager->VertexBuffers[STATIC_U8(EVertexBufferPrimitives::SkyboxCube)]);
		RenderStateManager->IASetIndexBuffer(RenderStateManager->IndexBuffers[STATIC_U8(EDefaultIndexBuffers::SkyboxCube)]);

		passData.CubemapTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		RenderStateManager->DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}
}
