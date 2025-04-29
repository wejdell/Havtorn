// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "FullScreenTexture.h"
#include <d3d11.h>

namespace Havtorn
{
	void CRenderTexture::ClearTexture(SVector4 clearColor)
	{
		Context->ClearRenderTargetView(RenderTarget, &clearColor.X);
	}

	void CRenderTexture::ClearDepth(F32 /*clearDepth*/, U32 /*clearStencil*/) 
	{
		Context->ClearDepthStencilView(Depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	void CRenderTexture::SetAsActiveTarget(CRenderTexture* depth) 
	{
		if (depth) 
			Context->OMSetRenderTargets(1, &RenderTarget, depth->Depth);
		
		else 
			Context->OMSetRenderTargets(1, &RenderTarget, nullptr);
		
		Context->RSSetViewports(1, Viewport);
	}

	void CRenderTexture::SetAsDepthTarget()
	{
		Context->OMSetRenderTargets(0, NULL, Depth);
		Context->RSSetViewports(1, Viewport);
	}

	void CRenderTexture::SetAsDepthTarget(CRenderTexture* intermediateRenderTarget)
	{
		Context->OMSetRenderTargets(1, &intermediateRenderTarget->RenderTarget, Depth);
		Context->RSSetViewports(1, Viewport);
	}

	void CRenderTexture::SetAsPSResourceOnSlot(U16 slot) 
	{
		Context->PSSetShaderResources(slot, 1, &ShaderResource);
	}

	void CRenderTexture::SetAsVSResourceOnSlot(U16 slot)
	{
		Context->VSSetShaderResources(slot, 1, &ShaderResource);
	}

	void* CRenderTexture::MapToCPUFromGPUTexture(ID3D11Texture2D* gpuTexture)
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

	void CRenderTexture::CopyFromTexture(ID3D11Texture2D* texture)
	{
		Context->CopyResource(Texture, texture);
	}

	void CRenderTexture::WriteToCPUTexture(void* data, U64 size)
	{
		if (!CPUAccess)
		{
			HV_LOG_WARN("UnmapFromCPU: Tried to write data to a CPU texture, but the texture was not bound correctly for CPU access. Skipping write.");
			return;
		}

		D3D11_MAPPED_SUBRESOURCE resourceDesc = {};
		Context->Map(Texture, 0, D3D11_MAP_WRITE, 0, &resourceDesc);
		
		memcpy(resourceDesc.pData, data, size);

		Context->Unmap(Texture, 0);
	}

	void CRenderTexture::UnmapFromCPU()
	{
		if (!CPUAccess)
		{
			HV_LOG_WARN("UnmapFromCPU: Tried to unmap texture data from a CPU texture, but the texture was not bound correctly for CPU access. Skipping unmapping.");
			return;
		}

		Context->Unmap(Texture, 0);
	}

	void CRenderTexture::Release()
	{
		if (IsRenderTexture)
			ReleaseTexture();
		else
			ReleaseDepth();
	}

	void CRenderTexture::ReleaseTexture()
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

	void CRenderTexture::ReleaseDepth()
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

	ID3D11Texture2D* const CRenderTexture::GetTexture() const
	{
		return Texture;
	}

	ID3D11ShaderResourceView* const CRenderTexture::GetShaderResourceView() const
	{
		return ShaderResource;
	}

	ENGINE_API ID3D11ShaderResourceView* CRenderTexture::MoveShaderResourceView()
	{
		auto movedResource = std::move(ShaderResource);
		ShaderResource = nullptr;
		return movedResource;
	}

	ID3D11RenderTargetView* const CRenderTexture::GetRenderTargetView() const
	{
		return RenderTarget;
	}
	ID3D11DepthStencilView* const CRenderTexture::GetDepthStencilView() const
	{
		return Depth;
	}
	D3D11_VIEWPORT* const CRenderTexture::GetViewport() const
	{
		return Viewport;
	}
}
