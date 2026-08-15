// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "BlendState.h"
#include "RHI/RHI.h"

#include <fstream>

namespace Havtorn
{
	CBlendState::CBlendState(const CRHI* rhi, const SBlendStateDescription& description)
		: Context(rhi->GetContext())
	{
		constexpr U8 maxSimultaneousRenderTargets = 8;
		D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{};
		blendDesc.AlphaToCoverageEnable = description.EnableAlphaToCoverage;
		blendDesc.IndependentBlendEnable = description.EnableIndependentBlend;
		
		for (U8 i = 0; i < maxSimultaneousRenderTargets; i++)
		{
			const SRenderTargetBlendDescription& targetDescription = description.RenderTargetDescriptions[i];
			blendDesc.RenderTarget[i].BlendEnable = targetDescription.EnableBlend;
			blendDesc.RenderTarget[i].SrcBlend = static_cast<D3D11_BLEND>(targetDescription.SourceBlend);
			blendDesc.RenderTarget[i].DestBlend = static_cast<D3D11_BLEND>(targetDescription.DestinationBlend);
			blendDesc.RenderTarget[i].BlendOp = static_cast<D3D11_BLEND_OP>(targetDescription.BlendOperation);
			blendDesc.RenderTarget[i].SrcBlendAlpha = static_cast<D3D11_BLEND>(targetDescription.SourceAlphaBlend);
			blendDesc.RenderTarget[i].DestBlendAlpha = static_cast<D3D11_BLEND>(targetDescription.DestinationAlphaBlend);
			blendDesc.RenderTarget[i].BlendOpAlpha = static_cast<D3D11_BLEND_OP>(targetDescription.AlphaBlendOperation);
			blendDesc.RenderTarget[i].RenderTargetWriteMask = targetDescription.RenderTargetWriteMask;
		}
		
		ENGINE_HR_MESSAGE(rhi->GetDevice()->CreateBlendState(&blendDesc, &BlendState), "Blend State could not be created.");
	}

	void CBlendState::Release()
	{
		BlendState->Release();
	}

	void CBlendState::SetBlendState() const
	{
		std::array<F32, 4> blendFactors = { 0.5f, 0.5f, 0.5f, 0.5f };
		Context->OMSetBlendState(BlendState, blendFactors.data(), 0xFFFFFFFFu);
	}

	void CBlendState::ResetBlendState(const CRHI* rhi)
	{
		std::array<F32, 4> blendFactors = { 0.5f, 0.5f, 0.5f, 0.5f };
		rhi->GetContext()->OMSetBlendState(nullptr, blendFactors.data(), 0xFFFFFFFFu);
	}
}
