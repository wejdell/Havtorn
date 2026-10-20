// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GeometryRenderer.h"

#include "Graphics/RenderManager.h"
#include "Graphics/RenderStateManager.h" 
#include "Graphics/RenderResourceRegistry.h"

namespace Havtorn
{
	bool CGeometryRenderer::Init(CRHI* /*rhi*/, CRenderManager* manager)
	{
		RenderStateManager = &manager->GetRenderStateManager();

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
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::Decal)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::GBufferAlphaBlend)], // TODO.NW: Make sure this is correct
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)], // TODO.NW: See if this should be front face culling?
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::OnlyRead)],
			.RootSignature = nullptr
		};
		DecalGBufferPassPSOIndex = RenderStateManager->AddPipelineStateObject(deferredDecal);

		return true;
	}

	void CGeometryRenderer::SetupRender(const SGeometryConstantRenderData& constantData, const SGeometryTargetRenderData& targetData)
	{
		RenderStateManager->VSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);
		RenderStateManager->GSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer); // TODO.NW: Remove this when we get rid of geometry shaders for sprites/widgets

		RenderStateManager->OMSetRenderTargets(targetData.GetRenderTargets(), targetData.DepthTexture);
	}

	void CGeometryRenderer::SetupRender(const SGeometryConstantRenderData& constantData, const SGeometryTargetRenderData& targetData, const SEditorRenderData& editorRenderData)
	{
		RenderStateManager->VSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer);
		RenderStateManager->GSSetConstantBuffer(0, *constantData.ViewDataBuffer->DataBuffer); // TODO.NW: Remove this when we get rid of geometry shaders for sprites/widgets

		std::vector<SRenderResource*> renderTargets = targetData.GetRenderTargets();
		renderTargets.append_range(editorRenderData.GetRenderTargets());
		RenderStateManager->OMSetRenderTargets(renderTargets, targetData.DepthTexture);
	}

	void CGeometryRenderer::RenderStaticMesh(const SStaticMeshRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(StaticMeshGBufferPassPSOIndex);

		RenderStateManager->IASetVertexBuffers(0, std::vector<SRenderResource*>{ passData.VertexBuffer, passData.TransformBuffer });
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		RenderStateManager->PSSetResources(5, passData.MaterialTextures);
		RenderStateManager->PSSetConstantBuffer(8, *passData.MaterialBuffer->DataBuffer);

		RenderStateManager->DrawIndexedInstanced(passData.IndexCount, passData.InstanceCount, 0, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CGeometryRenderer::RenderStaticMesh(const SStaticMeshRenderData& passData, const SEditorRenderData& editorPassData)
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

		RenderStateManager->PSSetResources(5, passData.MaterialTextures);
		RenderStateManager->PSSetConstantBuffer(8, *passData.MaterialBuffer->DataBuffer);

		RenderStateManager->DrawIndexedInstanced(passData.IndexCount, passData.InstanceCount, 0, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CGeometryRenderer::RenderSkeletalMesh(const SSkeletalMeshRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(SkeletalMeshGBufferPassPSOIndex);

		RenderStateManager->IASetVertexBuffers(0, std::vector<SRenderResource*>{ passData.VertexBuffer, passData.TransformBuffer });
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		RenderStateManager->VSSetConstantBuffer(6, *passData.BoneBuffer->DataBuffer);
		RenderStateManager->PSSetResources(5, passData.MaterialTextures);
		RenderStateManager->PSSetConstantBuffer(8, *passData.MaterialBuffer->DataBuffer);

		RenderStateManager->DrawIndexedInstanced(passData.IndexCount, passData.InstanceCount, 0, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CGeometryRenderer::RenderSkeletalMesh(const SSkeletalMeshRenderData& passData, const SEditorRenderData& editorPassData)
	{
		RenderStateManager->TrySetPipelineStateObject(SkeletalMeshEditorGBufferPassPSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.VertexBuffer, editorPassData.EntityBuffer, passData.TransformBuffer });
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		RenderStateManager->VSSetConstantBuffer(6, *passData.BoneBuffer->DataBuffer);
		RenderStateManager->PSSetResources(5, passData.MaterialTextures);
		RenderStateManager->PSSetConstantBuffer(8, *passData.MaterialBuffer->DataBuffer);

		RenderStateManager->DrawIndexedInstanced(passData.IndexCount, passData.InstanceCount, 0, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CGeometryRenderer::RenderDecal(const SDecalRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(DecalGBufferPassPSOIndex);

		RenderStateManager->IASetVertexBuffers(0, std::vector<CDataBuffer*>{ &RenderStateManager->VertexBuffers[STATIC_U8(EVertexBufferPrimitives::DecalProjector)], passData.TransformBuffer->DataBuffer });
		RenderStateManager->IASetIndexBuffer(RenderStateManager->IndexBuffers[STATIC_U8(EDefaultIndexBuffers::DecalProjector)]);
		
		RenderStateManager->PSSetConstantBuffer(8, *passData.MaterialBuffer->DataBuffer);
		RenderStateManager->PSSetResources(5, passData.MaterialTextures);
		RenderStateManager->PSSetResources(21, { passData.DepthCopyTexture });

		RenderStateManager->DrawIndexedInstanced(36, passData.InstanceCount, 0, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++; // TODO.NW: Move this incrementation into a singular function so we don't have to add it everywhere
	}
}
