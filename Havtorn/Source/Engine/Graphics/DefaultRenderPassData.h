// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "RenderGraph.h"

namespace Havtorn
{
	struct SShadowmapPrePassData : public SRenderPassParams
	{
		SRenderResourceHandle ViewDataBufferHandle;
		SRenderResourceHandle TransformBufferHandle;

		SRenderResourceHandle EmptyRenderTargetHandle;
		SRenderResourceHandle ShadowmapRenderTargetHandle;
	};

	struct SGBufferBasePassData : public SRenderPassParams
	{
		SRenderResourceHandle ViewDataBufferHandle;
		SRenderResourceHandle TransformBufferHandle;
		SRenderResourceHandle MaterialBufferHandle;
		SRenderResourceHandle BoneBufferHandle;
		SRenderResourceHandle UVRectBufferHandle;
		SRenderResourceHandle ColorBufferHandle;
		SRenderResourceHandle EntityBufferHandle;

		SRenderResourceHandle GBufferAlbedoTargetHandle;
		SRenderResourceHandle GBufferWNormalTargetHandle;
		SRenderResourceHandle GBufferVNormalTargetHandle;
		SRenderResourceHandle GBufferMaterialTargetHandle;
		SRenderResourceHandle GBufferWorldPositionTargetHandle;
		SRenderResourceHandle GBufferEditorDataTargetHandle;
		SRenderResourceHandle IntermediateDepthTargetHandle;
	};

	struct SGeometryDepthCopyPassData : public SRenderPassParams
	{
		SRenderResourceHandle DepthTextureHandle;
		SRenderResourceHandle DepthRenderTargetHandle;
	};

	struct SGeometryDecalPassData : public SRenderPassParams
	{
		SRenderResourceHandle ViewDataBufferHandle;
		SRenderResourceHandle TransformBufferHandle;
		SRenderResourceHandle MaterialBufferHandle;
		SRenderResourceHandle DepthCopyTextureHandle;

		SRenderResourceHandle GBufferAlbedoTargetHandle;
		SRenderResourceHandle GBufferWNormalTargetHandle;
		SRenderResourceHandle GBufferVNormalTargetHandle;
		SRenderResourceHandle GBufferMaterialTargetHandle;
		SRenderResourceHandle IntermediateDepthTargetHandle;
	};

	struct SSSAOPassData : public SRenderPassParams
	{
		SRenderResourceHandle SSAOBufferTextureHandle;
		SRenderResourceHandle DepthTextureHandle;
		SRenderResourceHandle GBufferWNormalTextureHandle;
		SRenderResourceHandle SSAOBlurTargetHandle;
	};

	struct SLightPassData : public SRenderPassParams
	{
		SRenderResourceHandle ViewDataBufferHandle;
		SRenderResourceHandle GBufferAlbedoTextureHandle;
		SRenderResourceHandle GBufferWNormalTextureHandle;
		SRenderResourceHandle GBufferVNormalTextureHandle;
		SRenderResourceHandle GBufferMaterialTextureHandle;
		SRenderResourceHandle IntermediateDepthTextureHandle;
		SRenderResourceHandle ShadowAtlasDepthTextureHandle;
		SRenderResourceHandle SSAOBlurTextureHandle;

		SRenderResourceHandle DirectionalLightBufferHandle;
		SRenderResourceHandle PointLightBufferHandle;
		SRenderResourceHandle SpotlightBufferHandle;
		SRenderResourceHandle ShadowmapBufferHandle;
		SRenderResourceHandle EmissiveBufferHandle;

		SRenderResourceHandle MainRenderTargetHandle;
	};

	struct SVolumetricLightPassData : public SRenderPassParams
	{
		SRenderResourceHandle ViewDataBufferHandle;
		SRenderResourceHandle IntermediateDepthTextureHandle;
		SRenderResourceHandle ShadowAtlasDepthTextureHandle;
		SRenderResourceHandle DirectionalLightBufferHandle;
		SRenderResourceHandle PointLightBufferHandle;
		SRenderResourceHandle SpotlightBufferHandle;
		SRenderResourceHandle VolumetricLightBufferHandle;
		SRenderResourceHandle ShadowmapBufferHandle;
		SRenderResourceHandle AccumulationRenderTargetHandle;
	};

	struct SVolumetricBlurPassData : public SRenderPassParams
	{
		SRenderResourceHandle DepthTextureHandle;
		SRenderResourceHandle DownsamplingDepthTextureHandle;
		SRenderResourceHandle PrimaryBlurTextureHandle;
		SRenderResourceHandle SecondaryBlurTextureHandle;
		SRenderResourceHandle RenderTargetHandle;
	};

	struct SBloomPassData : public SRenderPassParams
	{
		SRenderResourceHandle BloomBufferTextureHandle;
		SRenderResourceHandle TargetCopyTextureHandle;
		SRenderResourceHandle HalfResolutionTextureHandle;
		SRenderResourceHandle PrimaryBlurTextureHandle;
		SRenderResourceHandle SecondaryBlurTextureHandle;
		SRenderResourceHandle MainRenderTargetHandle;
	};

	struct STonemappingPassData : public SRenderPassParams
	{
		SRenderResourceHandle RenderTargetTextureHandle;
		SRenderResourceHandle TonemappingTargetHandle;
	};

	struct SUIPassData : public SRenderPassParams
	{
		SRenderResourceHandle TransformBufferHandle;
		SRenderResourceHandle UVRectBufferHandle;
		SRenderResourceHandle ColorBufferHandle;

		SRenderResourceHandle MainRenderTargetHandle;
		SRenderResourceHandle DepthTargetHandle;
	};

	struct SDebugDrawPassData : public SRenderPassParams
	{
		SRenderResourceHandle ViewDataBufferHandle;
		SRenderResourceHandle DebugObjectBufferHandle;
		SRenderResourceHandle MainRenderTargetHandle;
		SRenderResourceHandle DepthTargetHandle;
	};

	struct SAntialiasingPassData : public SRenderPassParams
	{
		SRenderResourceHandle TonemappedTextureHandle;
		SRenderResourceHandle AntialiasingTargetHandle;
	};

	struct SGammaCorrectionPassData : public SRenderPassParams
	{
		SRenderResourceHandle AntialiasedTextureHandle;
		SRenderResourceHandle MainRenderTargetHandle;
	};

	struct SFinalizePassData : public SRenderPassParams
	{
		SRenderResourceHandle PrimaryOptionalTextureHandle;
		SRenderResourceHandle SecondaryOptionalTextureHandle;
		SRenderResourceHandle MainRenderTargetHandle;
	};
}