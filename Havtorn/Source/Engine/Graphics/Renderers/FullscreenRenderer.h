// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"
#include <array>

#include "Graphics/RenderStateManager.h"
#include "Graphics/GraphicsEnums.h"
#include "Graphics/RenderingPrimitives/DataBuffer.h"
#include "Graphics/RenderingPrimitives/RenderTexture.h"

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
	HV_ASSERT_BUFFER(SPostProcessingBufferData)

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

	private:
		CFullscreenRenderer() = default;
		~CFullscreenRenderer();
		bool Init(CRHI* rhi, CRenderManager* manager);
		void Render(const EPixelShaders effect, const CRenderStateManager& stateManager);

		SPostProcessingBufferData GetPostProcessBuffer() const;
		void SetPostProcessBuffer(const SPostProcessingBufferData& data);

		const std::string ShaderRoot = "Shaders/";

		SPostProcessingBufferData PostProcessingBufferData;

		CDataBuffer FullscreenDataBuffer;
		CDataBuffer FrameBuffer;
		CDataBuffer PostProcessingBuffer;

		CRenderTexture NoiseTexture;
		SVector4 Kernel[KernelSize];

		CRenderManager* Manager = nullptr;
	};
}
