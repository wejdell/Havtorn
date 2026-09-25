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
		// void CreateBuffer(const std::string& bufferName, const CRHI* rhi, U32 byteWidth, const void* subResourceData = nullptr, EDataBufferType bufferType = EDataBufferType::Constant, EDataBufferUsage usage = EDataBufferUsage::Dynamic, EResourceCPUAccess cpuAccess = EResourceCPUAccess::CPUAccessWrite);
		// CRenderTexture CreateTexture(SVector2<U16> size, DXGI_FORMAT format, bool cpuAccess = false);

		CHavtornStaticString<RenderDebugNameMaxSize> Name;
		bool IsImported = false;
		//EShaderType PipelineStage = EShaderType::Vertex;
		ERenderResourceType Type = ERenderResourceType::Unknown;
		ERenderResourceFormat Format = ERenderResourceFormat::Unknown;
		ESamplerType SamplerType = ESamplerType::Border;
		EDataBufferUsage BufferUsage = EDataBufferUsage::Dynamic;
		EResourceCPUAccess CPUAccess = EResourceCPUAccess::None;
		union
		{
			SVector2<U16> Size2D = SVector2<U16>::Zero;
			U32 Size;
		};
		U32 ID = 0;
		U16 Stride = 0;
		//U8 BindSlot = 0;
		U8 Offset = 0;
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

	// NW: Maybe put in short namespace?
	template<typename T>
	static SRenderResourceDeclaration DeclareConstantBuffer(const char* name, const U64 sizeMultiplier = 1);
	
	template<typename T>
	static SRenderResourceDeclaration DeclareVertexBuffer(const char* name, const U16 maxInstances);
	
	template<typename T>
	static SRenderResourceDeclaration DeclareVertexBuffer(const char* name, CDataBuffer* externalMemory);

	static SRenderResourceDeclaration DeclareIndexBuffer(const char* name, CDataBuffer* externalMemory);

	static SRenderResourceDeclaration DeclareRenderTexture(const char* name, const ERenderResourceFormat format, const ERenderTextureSizeHint size = ERenderTextureSizeHint::FullResolution);
	static SRenderResourceDeclaration DeclareRenderTexture(const char* name, const ERenderResourceFormat format, const SVector2<U16>& size);
	static SRenderResourceDeclaration DeclareRenderTexture(const char* name, CStaticRenderTexture* externalMemory);

	static SRenderResourceDeclaration DeclareRenderDepth(const char* name, const ERenderResourceFormat format, const ERenderTextureSizeHint size = ERenderTextureSizeHint::FullResolution);
	static SRenderResourceDeclaration DeclareRenderDepth(const char* name, const ERenderResourceFormat format, const SVector2<U16>& size);

	struct SRenderResourceHandle
	{
		SRenderResourceHandle(const CHavtornStaticString<RenderDebugNameMaxSize>& name, const U32 index);
		SRenderResourceHandle() = default;

		U32 ID = 0;
		I32 RegistryIndex = -1;
		U16 RefCount = 0;
	};

	struct SRenderResource
	{
		SRenderResource(const SRenderResourceDescription& description, void* externalMemory);
		SRenderResource() = default;

		void Allocate();
		void Deallocate();
		void Bind(CRenderStateManager* renderStateManager);

		union
		{
			CDataBuffer* DataBuffer = nullptr;
			CRenderTexture* RenderTexture;
			CStaticRenderTexture* StaticRenderTexture;
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

		SRenderResource* GetResource(const SRenderResourceHandle& handle);

	private:
		std::vector<SRenderResource> Resources;
		std::vector<U8> ResourceLifetimes;
	};

	template<typename T>
	inline SRenderResourceDeclaration DeclareConstantBuffer(const char* name, const U64 sizeMultiplier)
	{
		return	{
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::ConstantBufferView,
					.CPUAccess = EResourceCPUAccess::CPUAccessWrite,
					.Size = sizeof(T) * sizeMultiplier
				},
				.ExternalMemory = externalMemory 
				};
	}

	template<typename T>
	SRenderResourceDeclaration DeclareVertexBuffer(const char* name, const U16 maxInstances)
	{
		return	{
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::VertexBufferView,
					.CPUAccess = EResourceCPUAccess::CPUAccessWrite,
					.Size = sizeof(T) * maxInstances,
					.Stride = sizeof(T)
				},
				.ExternalMemory = nullptr 
				};
	}

	template<typename T>
	SRenderResourceDeclaration DeclareVertexBuffer(const char* name, CDataBuffer* externalMemory)
	{
		return	{
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::VertexBufferView, // Not stricly needed for any external resources, should we just skip everything but the names? May want to show some info in debug
					.BufferUsage = EDataBufferUsage::Immutable,
					.Stride = sizeof(T)
				},
				.ExternalMemory = externalMemory 
				};
	}
}
