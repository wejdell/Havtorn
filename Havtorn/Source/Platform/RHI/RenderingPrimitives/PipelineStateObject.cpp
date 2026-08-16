// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "PipelineStateObject.h"

#include "RHI/RHI.h"
#include "RHI/RHIEnums.h"
#include "Shader.h"
#include "VertexInputLayout.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "RootSignature.h"

namespace Havtorn
{
	U64 SPSODescription::Hash() const
	{
		U64 hash = 0;
		if (VertexShader != nullptr)
			hash += reinterpret_cast<uintptr_t>(VertexShader) >> 3;

		if (PixelShader != nullptr)
			hash += reinterpret_cast<uintptr_t>(PixelShader) >> 3;

		if (GeometryShader != nullptr)
			hash += reinterpret_cast<uintptr_t>(GeometryShader) >> 3;

		if (ComputeShader != nullptr)
			hash += reinterpret_cast<uintptr_t>(ComputeShader) >> 3;

		if (InputLayout != nullptr)
			hash += reinterpret_cast<uintptr_t>(InputLayout) >> 3;

		if (BlendState != nullptr)
			hash += reinterpret_cast<uintptr_t>(BlendState) >> 3;

		if (RasterizerState != nullptr)
			hash += reinterpret_cast<uintptr_t>(RasterizerState) >> 3;

		if (DepthStencilState != nullptr)
			hash += reinterpret_cast<uintptr_t>(DepthStencilState) >> 3;

		if (RootSignature != nullptr)
			hash += reinterpret_cast<uintptr_t>(RootSignature) >> 3;

		return hash;
	}

	CPipelineStateObject::CPipelineStateObject(CRHI* rhi, const SPSODescription& description)
		: RHI(rhi)
		, Description(description)
	{
		Hash = description.Hash();
	}

	U64 CPipelineStateObject::TrySetPipelineState(const U64 currentHash)
	{
		if (Hash == currentHash)
			return currentHash;

		if (Description.VertexShader != nullptr)
			Description.VertexShader->SetShader();
		else
			CShader::ResetShader(RHI, EShaderType::Vertex);

		if (Description.PixelShader != nullptr)
			Description.PixelShader->SetShader();
		else
			CShader::ResetShader(RHI, EShaderType::Pixel);

		if (Description.GeometryShader != nullptr)
			Description.GeometryShader->SetShader();
		else
			CShader::ResetShader(RHI, EShaderType::Geometry);

		// NW: We aren't using compute shaders yet so no need to set these right now

		if (Description.InputLayout != nullptr)
			Description.InputLayout->SetInputLayout();
		else
			CVertexInputLayout::ResetInputLayout(RHI);

		if (Description.BlendState != nullptr)
			Description.BlendState->SetBlendState();
		else
			CBlendState::ResetBlendState(RHI);

		if (Description.RasterizerState != nullptr)
			Description.RasterizerState->SetRasterizerState();
		else
			CRasterizerState::ResetRasterizerState(RHI);

		U32 stencilRef = 0;
		if (Description.DepthStencilState != nullptr)
			Description.DepthStencilState->SetDepthStencilState(stencilRef);
		else
			CDepthStencilState::ResetDepthStencilState(RHI);

		// NW: We aren't using the root signature object yet so no need to check it right now

		return Hash;
	}
}
