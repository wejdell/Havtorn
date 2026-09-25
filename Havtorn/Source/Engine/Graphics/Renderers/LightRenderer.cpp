// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "LightRenderer.h"

#include "Graphics/RenderManager.h"
#include "Graphics/RenderStateManager.h" 
#include "Graphics/RenderResourceRegistry.h"

namespace Havtorn
{
	CLightRenderer::CLightRenderer(CRenderStateManager* stateManager)
	{
		RenderStateManager = stateManager;

		const SPSODescription staticMeshShadowPass =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::StaticMeshInstanced)],
			.PixelShader = nullptr,
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::Disable)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		StaticMeshShadowPassPSOIndex = RenderStateManager->AddPipelineStateObject(staticMeshShadowPass);

		const SPSODescription lightingDirectional =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::Fullscreen)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::DeferredDirectional)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = nullptr,
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::AdditiveBlend)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::BackfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		DirectionalLightPSOIndex = RenderStateManager->AddPipelineStateObject(lightingDirectional);

		const SPSODescription lightingPoint =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::PointAndSpotLight)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::DeferredPoint)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Position4)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::AdditiveBlend)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::FrontfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		PointLightPSOIndex = RenderStateManager->AddPipelineStateObject(lightingPoint);

		const SPSODescription lightingSpot =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::PointAndSpotLight)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::DeferredSpot)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Position4)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::AdditiveBlend)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::FrontfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		SpotlightPSOIndex = RenderStateManager->AddPipelineStateObject(lightingSpot);

		const SPSODescription volumetricLightDirectional =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::Fullscreen)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::VolumetricDirectional)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Null)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::AdditiveBlend)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::Default)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		}; 
		VolumetricDirectionalLightPSOIndex = RenderStateManager->AddPipelineStateObject(volumetricLightDirectional);

		const SPSODescription volumetricLightPoint =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::PointAndSpotLight)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::VolumetricPoint)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Position4)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::AdditiveBlend)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::FrontfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		VolumetricPointLightPSOIndex = RenderStateManager->AddPipelineStateObject(volumetricLightPoint);

		const SPSODescription volumetricLightSpot =
		{
			.VertexShader = RenderStateManager->VertexShaders[STATIC_U8(EVertexShaders::PointAndSpotLight)],
			.PixelShader = RenderStateManager->PixelShaders[STATIC_U8(EPixelShaders::VolumetricSpot)],
			.GeometryShader = nullptr,
			.ComputeShader = nullptr,
			.InputLayout = RenderStateManager->InputLayouts[STATIC_U8(EInputLayoutType::Position4)],
			.Topology = ETopologies::TriangleList,
			.BlendState = RenderStateManager->BlendStates[STATIC_U8(EBlendStates::AdditiveBlend)],
			.RasterizerState = RenderStateManager->RasterizerStates[STATIC_U8(ERasterizerStates::FrontfaceCulling)],
			.DepthStencilState = RenderStateManager->DepthStencilStates[STATIC_U8(EDepthStencilStates::Default)],
			.RootSignature = nullptr
		};
		VolumetricSpotlightPSOIndex = RenderStateManager->AddPipelineStateObject(volumetricLightSpot);
	}

	void CLightRenderer::RenderStaticMeshDepthPrePass(const SStaticMeshShadowRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(StaticMeshShadowPassPSOIndex);

		RenderStateManager->IASetVertexBuffers(0, { passData.VertexBuffer, passData.TransformBuffer });
		RenderStateManager->IASetIndexBuffer(*passData.IndexBuffer->DataBuffer);

		RenderStateManager->VSSetConstantBuffer(0, *passData.FrameBuffer->DataBuffer);

		RenderStateManager->Viewports[passData.ShadowmapViewportIndex].SetViewport();
		
		RenderStateManager->DrawIndexedInstanced(passData.IndexCount, passData.InstanceCount, 0, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;			
	}

	void CLightRenderer::RenderDirectionalLight(const SDirectionalLightRenderData& passData)
	{
		// add Alpha blend PS shader
		RenderStateManager->TrySetPipelineStateObject(DirectionalLightPSOIndex);

		RenderStateManager->IASetVertexBuffer(0, CDataBuffer::Null, 0, 0);
		RenderStateManager->IASetIndexBuffer(CDataBuffer::Null);

		RenderStateManager->PSSetConstantBuffer(2, *passData.LightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(5, *passData.ShadowmapBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(7, *passData.EmissiveBuffer->DataBuffer);

		// Imported resource from cubemap texture, declared in setup
		passData.CubemapTexture->RenderTexture->SetAsPSResourceOnSlot(0);

		RenderStateManager->Draw(3, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CLightRenderer::RenderPointLight(const SPointLightRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(PointLightPSOIndex);
		
		RenderStateManager->IASetVertexBuffer(0, RenderStateManager->VertexBuffers[STATIC_U8(EVertexBufferPrimitives::PointLightCube)], RenderStateManager->MeshVertexStrides[1], RenderStateManager->MeshVertexOffsets[0]);
		RenderStateManager->IASetIndexBuffer(RenderStateManager->IndexBuffers[STATIC_U8(EDefaultIndexBuffers::PointLightCube)]);

		RenderStateManager->VSSetConstantBuffer(3, *passData.LightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(3, *passData.LightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(5, *passData.ShadowmapBuffer->DataBuffer);

		RenderStateManager->DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CLightRenderer::RenderSpotlight(const SSpotlightRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(SpotlightPSOIndex);

		// TODO.NW: look at these hard coded indices, surely we have a representation for them?
		RenderStateManager->IASetVertexBuffer(0, RenderStateManager->VertexBuffers[1], RenderStateManager->MeshVertexStrides[1], RenderStateManager->MeshVertexOffsets[0]);
		RenderStateManager->IASetIndexBuffer(RenderStateManager->IndexBuffers[STATIC_U8(EDefaultIndexBuffers::PointLightCube)]);

		RenderStateManager->VSSetConstantBuffer(3, *passData.PointLightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(3, *passData.SpotlightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(5, *passData.ShadowmapBuffer->DataBuffer);

		RenderStateManager->DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CLightRenderer::RenderVolumetricDirectionalLight(const SVolumetricDirectionalLightRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(VolumetricDirectionalLightPSOIndex);
		
		RenderStateManager->IASetVertexBuffer(0, CDataBuffer::Null, 0, 0);
		RenderStateManager->IASetIndexBuffer(CDataBuffer::Null);

		RenderStateManager->PSSetConstantBuffer(1, *passData.LightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(4, *passData.VolumetricLightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(5, *passData.ShadowmapBuffer->DataBuffer);
		
		RenderStateManager->Draw(3, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CLightRenderer::RenderVolumetricPointLight(const SVolumetricPointLightRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(VolumetricPointLightPSOIndex);

		RenderStateManager->IASetVertexBuffer(0, RenderStateManager->VertexBuffers[1], RenderStateManager->MeshVertexStrides[1], RenderStateManager->MeshVertexOffsets[0]);
		RenderStateManager->IASetIndexBuffer(RenderStateManager->IndexBuffers[STATIC_U8(EDefaultIndexBuffers::PointLightCube)]);

		RenderStateManager->VSSetConstantBuffer(3, *passData.LightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(3, *passData.LightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(4, *passData.VolumetricLightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(5, *passData.ShadowmapBuffer->DataBuffer);

		RenderStateManager->DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CLightRenderer::RenderVolumetricSpotlight(const SVolumetricSpotlightRenderData& passData)
	{
		RenderStateManager->TrySetPipelineStateObject(VolumetricSpotlightPSOIndex);

		RenderStateManager->IASetVertexBuffer(0, RenderStateManager->VertexBuffers[1], RenderStateManager->MeshVertexStrides[1], RenderStateManager->MeshVertexOffsets[0]);
		RenderStateManager->IASetIndexBuffer(RenderStateManager->IndexBuffers[STATIC_U8(EDefaultIndexBuffers::PointLightCube)]);

		RenderStateManager->VSSetConstantBuffer(3, *passData.PointLightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(3, *passData.SpotlightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(4, *passData.VolumetricLightBuffer->DataBuffer);
		RenderStateManager->PSSetConstantBuffer(5, *passData.ShadowmapBuffer->DataBuffer);

		RenderStateManager->DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}
}
