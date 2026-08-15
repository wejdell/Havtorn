// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RasterizerState.h"
#include "RHI/RHI.h"

#include <fstream>

namespace Havtorn
{
	CRasterizerState::CRasterizerState(const CRHI* rhi, const SRasterizerDescription& description)
		: Context(rhi->GetContext())
	{
		D3D11_RASTERIZER_DESC rasterizerDescription = {};
		rasterizerDescription.FillMode = static_cast<D3D11_FILL_MODE>(description.FillMode);
		rasterizerDescription.CullMode = static_cast<D3D11_CULL_MODE>(description.CullMode);
		rasterizerDescription.FrontCounterClockwise = description.FrontCounterClockwise;
		rasterizerDescription.DepthBias = description.DepthBias;
		rasterizerDescription.DepthBiasClamp = description.DepthBiasClamp;
		rasterizerDescription.SlopeScaledDepthBias = description.SlopeScaledDepthBias;
		rasterizerDescription.DepthClipEnable = description.EnableDepthClip;
		rasterizerDescription.ScissorEnable = description.EnableScissor;
		rasterizerDescription.MultisampleEnable = description.EnableMultisample;
		rasterizerDescription.AntialiasedLineEnable = description.EnableAntialiasedLine;

		ENGINE_HR_MESSAGE(rhi->GetDevice()->CreateRasterizerState(&rasterizerDescription, &RasterizerState), "Rasterizer State could not be created.");
	}

	void CRasterizerState::Release()
	{
		RasterizerState->Release();
	}

	void CRasterizerState::SetRasterizerState() const
	{
		Context->RSSetState(RasterizerState);
	}

	void CRasterizerState::ResetRasterizerState(const CRHI* rhi)
	{
		rhi->GetContext()->RSSetState(nullptr);
	}
}
