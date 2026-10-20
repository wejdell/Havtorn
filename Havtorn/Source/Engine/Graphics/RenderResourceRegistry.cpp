// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderResourceRegistry.h"
#include "RenderStateManager.h"
#include "RenderManager.h"
#include "Engine.h"

#include <GeneralUtilities.h>

namespace Havtorn
{
	SRenderResourceHandle::SRenderResourceHandle(const CHavtornStaticString<RenderDebugNameMaxSize>& name, const U32 index)
		: ID(UGeneralUtils::HashString(name.AsString()))
		, RegistryIndex(index)
	{
	}

	bool SRenderResourceHandle::IsValid() const
	{
		return RegistryIndex != -1;
	}

	SRenderResource::SRenderResource(const SRenderResourceDescription& description, void* externalMemory)
		: Description(description)
		, ID(description.ID)
	{
		if (externalMemory != nullptr)
		{
			switch (Description.Type)
			{
			case ERenderResourceType::ConstantBufferView:
				DataBuffer = reinterpret_cast<CDataBuffer*>(externalMemory);
				break;
			case ERenderResourceType::ShaderResourceView:
				StaticRenderTexture = reinterpret_cast<CStaticRenderTexture*>(externalMemory);
				break;
			case ERenderResourceType::UnorderedAccessView:
				break;
			case ERenderResourceType::DepthStencilView:
				break;
			case ERenderResourceType::RenderTargetView:
				RenderTexture = reinterpret_cast<CRenderTexture*>(externalMemory);
				break;
			case ERenderResourceType::StreamOutputView:
				break;
			case ERenderResourceType::IndexBufferView:
				DataBuffer = reinterpret_cast<CDataBuffer*>(externalMemory);
				break;
			case ERenderResourceType::VertexBufferView:
				DataBuffer = reinterpret_cast<CDataBuffer*>(externalMemory);
				break;
			case ERenderResourceType::SamplerState:
				SamplerState = reinterpret_cast<CSamplerState*>(externalMemory);
				break;
			default:
				break;
			}

			Description.IsImported = true;
		}
	}

	void SRenderResource::Allocate()
	{
	}

	void SRenderResource::Deallocate()
	{
		if (Description.IsImported)
			return;

		switch (Description.Type)
		{
		case ERenderResourceType::ConstantBufferView:
			DataBuffer->Release();
			break;
		case ERenderResourceType::ShaderResourceView:
			RenderTexture->Release();
			break;
		case ERenderResourceType::UnorderedAccessView:
			break;
		case ERenderResourceType::DepthStencilView:
			break;
		case ERenderResourceType::RenderTargetView:
			RenderTexture->Release();
			break;
		case ERenderResourceType::StreamOutputView:
			break;
		case ERenderResourceType::IndexBufferView:
			DataBuffer->Release();
			break;
		case ERenderResourceType::VertexBufferView:
			DataBuffer->Release();
			break;
		case ERenderResourceType::SamplerState:
			SamplerState->Release();
			break;
		default:
			break;
		}
	}

	SRenderResourceHandle CRenderResourceRegistry::DeclareResource(const SRenderResourceDeclaration& declaration)
	{
		// TODO.NW: Add memoization with map to index?
		if (auto iterator = std::ranges::find(Resources, declaration.Description.ID, &SRenderResource::ID); iterator != Resources.end())
		{
			return SRenderResourceHandle(declaration.Description.Name, STATIC_U32(std::distance(Resources.begin(), iterator)));
		}

		const U32 index = STATIC_U32(Resources.size());
		Resources.emplace_back(declaration.Description, declaration.ExternalMemory);
		ResourceLifetimes.emplace_back(STATIC_U8(0u));

		return SRenderResourceHandle(declaration.Description.Name, index);
	}

	void CRenderResourceRegistry::TouchResources(const std::vector<SRenderResourceHandle>& handles)
	{
		for (const SRenderResourceHandle& handle : handles)
			ResourceLifetimes[handle.RegistryIndex] = ResourceMaxLifetime;
	}

	void CRenderResourceRegistry::Allocate()
	{
		for (U64 index = 0; index < Resources.size(); index++)
		{
			if (ResourceLifetimes[index] == 0)
				continue;

			if (Resources[index].Description.IsImported)
				continue;
				
			Resources[index].Allocate();
		}
	}

	void CRenderResourceRegistry::Deallocate()
	{
		//std::vector<U64> deadResourceIndices;
		//for (U64 index = 0; index < Resources.size(); index++)
		//{
		//	if (ResourceLifetimes[index] > 0)
		//	{
		//		ResourceLifetimes[index]--;
		//		continue;
		//	}

		//	deadResourceIndices.emplace_back(index);			
		//	Resources[index].Deallocate();
		//}

		//// Descending order
		//std::sort(deadResourceIndices.rbegin(), deadResourceIndices.rend());

		//for (U64 index : deadResourceIndices)
		//{
		//	Resources[index] = std::move(Resources.back());
		//	Resources.pop_back();
		//}

		//ResourceLifetimes.resize(Resources.size(), 0);

		// TODO.NW: Revisit this when we can deallocate on the render thread safely

		Resources.clear();
		ResourceLifetimes.clear();
	}

	SRenderResource* CRenderResourceRegistry::GetResource(const SRenderResourceHandle& handle)
	{
		if (!UMath::IsWithin(handle.RegistryIndex, 0, STATIC_I32(Resources.size() - 1)))
			return nullptr;
		
		return &Resources[handle.RegistryIndex];
	}

	SRenderResourceHandle CRenderResourceRegistry::DeclareIndexBuffer(const char* name, CDataBuffer* externalMemory)
	{
		return	DeclareResource({
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::IndexBufferView,
					.BufferUsage = EDataBufferUsage::Immutable
				},
				.ExternalMemory = externalMemory
		});
	}

	SRenderResourceHandle CRenderResourceRegistry::DeclareRenderTexture(const char* name, const ERenderResourceFormat format, const ERenderTextureSizeHint size)
	{
		SVector2<U16> fullResolution = GEngine::GetRenderManager()->GetCurrentWindowResolution(); // TODO.NW: Figure out where this should come from
		return DeclareRenderTexture(name, format, SVector2<U16>(fullResolution.X / STATIC_U8(size), fullResolution.Y / STATIC_U8(size)));
	}

	SRenderResourceHandle CRenderResourceRegistry::DeclareRenderTexture(const char* name, const ERenderResourceFormat format, const SVector2<U16>& size)
	{
		return	DeclareResource({
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::RenderTargetView,
					.Format = format,
					.Size2D = size
				}
		});
	}

	SRenderResourceHandle CRenderResourceRegistry::DeclareRenderTexture(const char* name, CRenderTexture* externalMemory)
	{
		HV_ASSERT(externalMemory != nullptr, "DeclareRenderTexture: Provided memory was nullptr! If you are declaring a texture owned by the render graph, use the overload taking format and size instead!");

		return	DeclareResource({
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::RenderTargetView,
					.Format = ERenderResourceFormat::R32G32B32A32_Float, // TODO.NW: Might want to store the format in our RHI textures, just need to provide it in the factory (needs a rewrite anyway)
					.Size2D = SVector2<U16>(512, 512) // TODO.NW: None of these fields are used anyway but it might be nice to store sizes for texture resources also. Could display that in the tooltip of the HVAs
				},
				.ExternalMemory = externalMemory
		});
	}

	SRenderResourceHandle CRenderResourceRegistry::DeclareRenderTexture(const char* name, CStaticRenderTexture* externalMemory)
	{
		HV_ASSERT(externalMemory != nullptr, "DeclareRenderTexture: Provided memory was nullptr! If you are declaring a texture owned by the render graph, use DeclareRenderTexture instead!");

		return	DeclareResource({
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::ShaderResourceView,
					.Format = ERenderResourceFormat::R32G32B32A32_Float, // TODO.NW: Might want to store the format in our RHI textures, just need to provide it in the factory (needs a rewrite anyway)
					.Size2D = SVector2<U16>(512, 512) // TODO.NW: None of these fields are used anyway but it might be nice to store sizes for texture resources also. Could display that in the tooltip of the HVAs
				},
				.ExternalMemory = externalMemory
		});
	}

	SRenderResourceHandle CRenderResourceRegistry::DeclareRenderDepth(const char* name, const ERenderResourceFormat format, const ERenderTextureSizeHint size)
	{
		SVector2<U16> fullResolution = GEngine::GetRenderManager()->GetCurrentWindowResolution(); // TODO.NW: Figure out where this should come from
		return DeclareRenderDepth(name, format, SVector2<U16>(fullResolution.X / STATIC_U8(size), fullResolution.Y / STATIC_U8(size)));
	}

	SRenderResourceHandle CRenderResourceRegistry::DeclareRenderDepth(const char* name, const ERenderResourceFormat format, const SVector2<U16>& size)
	{
		return	DeclareResource({
				.Description =
				{
					.Name = { name },
					.Type = ERenderResourceType::DepthStencilView,
					.Format = format,
					.Size2D = size
				}
		});
	}
}
