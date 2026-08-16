// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RootSignature.h"
#include "RHI/RHI.h"

#include "RHI/RenderingPrimitives/DataBuffer.h"
#include "RHI/RenderingPrimitives/RenderTexture.h"
#include "RHI/RenderingPrimitives/SamplerState.h"

namespace Havtorn
{
	CRootSignature::CRootSignature(const CRHI* /*rhi*/, const SRootSignatureDescription& /*description*/)
		//: Context(rhi->GetContext())
	{
		//Buffers = description.Buffers;
		//Resources = description.Textures;
		//Samplers = description.Samplers;
	}
	
	//void CRootSignature::SetRootSignature()
	//{
	//	//Samplers[STATIC_U8(sampler)]->SetSamplerState(slot);
	//	//Context->PSSetConstantBuffers(slot, 1, &buffer.Buffer);
	//	//Context->PSSetShaderResources(startSlot, numberOfResources, resources);

	//	if (IndexBuffer->Buffer == nullptr)
	//		Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	//	else
	//		Context->IASetIndexBuffer(IndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	//	std::vector<ID3D11Buffer*> bufferPointers;
	//	for (const CDataBuffer* buffer : VertexBuffers)
	//		bufferPointers.emplace_back(buffer->Buffer);

	//	Context->IASetVertexBuffers(VertexBufferStartSlot, bufferPointers.size(), bufferPointers.data(), VertexStrides.data(), VertexOffsets.data());


	//}

	//void CRootSignature::SetIndexBuffer() const
	//{
	//	if (IndexBuffer->Buffer == nullptr)
	//	{
	//		Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	//		return;
	//	}

	//	Context->IASetIndexBuffer(IndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);
	//}

	//void CRootSignature::SetVertexBuffers(const U8 startSlot, const U8 startIndex, const U8 endIndex) const
	//{
	//	std::vector<ID3D11Buffer*> bufferPointers;
	//	for (const CDataBuffer* buffer : VertexBuffers)
	//		bufferPointers.emplace_back(buffer->Buffer);

	//	Context->IASetVertexBuffers(startSlot, bufferPointers.size(), bufferPointers.data(), VertexStrides.data(), VertexOffsets.data());
	//}

	//void CRootSignature::SetConstantBuffers(const EShaderType pipelineStage, const U8 startSlot, const U8 startIndex, const U8 endIndex) const
	//{
	//	switch (pipelineStage)
	//	{
	//	case EShaderType::Vertex:
	//		break;
	//	case EShaderType::Geometry:
	//		break;
	//	case EShaderType::Compute:
	//		break;
	//	case EShaderType::Pixel:
	//		break;
	//	default:
	//		break;
	//	}
	//}

	//void CRootSignature::SetResources(const EShaderType pipelineStage, const U8 startSlot, const U8 startIndex, const U8 endIndex) const
	//{
	//
	//}

	//void CRootSignature::SetSamplers(const EShaderType pipelineStage, const U8 startSlot, const U8 startIndex, const U8 endIndex) const
	//{
	//
	//}
}
