// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>

#include "RHI/RHIEnums.h"

#include <d3d11.h>

namespace Havtorn
{
	class CRHI;

	struct SInputElementDescription
	{
		const char* SemanticName;
		ERenderResourceFormat Format = ERenderResourceFormat::Unknown;
		U8 SemanticIndex = 0;
		U8 InputSlot = 0;
		U8 InstanceDataStepRate = 0;
		ERenderInputClassification InputClassification = ERenderInputClassification::InputPerVertexData;
		U32 AlignedByteOffset = AppendAlignedElementCode;
	};

	struct SInputLayoutDescription
	{
		std::vector<SInputElementDescription> Layout;
	};

	class CVertexInputLayout
	{
	public:
		PLATFORM_API CVertexInputLayout(CRHI* rhi, const SInputLayoutDescription& description, const std::string& shaderByteCode);
		CVertexInputLayout() = delete;
		~CVertexInputLayout() = default;

		PLATFORM_API void Release();
		PLATFORM_API void SetInputLayout() const;
		PLATFORM_API static void ResetInputLayout(const CRHI* rhi);

	private:
		ID3D11DeviceContext* Context = nullptr;
		ID3D11InputLayout* InputLayout = nullptr;
	};
}
