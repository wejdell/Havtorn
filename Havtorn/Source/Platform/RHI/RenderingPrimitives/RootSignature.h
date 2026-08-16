// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>

#include <d3d11.h>

namespace Havtorn
{
	class CRHI;
	class CDataBuffer;
	class CRenderTexture;
	class CSamplerState;

	struct SRootSignatureDescription
	{
		std::vector<CDataBuffer*> Buffers;
		U8 BufferSlots;
		std::vector<CRenderTexture*> Resources;
		std::vector<U8> ResourceSlots;
		std::vector<CSamplerState*> Samplers;
		std::vector<U8> SamplerSlots;
	};

	class CRootSignature
	{
	public:
		CRootSignature(const CRHI* rhi, const SRootSignatureDescription& description);
		CRootSignature() = delete;
		~CRootSignature() = default;

		// NW: The root signature is a description for the PSO of what to expect, in D3D12 compiled to a binary blob.
		// We can offer this function as a way to set the actual data in the "command list" until we know how to represent this.
		//PLATFORM_API void SetRootSignature();

		//PLATFORM_API void SetIndexBuffer() const;
		//PLATFORM_API void SetVertexBuffers(const U8 startSlot, const U8 startIndex, const U8 endIndex) const;
		//PLATFORM_API void SetConstantBuffers(const EShaderType pipelineStage, const U8 startSlot, const U8 startIndex, const U8 endIndex) const;
		//PLATFORM_API void SetResources(const EShaderType pipelineStage, const U8 startSlot, const U8 startIndex, const U8 endIndex) const;
		//PLATFORM_API void SetSamplers(const EShaderType pipelineStage, const U8 startSlot, const U8 startIndex, const U8 endIndex) const;

	private:
		//ID3D11DeviceContext* Context = nullptr;

		//CDataBuffer* IndexBuffer = nullptr;

		//std::vector<CDataBuffer*> VertexBuffers;
		//std::vector<U32> VertexStrides;
		//std::vector<U32> VertexOffsets;
		//U8 VertexBufferStartSlot = 0;

		//std::vector<CDataBuffer*> ConstantBuffers;
		//U8 ConstantBufferStartSlot = 0;
		//
		//// NW: We will keep render targets in the Pipeline State Object for now, these resources are only input textures
		//std::vector<CRenderTexture*> Resources;
		//U8 ResourceStartSlot = 0;
		//
		//std::vector<CSamplerState*> Samplers;
		//U8 SamplerStartSlot = 0;
	};
}
