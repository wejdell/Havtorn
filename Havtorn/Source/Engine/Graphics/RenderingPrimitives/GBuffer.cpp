// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GBuffer.h"
#include "RenderTexture.h"

namespace Havtorn
{
	CGBuffer::~CGBuffer()
	{
		// TODO.NW: Figure out why this causes a crash in the entity picking texture mapping logic
		//ReleaseResources();
	}

	void CGBuffer::ClearTextures(SVector4 clearColor)
	{
		for (UINT i = 0; i < STATIC_U64(EGBufferTextures::Count); ++i) 
		{
			Context->ClearRenderTargetView(RenderTargets[i], &clearColor.X);
		}
	}

	void CGBuffer::ReleaseRenderTargets()
	{
		std::array<ID3D11RenderTargetView*, STATIC_U64(EGBufferTextures::Count)> nullViews = { NULL, NULL, NULL, NULL, NULL };
		Context->OMSetRenderTargets(STATIC_U64(EGBufferTextures::Count), &nullViews[0], nullptr);
	}

	void CGBuffer::SetAsActiveTarget(CRenderTexture* depth, bool isUsingEditor)
	{
		auto depthStencilView = depth ? depth->Depth : nullptr;
		Context->OMSetRenderTargets(isUsingEditor ? STATIC_U64(EGBufferTextures::Count) : STATIC_U64(EGBufferTextures::Count) - 1, &RenderTargets[0], depthStencilView);
		Context->RSSetViewports(1, &Viewport);
	}

	void CGBuffer::SetAsPSResourceOnSlot(EGBufferTextures resource, U16 slot)
	{
		Context->PSSetShaderResources(slot, 1, &ShaderResources[STATIC_U64(resource)]);
	}

	void CGBuffer::SetAllAsResources(U16 startSlot)
	{
		// TODO.NW: Figure out why we do this, should the editor texture not be included in this call?
		Context->PSSetShaderResources(startSlot, STATIC_U64(EGBufferTextures::Count) - 1, &ShaderResources[0]);
	}

	ID3D11RenderTargetView* CGBuffer::GetEditorDataRenderTarget() const
	{
		return RenderTargets[STATIC_U64(EGBufferTextures::EditorData)];
	}

	ID3D11Texture2D* CGBuffer::GetEditorDataTexture() const
	{
		return Textures[STATIC_U64(EGBufferTextures::EditorData)];
	}

	const D3D11_VIEWPORT& CGBuffer::GetViewport() const
	{
		return Viewport;
	}

	void CGBuffer::ReleaseResources()
	{
		Context = nullptr;

		for (UINT i = 0; i < STATIC_U64(EGBufferTextures::Count); ++i)
		{
			Textures[i]->Release();
			Textures[i] = nullptr;
			RenderTargets[i]->Release();
			RenderTargets[i] = nullptr;
			ShaderResources[i]->Release();
			ShaderResources[i] = nullptr;
		}
	}
}
