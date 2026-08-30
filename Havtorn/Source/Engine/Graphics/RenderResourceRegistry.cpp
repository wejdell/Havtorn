// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderResourceRegistry.h"
#include "RenderStateManager.h"

#include <GeneralUtilities.h>

namespace Havtorn
{
	SRenderResourceHandle::SRenderResourceHandle(const CHavtornStaticString<RenderDebugNameMaxSize>& name, const U32 index)
		: ID(UGeneralUtils::HashString(name.AsString()))
		, RegistryIndex(index)
	{
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
				RenderTexture = reinterpret_cast<CRenderTexture*>(externalMemory);
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

	void SRenderResource::Bind(CRenderStateManager* renderStateManager)
	{
		switch (Description.Type)
		{
		case ERenderResourceType::ConstantBufferView:
			switch (Description.PipelineStage)
			{
			case EShaderType::Vertex:
				renderStateManager->VSSetConstantBuffer(Description.BindSlot, *DataBuffer);
				break;
			case EShaderType::Pixel:
				renderStateManager->PSSetConstantBuffer(Description.BindSlot, *DataBuffer);
				break;
			case EShaderType::Geometry:
				renderStateManager->GSSetConstantBuffer(Description.BindSlot, *DataBuffer);
				break;
			default:
				break;
			}
			break;
		case ERenderResourceType::ShaderResourceView:
			switch (Description.PipelineStage)
			{
			case EShaderType::Vertex:
				RenderTexture->SetAsVSResourceOnSlot(Description.BindSlot);
				break;
			case EShaderType::Pixel:
				RenderTexture->SetAsPSResourceOnSlot(Description.BindSlot);
				break;
			default:
				break;
			}
			break;
		case ERenderResourceType::UnorderedAccessView:
			break;
		case ERenderResourceType::DepthStencilView:
			break;
		case ERenderResourceType::RenderTargetView:
			RenderTexture->SetAsActiveTarget(DepthTexture);
			break;
		case ERenderResourceType::StreamOutputView:
			break;
		case ERenderResourceType::IndexBufferView:
			renderStateManager->IASetIndexBuffer(*DataBuffer);
			break;
		case ERenderResourceType::VertexBufferView:
			renderStateManager->IASetVertexBuffer(Description.BindSlot, *DataBuffer, Description.Stride, Description.Offset);
			break;
		case ERenderResourceType::SamplerState:
			SamplerState->SetSamplerState(Description.BindSlot);	
			break;
		default:
			break;
		}
	}

	SRenderResourceHandle CRenderResourceRegistry::DeclareResource(const SRenderResourceDeclaration& declaration)
	{
		if (auto iterator = std::ranges::find(Resources, declaration.Description.ID, &SRenderResource::ID); iterator != Resources.end())
		{
			return SRenderResourceHandle(declaration.Description.Name, STATIC_U32(std::distance(Resources.begin(), iterator)));
		}

		U32 index = STATIC_U32(Resources.size());
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

	void CRenderResourceRegistry::Bind(const std::vector<SRenderResourceHandle>& handles, CRenderStateManager* renderStateManager)
	{
		for (const SRenderResourceHandle& handle : handles)
		{
			Resources[handle.RegistryIndex].Bind(renderStateManager);
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
}
