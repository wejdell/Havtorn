// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GeometryRenderer.h"

#include "Graphics/RenderManager.h"
#include "Graphics/RenderStateManager.h" 
#include "Graphics/RenderResourceRegistry.h"

namespace Havtorn
{
	CGeometryRenderer::CGeometryRenderer(CRenderStateManager* stateManager)
	{
		RenderStateManager = stateManager;

		const SPSODescription staticMeshGame =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::StaticMeshInstanced)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::GBuffer)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::Disable)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		StaticMeshGBufferPassPSOIndex = RenderStateManager->AddPipelineStateObject(staticMeshGame);

		const SPSODescription staticMeshEditor =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::StaticMeshInstancedEditor)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::GBufferInstanceEditor)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Entity2Trans)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::Disable)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		StaticMeshEditorGBufferPassPSOIndex = RenderStateManager->AddPipelineStateObject(staticMeshEditor);

		const SPSODescription skeletalMeshGame =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::SkeletalMeshInstanced)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::GBuffer)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4Trans)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::Disable)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		SkeletalMeshGBufferPassPSOIndex = RenderStateManager->AddPipelineStateObject(skeletalMeshGame);

		const SPSODescription skeletalMeshEditor =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::SkeletalMeshInstancedEditor)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::GBufferInstanceEditor)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4Entity2Trans)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::Disable)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		SkeletalMeshEditorGBufferPassPSOIndex = RenderStateManager->AddPipelineStateObject(skeletalMeshEditor);

		const SPSODescription deferredDecal =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::Decal)],
			.PixelShader = nullptr, // Shaders for each set of albedo / material / normal is set in the render command for now
			.GeometryShader = RenderStateManager->GeometryShaders[STATIC_U8(EGeometryShaders::SpriteWorldSpace)],
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Pos3Nor3Tan3Bit3UV2)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::AlphaBlend)], // TODO.NW: See if this should be GBufferAlphaBlend?
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)], // TODO.NW: See if this should be front face culling?
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::OnlyRead)],
			.RootSignature = nullptr
		};
		DecalGBufferPassPSOIndex = RenderStateManager->AddPipelineStateObject(deferredDecal);

		const SPSODescription skybox =
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
		SkyboxPassPSOIndex = RenderStateManager->AddPipelineStateObject(skybox);
	}

	void CGeometryRenderer::RenderStaticMesh(const SStaticMeshRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(StaticMeshGBufferPassPSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.VertexBuffer, passData.TransformBuffer });
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		// TODO.NW: This assumes the frame buffer has been set earlier, with the active camera view
		// Should we let pass data get control over what frame buffer to use? Probably. Need to set frame buffer here in that case
		//RenderStateManager->VSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);

		// TODO.NW: Figure out if we can just assume a default sampler binding for every pass, not using that many yet
		//RenderStateManager->PSSetSampler(0, ESamplers::DefaultWrap);

		RenderStateManager->PSSetResources(5, passData.MaterialTextures);
		RenderStateManager->PSSetConstantBuffer(8, *passData.MaterialBuffer->DataBuffer);

		RenderStateManager->DrawIndexedInstanced(passData.IndexCount, passData.InstanceCount, 0, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CGeometryRenderer::RenderEditorStaticMesh(const SStaticMeshRenderData& passData, const SEditorGeometryRenderData& editorPassData)
	{
		RenderStateManager->TrySetPipelineStateObject(StaticMeshEditorGBufferPassPSOIndex);
		
		// TODO.NW: Figure out a solution for this, can this be done at the render graph level?
		//const bool renderingPreviewEntity = std::ranges::find(entities, WorldEditorRenderExemptEntity) != entities.end();
		//if (renderingPreviewEntity)
		//{
		//	GBuffer.ReleaseRenderTargets();
		//	GBuffer.SetAsActiveTarget(&IntermediateDepth, false);
		//	GBufferDataInstanced(command);
		//	GBuffer.SetAsActiveTarget(&IntermediateDepth, true);
		//	return;
		//}

		RenderStateManager->IASetVertexBuffers(0, { passData.VertexBuffer, passData.TransformBuffer, editorPassData.EntityBuffer });
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		// TODO.NW: This assumes the frame buffer has been set earlier, with the active camera view
		// Should we let pass data get control over what frame buffer to use? Probably. Need to set frame buffer here in that case
		//RenderStateManager->VSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);

		// TODO.NW: Figure out if we can just assume a default sampler binding for every pass, not using that many yet
		//RenderStateManager->PSSetSampler(0, ESamplers::DefaultWrap);

		RenderStateManager->PSSetResources(5, passData.MaterialTextures);
		RenderStateManager->PSSetConstantBuffer(8, *passData.MaterialBuffer->DataBuffer);

		RenderStateManager->DrawIndexedInstanced(passData.IndexCount, passData.InstanceCount, 0, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CGeometryRenderer::RenderSkeletalMesh(const SSkeletalMeshRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(SkeletalMeshGBufferPassPSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.VertexBuffer, passData.TransformBuffer });
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		// TODO.NW: This assumes the frame buffer has been set earlier, with the active camera view
		// Should we let pass data get control over what frame buffer to use? Probably. Need to set frame buffer here in that case
		//RenderStateManager->VSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);
		RenderStateManager->VSSetConstantBuffer(6, *passData.BoneBuffer->DataBuffer);

		// TODO.NW: Figure out if we can just assume a default sampler binding for every pass, not using that many yet
		//RenderStateManager->PSSetSampler(0, ESamplers::DefaultWrap);

		RenderStateManager->PSSetResources(5, passData.MaterialTextures);
		RenderStateManager->PSSetConstantBuffer(8, *passData.MaterialBuffer->DataBuffer);

		RenderStateManager->DrawIndexedInstanced(passData.IndexCount, passData.InstanceCount, 0, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CGeometryRenderer::RenderEditorSkeletalMesh(const SSkeletalMeshRenderData& passData, const SEditorGeometryRenderData& editorPassData)
	{
		RenderStateManager->TrySetPipelineStateObject(SkeletalMeshEditorGBufferPassPSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.VertexBuffer, editorPassData.EntityBuffer, passData.TransformBuffer });
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		// TODO.NW: This assumes the frame buffer has been set earlier, with the active camera view
		// Should we let pass data get control over what frame buffer to use? Probably. Need to set frame buffer here in that case
		//RenderStateManager->VSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);
		RenderStateManager->VSSetConstantBuffer(6, *passData.BoneBuffer->DataBuffer);

		// TODO.NW: Figure out if we can just assume a default sampler binding for every pass, not using that many yet
		//RenderStateManager->PSSetSampler(0, ESamplers::DefaultWrap);

		RenderStateManager->PSSetResources(5, passData.MaterialTextures);
		RenderStateManager->PSSetConstantBuffer(8, *passData.MaterialBuffer->DataBuffer);

		RenderStateManager->DrawIndexedInstanced(passData.IndexCount, passData.InstanceCount, 0, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CGeometryRenderer::RenderDecal(const SDecalRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(DecalGBufferPassPSOIndex);

		RenderStateManager->IASetVertexBuffer(0, RenderStateManager->VertexBuffers[STATIC_U8(EDefaultIndexBuffers::DecalProjector)]);
		RenderStateManager->IASetIndexBuffer(RenderStateManager->IndexBuffers[STATIC_U8(EDefaultIndexBuffers::DecalProjector)]);
	
		// TODO.NW: This assumes the frame buffer has been set earlier, with the active camera view
		// Should we let pass data get control over what frame buffer to use? Probably. Need to set frame buffer here in that case
		//RenderStateManager->VSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);
		//RenderStateManager->PSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);
		RenderStateManager->VSSetConstantBuffer(1, *passData.DecalBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(1, *passData.DecalBuffer->DataBuffer);

		// TODO.NW: Figure out if we can just assume a default sampler binding for every pass, not using that many yet
		//RenderStateManager->PSSetSampler(0, ESamplers::DefaultWrap);

		if (passData.OptionalAlbedoTexture != nullptr)
		{
			passData.OptionalAlbedoTexture->RenderTexture->SetAsPSResourceOnSlot(5);
			RenderStateManager->PSSetShader(EPixelShaders::DecalAlbedo);
			RenderStateManager->DrawIndexed(36, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		if (passData.OptionalMaterialTexture)
		{
			passData.OptionalAlbedoTexture->RenderTexture->SetAsPSResourceOnSlot(6);
			RenderStateManager->PSSetShader(EPixelShaders::DecalMaterial);
			RenderStateManager->DrawIndexed(36, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		if (passData.OptionalNormalTexture)
		{
			passData.OptionalAlbedoTexture->RenderTexture->SetAsPSResourceOnSlot(7);
			RenderStateManager->PSSetShader(EPixelShaders::DecalNormal);
			RenderStateManager->DrawIndexed(36, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CGeometryRenderer::RenderSkybox(const SSkyboxRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(SkyboxPassPSOIndex);

		RenderStateManager->IASetVertexBuffer(0, RenderStateManager->VertexBuffers[STATIC_U8(EVertexBufferPrimitives::SkyboxCube)]);
		RenderStateManager->IASetIndexBuffer(RenderStateManager->IndexBuffers[STATIC_U8(EDefaultIndexBuffers::SkyboxCube)]);

		RenderStateManager->PSSetResources(21, 1, nullptr);
		passData.CubemapTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		// TODO.NW: Might want to do this, but then the sync points between passes get a bit blurry. Probably would like to manage this outside of this function
		passData.LitSceneTextureWithDepth->RenderTexture->SetAsActiveTarget(passData.LitSceneTextureWithDepth->DepthTexture);

		RenderStateManager->DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}
}
