// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include <Core.h>
#include <CoreTypes.h>

#ifdef HV_RENDER_BACKEND_DIRECTX11
#include <d3d11.h>
#endif

namespace Havtorn
{
	class CPlatformManager;
	class CRenderImpl;

#ifdef HV_RENDER_BACKEND_DIRECTX11
	using SRenderDevice = ID3D11Device;
	using SRenderContext = ID3D11DeviceContext;
	using SRenderTexture = ID3D11Texture2D;
	using SRenderSwapChain = IDXGISwapChain;

	enum class ETopologies : U8
	{
		TriangleList = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		LineList = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		PointList = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
	};
#endif

	class CRHI
	{
	public:
		CRHI(CPlatformManager* platformManager);
		~CRHI();

		PLATFORM_API void EndFrame();

		void ToggleFullscreenState(bool setFullscreen);

		PLATFORM_API SRenderDevice* GetDevice() const;
		PLATFORM_API SRenderContext* GetContext() const;
		PLATFORM_API SRenderTexture* GetBackbufferTexture() const;
		PLATFORM_API SRenderSwapChain* GetSwapChain() const;

	private:
		Ptr<CRenderImpl> Impl;
	};
}
