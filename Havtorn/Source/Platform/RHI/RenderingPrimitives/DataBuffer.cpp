// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "DataBuffer.h"
#include "RHI/RHI.h"

#include <EngineException.h>

namespace Havtorn
{
	const CDataBuffer CDataBuffer::Null = CDataBuffer();

	void CDataBuffer::Release()
	{
		Buffer->Release();
	}

	void CDataBuffer::CreateBuffer(const std::string& bufferName, const CRHI* rhi, const U32 stride, const U32 instanceCount, const void* subResourceData, EDataBufferType bufferType, EDataBufferUsage usage, EResourceCPUAccess cpuAccess)
	{
		Name = bufferName;
		Context = rhi->GetContext();
		VertexBufferStride = stride;

		D3D11_BUFFER_DESC bufferDescription = CD3D11_BUFFER_DESC{};
		bufferDescription.Usage = static_cast<D3D11_USAGE>(usage);
		bufferDescription.BindFlags = static_cast<D3D11_BIND_FLAG>(bufferType);
		bufferDescription.CPUAccessFlags = static_cast<D3D11_CPU_ACCESS_FLAG>(cpuAccess);
		bufferDescription.ByteWidth = stride * instanceCount;

		if (subResourceData != nullptr)
		{
			D3D11_SUBRESOURCE_DATA subResource = {};
			subResource.pSysMem = subResourceData;
			ENGINE_HR_MESSAGE(rhi->GetDevice()->CreateBuffer(&bufferDescription, &subResource, &Buffer), "%s could not be created.", Name.c_str());
		}
		else
		{
			ENGINE_HR_MESSAGE(rhi->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &Buffer), "%s could not be created.", Name.c_str());
		}
	}

	U32 CDataBuffer::GetStride() const
	{
		return VertexBufferStride;
	}

	U32 CDataBuffer::GetOffset() const
	{
		return VertexBufferOffset;
	}

	CDataBuffer::CDataBuffer(const std::string& name, ID3D11DeviceContext* context, ID3D11Buffer* buffer)
		: Name(name)
		, Context(context)
		, Buffer(buffer)
	{}
}
