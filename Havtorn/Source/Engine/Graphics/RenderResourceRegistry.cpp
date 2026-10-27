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
				RenderTexture = new CRenderTexture(*reinterpret_cast<CStaticRenderTexture*>(externalMemory));
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

	bool SRenderResource::IsTextureType() const
	{
		switch (Description.Type)
		{
		case ERenderResourceType::UnorderedAccessView: [[fallthrough]];
		case ERenderResourceType::DepthStencilView: [[fallthrough]];
		case ERenderResourceType::ShaderResourceView: [[fallthrough]];
		case ERenderResourceType::RenderTargetView:
			return true;
		case ERenderResourceType::ConstantBufferView: [[fallthrough]];
		case ERenderResourceType::IndexBufferView: [[fallthrough]];
		case ERenderResourceType::StreamOutputView: [[fallthrough]];
		case ERenderResourceType::VertexBufferView: [[fallthrough]];
		case ERenderResourceType::SamplerState:
		default:
			return false;
		}
	}

	void SRenderResource::Allocate(CRenderManager* manager, CRHI* rhi)
	{
		if (Description.IsImported || Description.IsAllocated)
			return;

		switch (Description.Type)
		{
		case ERenderResourceType::ConstantBufferView:
			DataBuffer = new CDataBuffer();
			DataBuffer->CreateBuffer(Description.Name.AsString(), rhi, Description.Size);
			break;
		case ERenderResourceType::ShaderResourceView:
			//StaticRenderTexture = manager->;
			break;
		case ERenderResourceType::UnorderedAccessView:
			break;
		case ERenderResourceType::DepthStencilView:
			RenderTexture = new CRenderTexture(manager->GetRenderTextureFactory().CreateDepth(Description.Size2D, static_cast<DXGI_FORMAT>(Description.Format)));
			break;
		case ERenderResourceType::RenderTargetView:
			RenderTexture = new CRenderTexture(manager->GetRenderTextureFactory().CreateTexture(Description.Size2D, static_cast<DXGI_FORMAT>(Description.Format), Description.CPUAccess != EResourceCPUAccess::None));
			break;
		case ERenderResourceType::StreamOutputView:
			break;
		case ERenderResourceType::IndexBufferView:
			//DataBuffer->CreateBuffer(Description.Name, ;
			break;
		case ERenderResourceType::VertexBufferView:
			DataBuffer = new CDataBuffer();
			DataBuffer->CreateBuffer(Description.Name.AsString(), rhi, Description.VertexStride, Description.Size / Description.VertexStride, nullptr, EDataBufferType::Vertex);
			break;
		case ERenderResourceType::SamplerState:
			//SamplerState = reinterpret_cast<CSamplerState*>(externalMemory);
			break;
		default:
			break;
		}

		Description.IsAllocated = true;
	}

	void SRenderResource::Deallocate(CRenderManager* /*manager*/, CRHI* /*rhi*/)
	{
		if (Description.IsImported || !Description.IsAllocated)
			return;

		switch (Description.Type)
		{
		case ERenderResourceType::ConstantBufferView:
			DataBuffer->Release();
			delete DataBuffer;
			break;
		case ERenderResourceType::ShaderResourceView:
			//RenderTexture->Release();
			break;
		case ERenderResourceType::UnorderedAccessView:
			break;
		case ERenderResourceType::DepthStencilView:
			RenderTexture->Release();
			delete RenderTexture;
			break;
		case ERenderResourceType::RenderTargetView:
			RenderTexture->Release();
			delete RenderTexture;
			break;
		case ERenderResourceType::StreamOutputView:
			break;
		case ERenderResourceType::IndexBufferView:
			//DataBuffer->Release();
			break;
		case ERenderResourceType::VertexBufferView:
			DataBuffer->Release();
			delete DataBuffer;
			break;
		case ERenderResourceType::SamplerState:
			//SamplerState->Release();
			break;
		default:
			break;
		}

		Description.IsAllocated = false;
	}

	void SRenderResource::Clear(CRenderManager* /*manager*/, CRHI* /*rhi*/)
	{
		if (Description.IsImported || !Description.IsAllocated)
			return;

		switch (Description.Type)
		{
		case ERenderResourceType::ConstantBufferView:
			break;
		case ERenderResourceType::ShaderResourceView:
			RenderTexture->ClearTexture();
			break;
		case ERenderResourceType::UnorderedAccessView:
			break;
		case ERenderResourceType::DepthStencilView:
			RenderTexture->ClearDepth();
			break;
		case ERenderResourceType::RenderTargetView:
			RenderTexture->ClearTexture();
			break;
		case ERenderResourceType::StreamOutputView:
			break;
		case ERenderResourceType::IndexBufferView:
			break;
		case ERenderResourceType::VertexBufferView:
			break;
		case ERenderResourceType::SamplerState:
			break;
		default:
			break;
		}
	}

	SRenderResourceHandle CRenderResourceRegistry::DeclareResource(SRenderResourceDeclaration declaration)
	{
		declaration.Description.ID = UGeneralUtils::HashString(declaration.Description.Name.AsString());

		// TODO.NW: Add memoization with map to index?
		if (auto iterator = std::ranges::find(Resources, declaration.Description.ID, &SRenderResource::ID); iterator != Resources.end())
		{
			return SRenderResourceHandle(declaration.Description.Name, STATIC_U32(std::distance(Resources.begin(), iterator)));
		}

		const U32 index = STATIC_U32(Resources.size());
		Resources.emplace_back(declaration.Description, declaration.ExternalMemory);
		ResourceLifetimes.emplace_back(STATIC_U8(0u));
		if (declaration.ExternalMemory != nullptr)
			ResourceLifetimes.back() = ResourceMaxLifetime;

		return SRenderResourceHandle(declaration.Description.Name, index);
	}

	void CRenderResourceRegistry::RemoveRenderViewRenderTarget(SRenderView* renderView)
	{
		// TODO.NW: Want to make sure render views can share resources, might make sense to introduce resolution to all texture names? 
		// And make sure render view specific resources can be cleared out when they are unrequested

		// We might want to keep all resources in the registry also if we can, at least all render view ones. The render target and editor textures
		// may not need to be held by the view, as long as we can guarantee the lifetime of it in the registry.

		// It might make sense to keep textures as shared_ptrs everywhere as well

		I64 eraseIndex = -1;
		for (I64 i = 0; i < STATIC_I64(Resources.size()); i++)
		{
			if (Resources[i].RenderTexture == &renderView->RenderTarget)
			{
				eraseIndex = i;
				break;
			}
		}

		if (eraseIndex != -1)
			Resources.erase(Resources.begin() + eraseIndex);
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
			Resources[index].Description.WasProcessed = true;

			if (ResourceLifetimes[index] == 0)
				continue;

			if (Resources[index].Description.IsImported)
				continue;
				
			Resources[index].Allocate(RenderManager, RHI);
		}
	}

	void CRenderResourceRegistry::Deallocate(const ERenderResourceDeallocationStrategy deallocationStrategy)
	{
		// TODO.NW: Figure out how to clear imported resources, might be nice to be able to clear draw data on a render target every frame, 
		// but can we optimize the removal of the imported assets then? It actually doesn't matter much since they are not allocated but owned
		// by the asset registry currently, could try just removing imported resources when draw data is cleared. But then we might end up
		// with the syncing issue again causing crashes
		std::vector<U64> deadResourceIndices;
		for (U64 index = 0; index < Resources.size(); index++)
		{
			SRenderResource& renderResource = Resources[index];
			const bool forceClear = deallocationStrategy == ERenderResourceDeallocationStrategy::ForceClear || (deallocationStrategy == ERenderResourceDeallocationStrategy::ForceClearTexturesOnly && renderResource.IsTextureType());

			if (renderResource.Description.IsImported)
				continue;

			if (!renderResource.Description.WasProcessed && !forceClear)
				continue;

			if (ResourceLifetimes[index] > 0 && !forceClear)
			{
				ResourceLifetimes[index]--;
				continue;
			}

			deadResourceIndices.emplace_back(index);			
			renderResource.Deallocate(RenderManager, RHI);
		}

		// Descending order
		std::sort(deadResourceIndices.rbegin(), deadResourceIndices.rend());

		for (U64 index : deadResourceIndices)
		{
			Resources[index] = std::move(Resources.back());
			Resources.pop_back();
		}

		ResourceLifetimes.resize(Resources.size(), 0);
	}

	void CRenderResourceRegistry::ClearResources()
	{
		for (U64 index = 0; index < Resources.size(); index++)
		{
			if (!Resources[index].Description.IsAllocated)
				continue;

			Resources[index].Clear(RenderManager, RHI);
		}
	}

	SRenderResource* CRenderResourceRegistry::GetResource(const SRenderResourceHandle& handle)
	{
		if (!UMath::IsWithin(handle.RegistryIndex, 0, STATIC_I32(Resources.size())))
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

	SRenderResourceHandle CRenderResourceRegistry::DeclareRenderTexture(const char* name, const ERenderResourceFormat format, const SRenderView* renderView, const ERenderTextureSizeHint size)
	{
		// TODO.NW: Use size to add to name? otherwise differently size views get the same resources
		const SVector2<U16> fullResolution = renderView != nullptr ? renderView->ViewSize : RenderManager->GetCurrentWindowResolution();
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
		HV_ASSERT(externalMemory != nullptr, "DeclareRenderTexture: Provided memory was nullptr! If you are declaring a texture owned by the render graph, use DeclareRenderTexture instead!");

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

	SRenderResourceHandle CRenderResourceRegistry::DeclareRenderDepth(const char* name, const ERenderResourceFormat format, const SRenderView* renderView, const ERenderTextureSizeHint size)
	{
		const SVector2<U16> fullResolution = renderView != nullptr ? renderView->ViewSize : RenderManager->GetCurrentWindowResolution();
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
