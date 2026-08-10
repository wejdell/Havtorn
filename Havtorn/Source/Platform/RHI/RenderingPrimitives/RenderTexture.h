// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>

#include <d3d11.h>

namespace Havtorn
{
	class CRenderTextureFactory;
	class CRenderTexture;
	class CGBuffer;

	class CStaticRenderTexture
	{
		friend CRenderTextureFactory;
		friend CRenderTexture;

	public:
		CStaticRenderTexture() = default;
		~CStaticRenderTexture() = default;
		PLATFORM_API void SetAsPSResourceOnSlot(U16 slot) const;
		PLATFORM_API void SetAsVSResourceOnSlot(U16 slot) const;
		PLATFORM_API void ReleaseTexture();
	
		// TODO.NW: This is silly. We should unify our render calls so they may all take a RenderTarget instead
		PLATFORM_API intptr_t GetResource() const;
		PLATFORM_API ID3D11ShaderResourceView* GetShaderResource() const;
		PLATFORM_API ID3D11ShaderResourceView* const* GetShaderResourceView() const;

	private:
		ID3D11DeviceContext* Context = nullptr;
		ID3D11ShaderResourceView* ShaderResource = nullptr;
	};

	// TODO.NW: Look over this class again. It's good to have an abstraction but now it's functioning as both SRV and render target/depth.
	
	class CRenderTexture
	{
		friend CRenderTextureFactory;
		friend CGBuffer;
	
	public:
		CRenderTexture() = default;
		~CRenderTexture() = default;
		PLATFORM_API CRenderTexture(const CStaticRenderTexture& staticTexture);
		PLATFORM_API void ClearTexture(SVector4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f });
		PLATFORM_API void ClearDepth(F32 clearDepth = 1.0f, U32 clearStencil = 0);
		PLATFORM_API void SetAsActiveTarget(CRenderTexture* depth = nullptr);
		PLATFORM_API void SetAsDepthTarget();
		PLATFORM_API void SetAsDepthTarget(CRenderTexture* intermediateRenderTarget);
		PLATFORM_API void SetAsPSResourceOnSlot(U16 slot);
		PLATFORM_API void SetAsVSResourceOnSlot(U16 slot);
		PLATFORM_API void* MapToCPUFromGPUTexture(ID3D11Texture2D* gpuTexture);
		PLATFORM_API void CopyFromTexture(ID3D11Texture2D* texture);
		PLATFORM_API void WriteToCPUTexture(void* data, U64 size);
		PLATFORM_API void UnmapFromCPU();
		PLATFORM_API void ReleaseTexture();
		PLATFORM_API void ReleaseDepth();
		
		PLATFORM_API bool IsShaderResourceValid() const;
		PLATFORM_API void Release();

		PLATFORM_API ID3D11Texture2D* const GetTexture() const;
		PLATFORM_API ID3D11ShaderResourceView* const GetShaderResourceView() const;
		PLATFORM_API ID3D11ShaderResourceView* MoveShaderResourceView();
		PLATFORM_API ID3D11RenderTargetView* const GetRenderTargetView() const;
		PLATFORM_API ID3D11DepthStencilView* const GetDepthStencilView() const;
		PLATFORM_API D3D11_VIEWPORT* const GetViewport();

	private:
		ID3D11DeviceContext* Context = nullptr;
		ID3D11Texture2D* Texture = nullptr;

		union 
		{
			ID3D11RenderTargetView* RenderTarget = nullptr;
			ID3D11DepthStencilView* Depth;
		};

		ID3D11ShaderResourceView* ShaderResource = nullptr;
		D3D11_VIEWPORT Viewport = {};

		// TODO.NW: Keep this now for debugging
		bool IsFromStaticTexture = false;
		bool IsRenderTexture = true;
		bool CPUAccess = true;
	};
}
