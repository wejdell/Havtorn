// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "Graphics/GraphicsEnums.h"

#include <HavtornString.h>

#include <RHI/RenderingPrimitives/DataBuffer.h>
#include <RHI/RenderingPrimitives/RenderTexture.h>
#include <RHI/RenderingPrimitives/SamplerState.h>
#include <RHI/RHIEnums.h>

#include <mutex>

namespace Havtorn
{
	class CRenderStateManager;
	struct SRenderView;
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
		ERenderResourceType Type = ERenderResourceType::Unknown;
		ERenderResourceFormat Format = ERenderResourceFormat::Unknown;
		ESamplerType SamplerType = ESamplerType::Wrap;
		EDataBufferUsage BufferUsage = EDataBufferUsage::Dynamic;
		EResourceCPUAccess CPUAccess = EResourceCPUAccess::None;
		union
		{
			SVector2<U16> Size2D = SVector2<U16>::Zero;
			U32 Size;
		};
		U32 ID = 0;
		U16 VertexStride = 0;
		bool IsAllocated = false;
		bool WasProcessed = false;
	};

	struct SRenderResourceDeclaration
	{
		SRenderResourceDescription Description;
		void* ExternalMemory = nullptr;
	};

	enum class ERenderTextureSizeHint : U8
	{
		FullResolution = 1,
		HalfResolution = 2,
		QuarterResolution = 4
	};

	struct ENGINE_API SRenderResourceHandle
	{
		SRenderResourceHandle(const CHavtornStaticString<RenderDebugNameMaxSize>& name, const U32 index);
		SRenderResourceHandle() = default;
		bool IsValid() const;

		U32 ID = 0;
		I32 RegistryIndex = -1;
		U16 RefCount = 0;
	};

	struct SRenderResource
	{
		SRenderResource(const SRenderResourceDescription& description, void* externalMemory);
		SRenderResource() = default;

		void Allocate(CRenderManager* manager, CRHI* rhi);
		void Deallocate(CRenderManager* manager, CRHI* rhi);
		void Clear(CRenderManager* manager, CRHI* rhi);

		SRenderResourceDescription Description;
		U32 ID = 0;
		
		union
		{
			CDataBuffer* DataBuffer = nullptr;
			CRenderTexture* RenderTexture;
			CSamplerState* SamplerState;
		};
	};

	constexpr U8 ResourceMaxLifetime = 20;

	class CRenderResourceRegistry
	{
		friend class CRenderManager;
	public:
		CRenderResourceRegistry() = default;
		~CRenderResourceRegistry() = default;

		ENGINE_API SRenderResourceHandle DeclareResource(SRenderResourceDeclaration declaration);

		template<typename T>
		SRenderResourceHandle DeclareConstantBuffer(const char* name, const U64 sizeMultiplier = 1);

		template<typename T>
		SRenderResourceHandle DeclareVertexBuffer(const char* name, const U16 maxInstances);

		template<typename T>
		SRenderResourceHandle DeclareVertexBuffer(const char* name, CDataBuffer* externalMemory);

		ENGINE_API SRenderResourceHandle DeclareIndexBuffer(const char* name, CDataBuffer* externalMemory);

		ENGINE_API SRenderResourceHandle DeclareRenderTexture(const char* name, const ERenderResourceFormat format, const SRenderView* renderView, const ERenderTextureSizeHint size = ERenderTextureSizeHint::FullResolution);
		ENGINE_API SRenderResourceHandle DeclareRenderTexture(const char* name, const ERenderResourceFormat format, const SVector2<U16>& size);
		ENGINE_API SRenderResourceHandle DeclareRenderTexture(const char* name, CRenderTexture* externalMemory);
		ENGINE_API SRenderResourceHandle DeclareRenderTexture(const char* name, CStaticRenderTexture* externalMemory);

		ENGINE_API SRenderResourceHandle DeclareRenderDepth(const char* name, const ERenderResourceFormat format, const SRenderView* renderView, const ERenderTextureSizeHint size = ERenderTextureSizeHint::FullResolution);
		ENGINE_API SRenderResourceHandle DeclareRenderDepth(const char* name, const ERenderResourceFormat format, const SVector2<U16>& size);

		void TouchResources(const std::vector<SRenderResourceHandle>& handles);
		void Allocate();
		void Deallocate();
		void ClearResources();

		ENGINE_API SRenderResource* GetResource(const SRenderResourceHandle& handle);

	private:
		CRenderManager* RenderManager = nullptr;
		CRHI* RHI = nullptr;
		std::vector<SRenderResource> Resources;
		std::vector<U8> ResourceLifetimes;
	};

	template<typename T>
	inline SRenderResourceHandle CRenderResourceRegistry::DeclareConstantBuffer(const char* name, const U64 sizeMultiplier)
	{
		return	DeclareResource({
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::ConstantBufferView,
					.CPUAccess = EResourceCPUAccess::CPUAccessWrite,
					.Size = STATIC_U32(sizeof(T) * sizeMultiplier)
				},
				.ExternalMemory = nullptr
		});
	}

	template<typename T>
	SRenderResourceHandle CRenderResourceRegistry::DeclareVertexBuffer(const char* name, const U16 maxInstances)
	{
		return	DeclareResource({
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::VertexBufferView,
					.CPUAccess = EResourceCPUAccess::CPUAccessWrite,
					.Size = sizeof(T) * maxInstances,
					.VertexStride = sizeof(T)
				},
				.ExternalMemory = nullptr
		});
	}

	template<typename T>
	SRenderResourceHandle CRenderResourceRegistry::DeclareVertexBuffer(const char* name, CDataBuffer* externalMemory)
	{
		return	DeclareResource({
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::VertexBufferView, // Not stricly needed for any external resources, should we just skip everything but the names? May want to show some info in debug
					.BufferUsage = EDataBufferUsage::Immutable,
					.VertexStride = sizeof(T)
				},
				.ExternalMemory = externalMemory 
		});
	}
}
