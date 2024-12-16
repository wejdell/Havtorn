// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "FullScreenTexture.h"
#include <d3d11.h>

namespace Havtorn
{
	void CFullscreenTexture::ClearTexture(SVector4 clearColor) 
	{
		Context->ClearRenderTargetView(RenderTarget, &clearColor.X);
	}

	void CFullscreenTexture::ClearDepth(F32 /*clearDepth*/, U32 /*clearStencil*/) 
	{
		Context->ClearDepthStencilView(Depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	void CFullscreenTexture::SetAsActiveTarget(CFullscreenTexture* depth) 
	{
		if (depth) 
			Context->OMSetRenderTargets(1, &RenderTarget, depth->Depth);
		
		else 
			Context->OMSetRenderTargets(1, &RenderTarget, nullptr);
		
		Context->RSSetViewports(1, Viewport);
	}

	void CFullscreenTexture::SetAsDepthTarget()
	{
		Context->OMSetRenderTargets(0, NULL, Depth);
		Context->RSSetViewports(1, Viewport);
	}

	void CFullscreenTexture::SetAsDepthTarget(CFullscreenTexture* intermediateRenderTarget)
	{
		Context->OMSetRenderTargets(1, &intermediateRenderTarget->RenderTarget, Depth);
		Context->RSSetViewports(1, Viewport);
	}

	void CFullscreenTexture::SetAsResourceOnSlot(U16 slot) 
	{
		Context->PSSetShaderResources(slot, 1, &ShaderResource);
	}

	void* CFullscreenTexture::MapToCPUFromGPUTexture(ID3D11Texture2D* gpuTexture)
	{
		if (!CPUAccess)
		{
			HV_LOG_WARN("MapToCPUFromGPUTexture: Tried to map texture data to a CPU texture, but textures were not bound correctly for CPU access. Skipping mapping.");
			return nullptr;
		}

		Context->CopyResource(Texture, gpuTexture);
		D3D11_MAPPED_SUBRESOURCE resourceDesc = {};
		Context->Map(Texture, 0, D3D11_MAP_READ, 0, &resourceDesc);
		return resourceDesc.pData;
	}

	void CFullscreenTexture::UnmapFromCPU()
	{
		if (!CPUAccess)
		{
			HV_LOG_WARN("UnmapFromCPU: Tried to unmap texture data from a CPU texture, but the texture was not bound correctly for CPU access. Skipping unmapping.");
			return;
		}

		Context->Unmap(Texture, 0);
	}

	void CFullscreenTexture::ReleaseTexture()
	{
		Context = nullptr;
		Texture->Release();
		Texture = nullptr;
		RenderTarget->Release();
		RenderTarget = nullptr;
		
		if (ShaderResource)
		{
			ShaderResource->Release();
			ShaderResource = nullptr;
		}

		SAFE_DELETE(Viewport);
	}

	void CFullscreenTexture::ReleaseDepth()
	{
		Context = nullptr;
		Texture->Release();
		Texture = nullptr;
		Depth->Release();
		Depth = nullptr;
		
		if (ShaderResource)
		{
			ShaderResource->Release();
			ShaderResource = nullptr;
		}

		SAFE_DELETE(Viewport);
	}

	ID3D11Texture2D* const CFullscreenTexture::GetTexture() const
	{
		return Texture;
	}

	ID3D11ShaderResourceView* const CFullscreenTexture::GetShaderResourceView() const
	{
		return ShaderResource;
	}
}
