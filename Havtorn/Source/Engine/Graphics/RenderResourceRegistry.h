// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "Graphics/GraphicsEnums.h"

#include <HavtornString.h>

#include <RHI/RenderingPrimitives/DataBuffer.h>
#include <RHI/RenderingPrimitives/RenderTexture.h>
#include <RHI/RenderingPrimitives/SamplerState.h>

namespace Havtorn
{
	struct SRenderResourceHandle
	{
		SRenderResourceHandle(const CHavtornStaticString<32>& name);
		SRenderResourceHandle() = delete;

		CHavtornStaticString<32> Name;
		U16 RefCount = 0;
		U32 ID = 0;
		U32 RegistryIndex = 0;
	};

	enum class ERenderResourceType : U8
	{
		Unknown,
		ConstantBufferView,
		ShaderResourceView,
		UnorderedAccessView,
		DepthStencilView,
		RenderTargetView,
		StreamOutputView,
		IndexBufferView,
		VertexBufferView,
		SamplerState
	};

	struct SRenderResourceDescription
	{
		CHavtornStaticString<32> Name;
		ERenderResourceType Type = ERenderResourceType::Unknown;
		ERenderResourceFormat Format = ERenderResourceFormat::Unknown;
		ESamplerType SamplerType = ESamplerType::Border;
		EResourceCPUAccess CPUAccess = EResourceCPUAccess::None;
		SVector2<U16> Size = SVector2<U16>::Zero;
	};

	struct SRenderResource
	{
		SRenderResource(const SRenderResourceDescription& description);
		SRenderResource() = delete;

		void Construct();
		void Bind();

		union
		{
			CDataBuffer* DataBuffer = nullptr;
			CRenderTexture* RenderTexture;
			CSamplerState* SamplerState;
		};

		SRenderResourceDescription Description;
	};

	class CRenderResourceRegistry
	{
	public:
		CRenderResourceRegistry() = default;
		~CRenderResourceRegistry() = default;

		void Allocate();
		void Bind(std::vector<const SRenderResourceHandle>& handles);

	private:
		std::vector<SRenderResource> Resources;
	};
}
