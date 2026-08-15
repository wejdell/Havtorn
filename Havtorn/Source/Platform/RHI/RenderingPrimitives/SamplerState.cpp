// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SamplerState.h"
#include "RHI/RHI.h"

namespace Havtorn
{
	CSamplerState::CSamplerState(const CRHI* rhi, const SSamplerDescription& description)
		: Context(rhi->GetContext())
	{
		D3D11_SAMPLER_DESC samplerDescription = {};
		samplerDescription.Filter = static_cast<D3D11_FILTER>(description.Filter);
		samplerDescription.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(description.AddressU);
		samplerDescription.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(description.AddressV);
		samplerDescription.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(description.AddressW);
		samplerDescription.MipLODBias = description.MipLODBias;
		samplerDescription.MaxAnisotropy = description.MaxAnisotropy;
		samplerDescription.ComparisonFunc = static_cast<D3D11_COMPARISON_FUNC>(description.ComparisonFunction);
		samplerDescription.BorderColor[0] = description.BorderColor[0];
		samplerDescription.BorderColor[1] = description.BorderColor[1];
		samplerDescription.BorderColor[2] = description.BorderColor[2];
		samplerDescription.BorderColor[3] = description.BorderColor[3];
		samplerDescription.MinLOD = description.MinLOD;
		samplerDescription.MaxLOD = description.MaxLOD;

		ENGINE_HR_MESSAGE(rhi->GetDevice()->CreateSamplerState(&samplerDescription, &SamplerState), "Sampler State could not be created.");
	}

	void CSamplerState::Release()
	{
		SamplerState->Release();
	}

	void CSamplerState::SetSamplerState(const U8 slot) const
	{
		Context->PSSetSamplers(slot, 1, &SamplerState);
	}

	void CSamplerState::ResetSamplerState(const CRHI* rhi)
	{
		rhi->GetContext()->RSSetState(nullptr);
	}
}
