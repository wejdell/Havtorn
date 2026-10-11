// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"
#include <array>

#include "Graphics/RenderStateManager.h"
#include "Graphics/GraphicsEnums.h"
#include <RHI/RenderingPrimitives/DataBuffer.h>
#include <RHI/RenderingPrimitives/RenderTexture.h>

namespace Havtorn
{
	class CRHI;
	class CRenderManager;
	class CRenderStateManager;

	struct SPostProcessingBufferData
	{
		SVector4 WhitePointColor;
		F32 WhitePointIntensity;
		F32 Exposure;
		F32 SSAORadius;
		F32 SSAOSampleBias;
		F32 SSAOMagnitude;
		F32 SSAOContrast;

		I32 IsReinhard;
		I32 IsUncharted;
		I32 IsACES;
		I32 IsAgX;

		F32 EmissiveStrength;
		F32 VignetteStrength;

		// AGX Settings
		F32 AgXMiddleGray;
		F32 AgXSlope;
		F32 AgXToePower;
		F32 AgXShoulderPower;
		F32 AgXCompressionR;
		F32 AgXCompressionG;
		F32 AgXCompressionB;
		F32 AgXSaturation;
		F32 AgXLerp;
		SVector Padding;

		SVector4 VignetteColor;
	};
	HV_ASSERT_BUFFER(SPostProcessingBufferData);

	struct SFullscreenRenderData
	{
		EPixelShaders PixelShader = EPixelShaders::FullscreenCopy;
		EBlendStates BlendState = EBlendStates::Disable;
		SRenderResource* RenderTarget = nullptr;
		SRenderResource* TextureResource1 = nullptr;
		SRenderResource* TextureResource2 = nullptr;
	};

	struct SBilateralBlurRenderData
	{
		SRenderResource* RenderTarget = nullptr;
		SRenderResource* DepthTexture = nullptr;
		SRenderResource* DownsamplingDepthTexture = nullptr;
		SRenderResource* PrimaryBlurTexture = nullptr;
		SRenderResource* SecondaryBlurTexture = nullptr;
	};

	struct SGaussianBlurRenderData
	{
		SRenderResource* RenderTarget = nullptr;
		SRenderResource* HalfResolutionTexture = nullptr;
		SRenderResource* PrimaryBlurTexture = nullptr; // 1/4 resolution
		SRenderResource* SecondaryBlurTexture = nullptr; // 1/4 resolution
	};

	class CFullscreenRenderer 
	{
	public:
		friend CRenderManager;

	private:
		static const U16 KernelSize = 16;

	private:
		struct SFullscreenData 
		{
			SVector2<F32> Resolution;
			SVector2<F32> NoiseScale;
			SVector4 SampleKernel[KernelSize];
		} FullscreenData;
		HV_ASSERT_BUFFER(SFullscreenData)

		struct SFrameBufferData
		{
			SMatrix ToCameraSpace;
			SMatrix ToWorldFromCamera;
			SMatrix ToProjectionSpace;
			SMatrix ToCameraFromProjection;
			SVector4 CameraPosition;
		} FrameBufferData;
		HV_ASSERT_BUFFER(SFrameBufferData)

	public:
		void Render(const SFullscreenRenderData& renderData);
		void RenderBilateralBlur(const SBilateralBlurRenderData& renderData);
		void RenderGaussianBlur(const SGaussianBlurRenderData& renderData);

	private:
		CFullscreenRenderer() = default;
		~CFullscreenRenderer();
		bool Init(CRHI* rhi, CRenderManager* manager);
		void Render(const EPixelShaders effect, const EBlendStates blendState);
		
		void BindBufferData();
		void PostDrawCall(const EPixelShaders shader, const EBlendStates blendState);
		
		SPostProcessingBufferData GetPostProcessBuffer() const;
		void SetPostProcessBuffer(const SPostProcessingBufferData& data);

		const std::string ShaderRoot = "Shaders/";

		SPostProcessingBufferData PostProcessingBufferData;

		CDataBuffer FullscreenDataBuffer;
		CDataBuffer ViewDataBuffer;
		CDataBuffer PostProcessingBuffer;

		CRenderTexture NoiseTexture;
		SVector4 Kernel[KernelSize];

		CRenderManager* Manager = nullptr;
		CRenderStateManager* RenderStateManager = nullptr;

		U16 FullscreenRenderPassPSOIndex = 0;
	};
}
