// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "FullscreenRenderer.h"
#include "Engine.h"
#include "Graphics/GraphicsFramework.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderStateManager.h" 
#include "Graphics/GraphicsUtilities.h"
#include <d3d11.h>

namespace Havtorn
{
	template<class T>
	void BindBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, T& bufferData, std::string bufferType)
	{
		D3D11_MAPPED_SUBRESOURCE localBufferData;
		ZeroMemory(&localBufferData, sizeof(D3D11_MAPPED_SUBRESOURCE));
		std::string errorMessage = bufferType + " could not be bound.";
		ENGINE_HR_MESSAGE(context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &localBufferData), errorMessage.c_str());

		memcpy(localBufferData.pData, &bufferData, sizeof(T));
		context->Unmap(buffer, 0);
	}

	CFullscreenRenderer::~CFullscreenRenderer() 
	{}

	bool CFullscreenRenderer::Init(CGraphicsFramework* framework, CRenderManager* renderManager) 
	{
		if (!framework) 
			return false;

		Manager = renderManager;
		if (!Manager)
			return false;

		FullscreenDataBuffer.CreateBuffer("Fullscreen Data Buffer", framework, sizeof(SFullscreenData));
		FrameBuffer.CreateBuffer("Frame Buffer", framework, sizeof(SFrameBufferData));
		PostProcessingBuffer.CreateBuffer("Post Processing Buffer", framework, sizeof(SPostProcessingBufferData));

#pragma region SSAO Setup
	// Hardcoded Kernel
		Kernel[0] = { 0.528985322f, 0.163332120f, 0.620016515f, 1.0f };
		Kernel[1] = { 0.573982120f, 0.378577918f, 0.470547318f, 1.0f };
		Kernel[2] = { 0.065050237f, 0.139410198f, 0.347815633f, 1.0f };
		Kernel[3] = { 0.041187014f, 0.130081877f, 0.164059237f, 1.0f };
		Kernel[4] = { -0.026605275f, 0.090929292f, 0.077286638f, 1.0f };
		Kernel[5] = { -0.113886870f, 0.154690191f, 0.197556734f, 1.0f };
		Kernel[6] = { -0.666800976f, 0.662895739f, 0.277599692f, 1.0f };
		Kernel[7] = { -0.399470448f, 0.096369371f, 0.417604893f, 1.0f };
		Kernel[8] = { -0.411310822f, -0.082451604f, 0.179119825f, 1.0f };
		Kernel[9] = { -0.117983297f, -0.095347963f, 0.374402136f, 1.0f };
		Kernel[10] = { -0.457335383f, -0.529036164f, 0.490310162f, 1.0f };
		Kernel[11] = { -0.119527563f, -0.291437626f, 0.206827655f, 1.0f };
		Kernel[12] = { 0.201868936f, -0.513456404f, 0.432056010f, 1.0f };
		Kernel[13] = { 0.096077450f, -0.107414119f, 0.527342558f, 1.0f };
		Kernel[14] = { 0.223280489f, -0.180109233f, 0.203371927f, 1.0f };
		Kernel[15] = { 0.163490131f, -0.039255358f, 0.532910645f, 1.0f };

		SVector4 noise[KernelSize];
		for (U16 i = 0; i < KernelSize; ++i)
		{
			noise[i] = SVector4(
				UMath::Random(-1.0f, 1.0f),
				UMath::Random(-1.0f, 1.0f),
				0.0f,
				0.0f
			);
			noise[i].Normalize();
		}

		U16 width = UMath::Sqrt(KernelSize);
		NoiseTexture = Manager->CreateRenderTextureFromData(SVector2<U16>(width), DXGI_FORMAT_R32G32B32A32_FLOAT, noise, sizeof(SVector4));
#pragma endregion

		PostProcessingBufferData.WhitePointColor = { 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f };
		PostProcessingBufferData.WhitePointIntensity = 1.0f;
		PostProcessingBufferData.Exposure = 1.1f;
		PostProcessingBufferData.IsReinhard = false;
		PostProcessingBufferData.IsUncharted = true;
		PostProcessingBufferData.IsACES = false;

		PostProcessingBufferData.SSAORadius = 0.6f;
		PostProcessingBufferData.SSAOSampleBias = 0.2420f;
		PostProcessingBufferData.SSAOMagnitude = 1.4f;
		PostProcessingBufferData.SSAOContrast = 0.6f;

		PostProcessingBufferData.EmissiveStrength = 2.1f;

		PostProcessingBufferData.VignetteStrength = 0.35f;
		PostProcessingBufferData.Padding = 0.0f;
		PostProcessingBufferData.VignetteColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		return true;
	}

	void CFullscreenRenderer::Render(const EPixelShaders effect, const CRenderStateManager& stateManager)
	{
		SVector2<U16> resolution = Manager->GetCurrentWindowResolution();
		FullscreenData.Resolution = SVector2<F32>(resolution.X, resolution.Y);
		FullscreenData.NoiseScale = { FullscreenData.Resolution.X / STATIC_F32(UMath::Sqrt(KernelSize)), FullscreenData.Resolution.Y / STATIC_F32(UMath::Sqrt(KernelSize)) };
		memcpy(&FullscreenData.SampleKernel[0], &Kernel[0], sizeof(Kernel));
		
		FullscreenDataBuffer.BindBuffer(FullscreenData);
		PostProcessingBuffer.BindBuffer(PostProcessingBufferData);

		stateManager.IASetTopology(ETopologies::TriangleList);
		stateManager.IASetInputLayout(EInputLayoutType::Null);
		stateManager.IASetVertexBuffer(0, CDataBuffer::Null, 0, 0);
		stateManager.IASetIndexBuffer(CDataBuffer::Null);

		stateManager.VSSetShader(EVertexShaders::Fullscreen);
		stateManager.PSSetShader(effect);
		stateManager.GSSetShader(EGeometryShaders::Null);
		stateManager.PSSetSampler(0, ESamplers::DefaultClamp);
		stateManager.PSSetSampler(1, ESamplers::DefaultWrap);
		stateManager.PSSetConstantBuffer(1, FullscreenDataBuffer);
		stateManager.PSSetConstantBuffer(2, PostProcessingBuffer);
		NoiseTexture.SetAsPSResourceOnSlot(23);

		stateManager.Draw(3, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;

		stateManager.ClearShaderResources();
	}
}
