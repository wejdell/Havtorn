// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "RHI/RHI.h"

#include "PlatformManager.h"

#include <Log.h>

#ifdef HV_RENDER_BACKEND_DIRECTX11
#include "RHIDirectX11.h"
#include <d3d11.h>

#pragma comment(lib, "runtimeobject.lib")
#pragma comment(lib, "d3d11.lib")

namespace Havtorn
{
	typedef ID3D11Device SRenderDevice;
	typedef ID3D11DeviceContext SRenderContext;
	typedef ID3D11Texture2D SRenderTexture;
	typedef IDXGISwapChain SRenderSwapChain;
}

#endif

#ifdef HV_RENDER_BACKEND_VULKAN
#include "RHIVulkan.h"
#endif

namespace Havtorn
{
	CRHI::CRHI(CPlatformManager* platformManager)
		: Impl(std::make_unique<CRenderImpl>())
	{
		if (Impl->Init(platformManager))
			HV_LOG_ERROR("CRHI::CRHI: Could not initialize render backend!");
	}

	CRHI::~CRHI()
	{
	}

	void CRHI::EndFrame()
	{
		Impl->EndFrame();
	}

	void CRHI::ToggleFullscreenState(bool setFullscreen)
	{
		Impl->ToggleFullscreenState(setFullscreen);
	}

	SRenderDevice* CRHI::GetDevice() const
	{
		return Impl->GetDevice();
	}

	SRenderContext* CRHI::GetContext() const
	{
		return Impl->GetContext();
	}

	SRenderTexture* CRHI::GetBackbufferTexture() const
	{
		return Impl->GetBackbufferTexture();
	}

	SRenderSwapChain* CRHI::GetSwapChain() const
	{
		return Impl->GetSwapChain();
	}
}
