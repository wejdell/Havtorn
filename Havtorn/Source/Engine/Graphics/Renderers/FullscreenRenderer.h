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
	class CGraphicsFramework;
	class CRenderManager;
	class CRenderStateManager;

	class CFullscreenRenderer 
	{
	public:
		struct SPostProcessingBufferData
		{
			SVector4 WhitePointColor = SVector4::Zero;
			F32 WhitePointIntensity = 1.0f;
			F32 Exposure = 1.0f;
			F32 SSAORadius = 0.25f;
			F32 SSAOSampleBias = 0.2f;
			F32 SSAOMagnitude = 0.2f;
			F32 SSAOContrast = 1.0f;

			I32 IsReinhard = 0;
			I32 IsUncharted = 1;
			I32 IsACES = 1;

			F32 EmissiveStrength = 20.0f;

			F32 VignetteStrength = 0.25f;
			F32 Padding = FLT_MAX;

			SVector4 VignetteColor = SVector4::Zero;
		};
		HV_ASSERT_BUFFER(SPostProcessingBufferData)

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
		bool Init(CGraphicsFramework* framework, CRenderManager* manager);
		void Render(const EPixelShaders effect, const CRenderStateManager& stateManager);

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
