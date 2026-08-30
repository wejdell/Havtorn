// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "Graphics/GraphicsEnums.h"

#include <HavtornString.h>

#include <RHI/RenderingPrimitives/DataBuffer.h>
#include <RHI/RenderingPrimitives/RenderTexture.h>
#include <RHI/RenderingPrimitives/SamplerState.h>
#include <RHI/RHIEnums.h>

namespace Havtorn
{
	class CRenderStateManager;
	constexpr U8 RenderDebugNameMaxSize = 30;

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
		CHavtornStaticString<RenderDebugNameMaxSize> Name;
		bool IsImported = false;
		EShaderType PipelineStage = EShaderType::Vertex;
		ERenderResourceType Type = ERenderResourceType::Unknown;
		ERenderResourceFormat Format = ERenderResourceFormat::Unknown;
		ESamplerType SamplerType = ESamplerType::Border;
		EResourceCPUAccess CPUAccess = EResourceCPUAccess::None;
		SVector2<U16> Size = SVector2<U16>::Zero;
		U32 ID = 0;
		U16 Stride = 0;
		U8 BindSlot = 0;
		U8 Offset = 0;
	};

	struct SRenderResourceDeclaration
	{
		SRenderResourceDescription Description;
		void* ExternalMemory = nullptr;
	};

	struct SRenderResourceHandle
	{
		SRenderResourceHandle(const CHavtornStaticString<RenderDebugNameMaxSize>& name, const U32 index);
		SRenderResourceHandle() = delete;

		U32 ID = 0;
		U32 RegistryIndex = 0;
		U16 RefCount = 0;
	};

	struct SRenderResource
	{
		SRenderResource(const SRenderResourceDescription& description, void* externalMemory);
		SRenderResource() = delete;

		void Allocate();
		void Deallocate();
		void Bind(CRenderStateManager* renderStateManager);

		union
		{
			CDataBuffer* DataBuffer = nullptr;
			CRenderTexture* RenderTexture;
			CSamplerState* SamplerState;
		};
		CRenderTexture* DepthTexture = nullptr;

		SRenderResourceDescription Description;
		U32 ID = 0;
	};

	constexpr U8 ResourceMaxLifetime = 20;

	class CRenderResourceRegistry
	{
	public:
		CRenderResourceRegistry() = default;
		~CRenderResourceRegistry() = default;

		SRenderResourceHandle DeclareResource(const SRenderResourceDeclaration& declaration);
		void TouchResources(const std::vector<SRenderResourceHandle>& handles);
		void Allocate();
		void Bind(const std::vector<SRenderResourceHandle>& handles, CRenderStateManager* renderStateManager);
		void Deallocate();

	private:
		std::vector<SRenderResource> Resources;
		std::vector<U8> ResourceLifetimes;
	};
}
