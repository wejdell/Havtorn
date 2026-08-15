// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <array>
#include <queue>
#include <mutex>

#include "GraphicsEnums.h"

#include <d3d11.h>

#include <RHI/RHI.h>
#include <RHI/RHIEnums.h>
#include <RHI/RenderingPrimitives/BlendState.h>
#include <RHI/RenderingPrimitives/DataBuffer.h>
#include <RHI/RenderingPrimitives/DepthStencilState.h>
#include <RHI/RenderingPrimitives/RasterizerState.h>
#include <RHI/RenderingPrimitives/SamplerState.h>
#include <RHI/RenderingPrimitives/Shader.h>

namespace Havtorn
{
	class CRenderStateManager
	{
	public:
		enum class EBlendStates
		{
			Disable,
			AlphaBlend,
			AdditiveBlend,
			GBufferAlphaBlend,
			Count
		};

		enum class EDepthStencilStates
		{
			Default,
			OnlyRead,
			StencilWrite,
			StencilMask,
			DepthFirst,
			Count
		};

		enum class ERasterizerStates
		{
			Default,
			Wireframe,
			BackfaceCulling,
			FrontfaceCulling,
			NoFaceCulling,
			Count
		};

	public:
		friend class CRenderManager;
		friend class CAssetRegistry;

		CRenderStateManager() = default;
		~CRenderStateManager();

		bool Init(CRHI* rhi);

		// Init order 1:1 to EVertexShaders
		void InitVertexShadersAndInputLayouts();
		// Init order 1:1 to EPixelShaders
		void InitPixelShaders();
		// Init order 1:1 to EGeometryShaders
		void InitGeometryShaders();
		// Init order 1:1 to ESamplers
		void InitSamplers();
		// Init order 1:1 to EVertexBufferPrimitives.
		void InitVertexBuffers();
		// Init order 1:1 to EIndexBufferPrimitives
		void InitIndexBuffers();
		void InitMeshVertexStrides();
		void InitMeshVertexOffset();

	private:
		template<typename T>
		U16 AddVertexBuffer(const std::vector<T>& vertices);
		U16 AddIndexBuffer(const std::vector<U32>& indices);
		U16 AddMeshVertexStride(U32 stride);
		U16 AddMeshVertexOffset(U32 offset);

		std::string AddShader(const std::string& fileName, const U64 index, const EShaderType shaderType);
		void AddInputLayout(const std::string& vsData, EInputLayoutType layoutType);
		void AddSampler(ESamplerType samplerType);
		void AddViewport(SVector2<F32> topLeftCoordinate, SVector2<F32> widthAndHeight, SVector2<F32> depth);

	public:
		// IA
		void IASetTopology(ETopologies topology) const;
		void IASetInputLayout(EInputLayoutType layout) const;
		void IASetVertexBuffer(U8 startSlot, const CDataBuffer& buffer, U32 stride, U32 offset) const;
		void IASetVertexBuffers(U8 startSlot, U8 numberOfBuffers, const std::vector<CDataBuffer>& buffers, const U32* strides, const U32* offsets) const;
		void IASetIndexBuffer(const CDataBuffer& buffer) const;

		// VS
		void VSSetShader(EVertexShaders shader) const;
		void VSSetConstantBuffer(U8 slot, const CDataBuffer& buffer);
		void VSSetResources(U8 startSlot, U8 numberOfResources, ID3D11ShaderResourceView* const* resources);

		// GS
		void GSSetShader(EGeometryShaders shader) const;
		void GSSetConstantBuffer(U8 slot, const CDataBuffer& buffer) const;

		// PS
		void PSSetSampler(U8 slot, ESamplers sampler) const;
		void PSSetShader(EPixelShaders shader) const;
		void PSSetConstantBuffer(U8 slot, const CDataBuffer& buffer) const;
		void PSSetResources(U8 startSlot, U8 numberOfResources, ID3D11ShaderResourceView* const* resources);

		// RS
		void RSSetViewports(U8 numberOfViewports, const D3D11_VIEWPORT* viewports);
		void RSSetRasterizerState(ERasterizerStates rasterizerState) const;

		// OM
		void OMSetBlendState(EBlendStates blendstate) const;
		void OMSetDepthStencilState(EDepthStencilStates depthStencilState, U32 stencilRef = 0) const;
		void OMSetRenderTargets(U8 numberOfTargets, ID3D11RenderTargetView* const* targetViews, ID3D11DepthStencilView* depthStencilView) const;

		void Draw(U32 vertexCount, U32 startVertexLocation) const;
		void DrawIndexed(U32 indexCount, U32 startIndexLocation, U32 baseVertexLocation) const;
		void DrawInstanced(U32 vertexCountPerInstance, U32 numberOfInstances, U32 startVertexLocation, U32 startInstanceLocation) const;
		void DrawIndexedInstanced(U32 indexCountPerInstance, U32 instanceCount, U32 startIndexLocation, U32 baseVertexLocation, U32 startInstanceLocation) const;

		// TODO.NR: Rename these to fit the naming of the above
		void SetAllStates(EBlendStates blendState, EDepthStencilStates depthStencilState, ERasterizerStates rasterizerState) const;
		void SetAllDefault() const;

		void ClearState();

		void ClearShaderResources() const;

		void Release();

		void FlushShaderChanges();

	private:
		void OnShaderSourceChange(const std::string& filePath);
		const U64 OnShaderSourceChangeFunctionHandle = 200;

		bool CreateBlendStates(CRHI* rhi);
		bool CreateDepthStencilStates(CRHI* rhi);
		bool CreateRasterizerStates(CRHI* rhi);

	private:
		const std::string ShaderRoot = "Shaders/";

		CRHI* RHI = nullptr;
		ID3D11DeviceContext* Context = nullptr;
		std::array<CBlendState*, STATIC_U64(EBlendStates::Count)> BlendStates;
		std::array<CDepthStencilState*, STATIC_U64(EDepthStencilStates::Count)> DepthStencilStates;
		std::array<CRasterizerState*, STATIC_U64(ERasterizerStates::Count)> RasterizerStates;

		std::array<CShader*, STATIC_U64(EVertexShaders::Count)> VertexShaders;
		std::array<CShader*, STATIC_U64(EPixelShaders::Count)> PixelShaders;
		std::array<CShader*, STATIC_U64(EGeometryShaders::Count)> GeometryShaders;

		std::vector<CSamplerState*> Samplers;
		std::vector<CDataBuffer> VertexBuffers;
		std::vector<CDataBuffer> IndexBuffers;
		std::vector<ID3D11InputLayout*> InputLayouts;
		std::vector<D3D11_VIEWPORT> Viewports;
		std::vector<U32> MeshVertexStrides;
		std::vector<U32> MeshVertexOffsets;

		struct SShaderInitData
		{
			const std::string OutputFileName;
			const EShaderType ShaderType = EShaderType::Vertex;
			const U64 ShaderIndex = 0; 
		};
		std::map<std::string, SShaderInitData> ShaderInitData;
		std::queue<std::string> QueuedShaderRecompiles;
		std::mutex ShaderRecompileMutex;
	};

	template <typename T>
	U16 CRenderStateManager::AddVertexBuffer(const std::vector<T>& vertices)
	{
		VertexBuffers.emplace_back(CDataBuffer());
		VertexBuffers.back().CreateBuffer("Vertex Buffer", RHI, sizeof(T) * STATIC_U32(vertices.size()), vertices.data(), EDataBufferType::Vertex, EDataBufferUsage::Immutable, EDataBufferCPUAccess::None);

		return STATIC_U16(VertexBuffers.size() - 1);
	}
}
