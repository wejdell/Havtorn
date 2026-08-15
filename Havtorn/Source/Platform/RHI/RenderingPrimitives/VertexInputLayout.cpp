// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "VertexInputLayout.h"
#include "RHI/RHI.h"

namespace Havtorn
{
	CVertexInputLayout::CVertexInputLayout(CRHI* rhi, const SInputLayoutDescription& description, const std::string& shaderByteCode)
		: Context(rhi->GetContext())
	{		
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout;

		for (const SInputElementDescription& element : description.Layout)
		{
			D3D11_INPUT_ELEMENT_DESC& nativeElement = layout.emplace_back();
			nativeElement = D3D11_INPUT_ELEMENT_DESC
			{	.SemanticName = element.SemanticName, 
				.SemanticIndex = STATIC_U32(element.SemanticIndex), 
				.Format = static_cast<DXGI_FORMAT>(element.Format), 
				.InputSlot = STATIC_U32(element.InputSlot),
				.AlignedByteOffset = element.AlignedByteOffset,
				.InputSlotClass = static_cast<D3D11_INPUT_CLASSIFICATION>(element.InputClassification),
				.InstanceDataStepRate = STATIC_U32(element.InstanceDataStepRate)
			};
		}
	
		ENGINE_HR_MESSAGE(rhi->GetDevice()->CreateInputLayout(layout.data(), STATIC_U32(layout.size()), shaderByteCode.data(), shaderByteCode.size(), &InputLayout), "Input Layout could not be created.");
	}

	void CVertexInputLayout::Release()
	{
		InputLayout->Release();
	}

	void CVertexInputLayout::SetInputLayout() const
	{
		Context->IASetInputLayout(InputLayout);
	}

	void CVertexInputLayout::ResetInputLayout(const CRHI* rhi)
	{
		rhi->GetContext()->IASetInputLayout(nullptr);
	}
}
