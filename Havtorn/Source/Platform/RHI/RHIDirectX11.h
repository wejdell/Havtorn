// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <EngineTypes.h>

#include <d3d11.h>

struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;

namespace Havtorn
{
	class CPlatformManager;

	class CRenderImpl
	{
	public:
		CRenderImpl() = default;
		~CRenderImpl();

		void EndFrame();

		bool Init(CPlatformManager* platformManager);

		void ToggleFullscreenState(bool setFullscreen);

		ID3D11Device* GetDevice() const;
		ID3D11DeviceContext* GetContext() const;
		ID3D11Texture2D* GetBackbufferTexture() const;
		IDXGISwapChain* GetSwapChain() const;

	private:
		WinComPtr<IDXGISwapChain> SwapChain;
		WinComPtr<ID3D11Device> Device;
		WinComPtr<ID3D11DeviceContext> Context;
	};
}