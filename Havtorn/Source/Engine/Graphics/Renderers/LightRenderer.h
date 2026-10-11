// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "SharedRendererData.h"

namespace Havtorn
{
	class CRenderStateManager;
	struct SRenderResource;

	struct SDirectionalLightBufferData
	{
		SVector4 ToDirectionalLight;
		SVector4 DirectionalLightColor;
	};
	HV_ASSERT_BUFFER(SDirectionalLightBufferData)

	struct SPointLightBufferData
	{
		SMatrix ToWorldFromObject;
		SVector4 ColorAndIntensity;
		SVector4 PositionAndRange;
	};
	HV_ASSERT_BUFFER(SPointLightBufferData)

	struct SSpotLightBufferData
	{
		SVector4 ColorAndIntensity;
		SVector4 PositionAndRange; // TODO.NW: Either remove the common members of point light buffer data and this, or remove usage of the point light buffer (think the common vertex shader expects point light buffer data)
		SVector4 Direction;
		SVector4 DirectionNormal1;
		SVector4 DirectionNormal2;
		F32 OuterAngle = 0.0f;
		F32 InnerAngle = 0.0f;
		SVector2<F32> Padding;
	};
	HV_ASSERT_BUFFER(SSpotLightBufferData)

	struct SShadowmapBufferData // TODO.NW: For arrayed shadowmap buffers (point light uses 6 of them), we should only have separate entries for the two matrices (the rest is data common for all sides)
	{
		SMatrix ToShadowmapView;
		SMatrix ToShadowmapProjection;
		SVector4 ShadowmapPosition;
		SVector2<F32> ShadowmapResolution;
		SVector2<F32> ShadowAtlasResolution;
		SVector2<F32> ShadowmapStartingUV;
		F32 ShadowTestTolerance = 0.0f;
		F32 Padding = -1.0f;
	};
	HV_ASSERT_BUFFER(SShadowmapBufferData)

	struct SVolumetricLightBufferData
	{
		F32 NumberOfSamplesReciprocal = (1.0f / 16.0f);
		F32 LightPower = 500000.0f;
		F32 ScatteringProbability = 0.0001f;
		F32 HenyeyGreensteinGValue = 0.0f;
	};
	HV_ASSERT_BUFFER(SVolumetricLightBufferData)

	struct SEmissiveBufferData
	{
		F32 EmissiveStrength = 1.0f;
		SVector Padding;
	};
	HV_ASSERT_BUFFER(SEmissiveBufferData)

	struct SLightConstantRenderData
	{
		SRenderResource* GBufferAlbedoTexture = nullptr;
		SRenderResource* GBufferWNormalTexture = nullptr;
		SRenderResource* GBufferVNormalTexture = nullptr;
		SRenderResource* GBufferMaterialTexture = nullptr;
		SRenderResource* IntermediateDepthTexture = nullptr;
		SRenderResource* ShadowAtlasDepthTexture = nullptr;
		SRenderResource* SSAOBlurTexture = nullptr;
	};

	struct SVolumetricConstantRenderData
	{
		SRenderResource* IntermediateDepthTexture = nullptr;
		SRenderResource* ShadowAtlasDepthTexture = nullptr;
	};

	struct SStaticMeshShadowRenderData
	{
		SRenderResource* ViewDataBuffer = nullptr; 
		SRenderResource* VertexBuffer = nullptr;
		SRenderResource* IndexBuffer = nullptr;
		SRenderResource* TransformBuffer = nullptr;
		U32 IndexCount = 0;
		U32 InstanceCount = 0;
		U16 ShadowmapViewportIndex = 0;
	};

	struct SDirectionalLightRenderData
	{
		SRenderResource* LightBuffer = nullptr;
		SRenderResource* ShadowmapBuffer = nullptr;
		SRenderResource* EmissiveBuffer = nullptr;
		SRenderResource* CubemapTexture = nullptr;
	};

	struct SPointLightRenderData
	{
		SRenderResource* LightBuffer = nullptr;
		SRenderResource* ShadowmapBuffer = nullptr;
	};

	struct SSpotlightRenderData
	{
		SRenderResource* PointLightBuffer = nullptr;
		SRenderResource* SpotlightBuffer = nullptr;
		SRenderResource* ShadowmapBuffer = nullptr;
	};

	struct SSkyboxRenderData
	{
		SRenderResource* CubemapTexture = nullptr;
	};

	struct SVolumetricDirectionalLightRenderData
	{
		SRenderResource* LightBuffer = nullptr;
		SRenderResource* VolumetricLightBuffer = nullptr;
		SRenderResource* ShadowmapBuffer = nullptr;
	};

	struct SVolumetricPointLightRenderData
	{
		SRenderResource* LightBuffer = nullptr;
		SRenderResource* VolumetricLightBuffer = nullptr;
		SRenderResource* ShadowmapBuffer = nullptr;
	};

	struct SVolumetricSpotlightRenderData
	{
		SRenderResource* PointLightBuffer = nullptr;
		SRenderResource* SpotlightBuffer = nullptr;
		SRenderResource* VolumetricLightBuffer = nullptr;
		SRenderResource* ShadowmapBuffer = nullptr;
	};

	class CLightRenderer 
	{
	public:
		CLightRenderer(CRenderStateManager* stateManager);
		~CLightRenderer() = default;

		void SetupShadowPrepassRender(const SMainTargetRenderData& targetData);
		void SetupLightRender(const SGeometryConstantRenderData& viewData, const SLightConstantRenderData& constantData, const SMainTargetRenderData& targetData);
		void SetupSkyboxRender(const SGeometryConstantRenderData& viewData, const SMainTargetRenderData& targetData);
		void SetupVolumetricRender(const SGeometryConstantRenderData& viewData, const SVolumetricConstantRenderData& constantData, const SMainTargetRenderData& targetData);

		void RenderStaticMeshDepthPrePass(const SStaticMeshShadowRenderData& passData);

		void RenderDirectionalLight(const SDirectionalLightRenderData& passData);
		void RenderPointLight(const SPointLightRenderData& passData);
		void RenderSpotlight(const SSpotlightRenderData& passData);
		void RenderSkybox(const SSkyboxRenderData& passData);

		void RenderVolumetricDirectionalLight(const SVolumetricDirectionalLightRenderData& passData);
		void RenderVolumetricPointLight(const SVolumetricPointLightRenderData& passData);
		void RenderVolumetricSpotlight(const SVolumetricSpotlightRenderData& passData);
	
	private:
		U16 StaticMeshShadowPassPSOIndex = 0;
		U16 DirectionalLightPSOIndex = 0;
		U16 PointLightPSOIndex = 0;
		U16 SpotlightPSOIndex = 0;
		U16 SkyboxPassPSOIndex = 0;
		U16 VolumetricDirectionalLightPSOIndex = 0;
		U16 VolumetricPointLightPSOIndex = 0;
		U16 VolumetricSpotlightPSOIndex = 0;

		CRenderStateManager* RenderStateManager = nullptr;
	};
}
