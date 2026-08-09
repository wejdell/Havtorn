// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "RHI.h"
#include "hvpch.h"

#include <../Platform/PlatformManager.h>
#include <d3d11.h>

#pragma comment(lib, "runtimeobject.lib")
#pragma comment(lib, "d3d11.lib")

template<typename T>
using WinComPtr = Microsoft::WRL::ComPtr<T>;

namespace Havtorn
{
	class CRenderImpl
	{
	public:
		~CRenderImpl()
		{
			BOOL isFullscreen;
			SwapChain->GetFullscreenState(&isFullscreen, nullptr);

			// if isFullscreen
			if (isFullscreen <= 0)
			{
				SwapChain->SetFullscreenState(FALSE, NULL);
			}
		}

		void EndFrame()
		{
			SwapChain->Present(0, 0);
		}

		bool Init(CPlatformManager* platformManager)
		{
			if (!platformManager)
				return false;

			D3D11_CREATE_DEVICE_FLAG createFlag = static_cast<D3D11_CREATE_DEVICE_FLAG>(0);
	#if _DEBUG
			createFlag = D3D11_CREATE_DEVICE_DEBUG;
	#endif

			DXGI_SWAP_CHAIN_DESC swapchainDesc = {};
			swapchainDesc.BufferCount = 1;
			swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchainDesc.OutputWindow = platformManager->GetWindowHandle();
			swapchainDesc.SampleDesc.Count = 1;
			swapchainDesc.Windowed = true;
			ENGINE_HR_MESSAGE(D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				createFlag,
				nullptr,
				0,
				D3D11_SDK_VERSION,
				&swapchainDesc,
				&SwapChain,
				&Device,
				nullptr,
				&Context), "Failed to created Device and Swap Chain.");

			return true;
		}

		void ToggleFullscreenState(bool setFullscreen)
		{
			SwapChain->SetFullscreenState(setFullscreen, NULL);
		}

		ID3D11Device* GetDevice() const
		{
			return Device.Get();
		}

		ID3D11DeviceContext* GetContext() const
		{
			return Context.Get();
		}

		ID3D11Texture2D* GetBackbufferTexture() const
		{
			ID3D11Texture2D* backbufferTexture = nullptr;
			ENGINE_HR_MESSAGE(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbufferTexture), "Failed to Get Buffer");
			return backbufferTexture;
		}

		IDXGISwapChain* GetSwapChain() const
		{
			return SwapChain.Get();
		}

	private:
		WinComPtr<IDXGISwapChain> SwapChain;
		WinComPtr<ID3D11Device> Device;
		WinComPtr<ID3D11DeviceContext> Context;
	};

	CRHI::CRHI(CPlatformManager* platformManager)
		: Impl(std::make_unique<CRenderImpl>())
	{
		if (!Impl->Init(platformManager))
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
