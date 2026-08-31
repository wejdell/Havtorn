// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "LightRenderer.h"
#include "Engine.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderStateManager.h" 
#include "Graphics/GraphicsUtilities.h"

#include <RHI/RHI.h>

#include <d3d11.h>

namespace Havtorn
{
	CLightRenderer::~CLightRenderer() 
	{}

	bool CLightRenderer::Init(CRHI* rhi, CRenderManager* renderManager, CRenderStateManager* stateManager)
	{
		if (!rhi) 
			return false;

		Manager = renderManager;
		if (!Manager)
			return false;

		RenderStateManager = stateManager;

		DirectionalLightBuffer.CreateBuffer("Directional Light Buffer", rhi, sizeof(SDirectionalLightBufferData));
		PointLightBuffer.CreateBuffer("Point Light Buffer", rhi, sizeof(SPointLightBufferData));
		SpotLightBuffer.CreateBuffer("Spot Light Buffer", rhi, sizeof(SSpotLightBufferData));
		
		FrameBuffer.CreateBuffer("Frame Buffer", rhi, sizeof(SFrameBufferData));
		InstancedTransformBuffer.CreateBuffer("Instanced Transform Buffer", rhi, sizeof(SMatrix) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);

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
		DirectonalLightPSOIndex = RenderStateManager->AddPipelineStateObject(lightingDirectional);

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
		SpotLightPSOIndex = RenderStateManager->AddPipelineStateObject(lightingSpot);

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
		VolumetricSpotLightPSOIndex = RenderStateManager->AddPipelineStateObject(volumetricLightSpot);

		return true;
	}

	U64 CLightRenderer::RenderStaticMeshDepthPrePass(const std::vector<SShadowmapViewData>& shadowmapData, const std::vector<SDrawCallData>& drawCallData, const std::vector<SMatrix>& instanceTransforms, const U64 currentPSOHash)
	{
		const U64 newPSOHash = RenderStateManager->TrySetPipelineStateObject(StaticMeshShadowPassPSOIndex, currentPSOHash);

		InstancedTransformBuffer.BindBuffer(instanceTransforms);

		for (const auto& shadowmapView : shadowmapData)
		{
			FrameBufferData.ToCameraFromWorld = shadowmapView.ShadowViewMatrix;
			FrameBufferData.ToWorldFromCamera = shadowmapView.ShadowViewMatrix.FastInverse();
			FrameBufferData.ToProjectionFromCamera = shadowmapView.ShadowProjectionMatrix;
			FrameBufferData.ToCameraFromProjection = shadowmapView.ShadowProjectionMatrix.Inverse();
			FrameBufferData.CameraPosition = shadowmapView.ShadowPosition;

			FrameBuffer.BindBuffer(FrameBufferData);
			RenderStateManager->VSSetConstantBuffer(0, FrameBuffer);

			RenderStateManager->Viewports[shadowmapView.ShadowmapViewportIndex].SetViewport();

			for (const SDrawCallData& drawData : drawCallData)
			{
				const std::vector<CDataBuffer> buffers = { RenderStateManager->VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
				const U32 strides[2] = { RenderStateManager->MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
				const U32 offsets[2] = { RenderStateManager->MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
				RenderStateManager->IASetVertexBuffers(0, 2, buffers, strides, offsets);
				RenderStateManager->IASetIndexBuffer(RenderStateManager->IndexBuffers[drawData.IndexBufferIndex]);
				RenderStateManager->DrawIndexedInstanced(drawData.IndexCount, STATIC_U32(instanceTransforms.size()), 0, 0, 0);
				CRenderManager::NumberOfDrawCallsThisFrame++;
			}
		}

		return newPSOHash;
	}
}
