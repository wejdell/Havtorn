// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderViewport.h"
#include "RHI/RHI.h"

namespace Havtorn
{
	CRenderViewport::CRenderViewport(CRHI* rhi, const F32 topLeftX, const F32 topLeftY, const F32 width, const F32 height, const F32 minDepth, const F32 maxDepth)
		: Context(rhi->GetContext())
		, TopLeftX(topLeftX)
		, TopLeftY(topLeftY)
		, Width(width)
		, Height(height)
		, MinDepth(minDepth)
		, MaxDepth(maxDepth)
	{
	}

	void CRenderViewport::SetViewport() const
	{
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = TopLeftX;
		viewport.TopLeftY = TopLeftY;
		viewport.Width = Width;
		viewport.Height = Height;
		viewport.MinDepth = MinDepth;
		viewport.MaxDepth = MaxDepth;
		Context->RSSetViewports(1, &viewport);
	}
}
