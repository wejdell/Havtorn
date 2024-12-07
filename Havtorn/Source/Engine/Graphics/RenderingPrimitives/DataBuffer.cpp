// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "DataBuffer.h"

#include "Graphics/GraphicsFramework.h"

namespace Havtorn
{
	const CDataBuffer CDataBuffer::Null = { std::string("Null"), nullptr, nullptr };

	void CDataBuffer::CreateBuffer(const std::string& bufferName, const CGraphicsFramework* framework, U32 byteWidth, const void* subResourceData, EDataBufferType bufferType, EDataBufferUsage usage, EDataBufferCPUAccess cpuAccess)
	{
		Name = bufferName;
		Context = framework->GetContext();

		D3D11_BUFFER_DESC bufferDescription = CD3D11_BUFFER_DESC{};
		bufferDescription.Usage = static_cast<D3D11_USAGE>(usage);
		bufferDescription.BindFlags = static_cast<D3D11_BIND_FLAG>(bufferType);
		bufferDescription.CPUAccessFlags = static_cast<D3D11_CPU_ACCESS_FLAG>(cpuAccess);
		bufferDescription.ByteWidth = byteWidth;

		if (subResourceData != nullptr)
		{
			D3D11_SUBRESOURCE_DATA subResource = {};
			subResource.pSysMem = subResourceData;
			ENGINE_HR_MESSAGE(framework->GetDevice()->CreateBuffer(&bufferDescription, &subResource, &Buffer), "%s could not be created.", Name.c_str());
		}
		else
		{
			ENGINE_HR_MESSAGE(framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &Buffer), "%s could not be created.", Name.c_str());
		}
	}

	CDataBuffer::CDataBuffer(const std::string& name, ID3D11DeviceContext* context, ID3D11Buffer* buffer)
		: Name(name)
		, Context(context)
		, Buffer(buffer)
	{}
}
