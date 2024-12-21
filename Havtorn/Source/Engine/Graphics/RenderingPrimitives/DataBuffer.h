// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class CGraphicsFramework;

	enum class EDataBufferType : U8
	{
		None,
		Vertex = BIT(0),
		Index = BIT(1),
		Constant = BIT(2),
	};

	enum class EDataBufferUsage : U8
	{
		Default,
		Immutable,
		Dynamic
	};

	enum class EDataBufferCPUAccess : U32
	{
		None,
		CPUAccessWrite = BIT(16),
		CPUAccessRead = BIT(17),
	};

	class CDataBuffer
	{
	public:
		const static CDataBuffer Null;
		friend class CRenderStateManager;
		friend class CRenderManager;

		CDataBuffer() = default;
		~CDataBuffer() = default;

		void CreateBuffer(const std::string& bufferName, const CGraphicsFramework* framework, U32 byteWidth, const void* subResourceData = nullptr, EDataBufferType bufferType = EDataBufferType::Constant, EDataBufferUsage usage = EDataBufferUsage::Dynamic, EDataBufferCPUAccess cpuAccess = EDataBufferCPUAccess::CPUAccessWrite);

		template<class T>
		void BindBuffer(const T& bufferData)
		{
			D3D11_MAPPED_SUBRESOURCE localBufferData;
			ZeroMemory(&localBufferData, sizeof(D3D11_MAPPED_SUBRESOURCE));
			const std::string errorMessage = Name + " could not be bound.";
			ENGINE_HR_MESSAGE(Context->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &localBufferData), errorMessage.c_str());

			memcpy(localBufferData.pData, &bufferData, sizeof(T));
			Context->Unmap(Buffer, 0);
		}

		template<class T>
		void BindBuffer(const std::vector<T>& bufferData)
		{
			D3D11_MAPPED_SUBRESOURCE localBufferData;
			ZeroMemory(&localBufferData, sizeof(D3D11_MAPPED_SUBRESOURCE));
			const std::string errorMessage = Name + " could not be bound.";
			ENGINE_HR_MESSAGE(Context->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &localBufferData), errorMessage.c_str());

			memcpy(localBufferData.pData, &bufferData[0], sizeof(T) * bufferData.size());
			Context->Unmap(Buffer, 0);
		}

	private:
		CDataBuffer(const std::string& name, ID3D11DeviceContext* context, ID3D11Buffer* buffer);

		std::string Name;
		ID3D11DeviceContext* Context = nullptr;
		ID3D11Buffer* Buffer = nullptr;
	};
}