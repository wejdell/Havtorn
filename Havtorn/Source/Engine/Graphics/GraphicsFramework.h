// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
//#include "Core/WindowsInclude.h"

struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;

namespace Havtorn
{
	class CPlatformManager;

	class CGraphicsFramework
	{
	public:
		CGraphicsFramework() = default;
		~CGraphicsFramework();

		void EndFrame();

		bool Init(CPlatformManager* platformManager);

		void ToggleFullscreenState(bool setFullscreen);

		ENGINE_API ID3D11Device* GetDevice() const;
		ENGINE_API ID3D11DeviceContext* GetContext() const;
		ENGINE_API ID3D11Texture2D* GetBackbufferTexture() const;
		ENGINE_API IDXGISwapChain* GetSwapChain() const;

	private:
		WinComPtr<IDXGISwapChain> SwapChain;
		WinComPtr<ID3D11Device> Device;
		WinComPtr<ID3D11DeviceContext> Context;
	};
}
