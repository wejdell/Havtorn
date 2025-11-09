// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct D3D11_VIEWPORT;

namespace Havtorn
{
	class CRenderTextureFactory;
	class CRenderTexture;
	class CGBuffer;

	class CStaticRenderTexture
	{
		friend CRenderTextureFactory;
		friend CRenderTexture;

	public:
		CStaticRenderTexture() = default;
		~CStaticRenderTexture() = default;
		void SetAsPSResourceOnSlot(U16 slot);
		void SetAsVSResourceOnSlot(U16 slot);
		void ReleaseTexture();
	
		// TODO.NW: This is silly. We should unify our render calls so they may all take a RenderTarget instead
		ENGINE_API intptr_t GetResource() const;
		ENGINE_API ID3D11ShaderResourceView* GetShaderResource() const;
		ENGINE_API ID3D11ShaderResourceView* const* GetShaderResourceView() const;

	private:
		ID3D11DeviceContext* Context = nullptr;
		ID3D11ShaderResourceView* ShaderResource = nullptr;
	};

	// TODO.NW: Look over this class again. It's good to have an abstraction but now it's functioning as both SRV and render target/depth.
	
	class CRenderTexture
	{
		friend CRenderTextureFactory;
		friend CGBuffer;
	
	public:
		CRenderTexture() = default;
		~CRenderTexture() = default;
		ENGINE_API CRenderTexture(const CStaticRenderTexture& staticTexture);
		void ClearTexture(SVector4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f });
		void ClearDepth(F32 clearDepth = 1.0f, U32 clearStencil = 0);
		void SetAsActiveTarget(CRenderTexture* depth = nullptr);
		void SetAsDepthTarget();
		void SetAsDepthTarget(CRenderTexture* intermediateRenderTarget);
		void SetAsPSResourceOnSlot(U16 slot);
		void SetAsVSResourceOnSlot(U16 slot);
		void* MapToCPUFromGPUTexture(ID3D11Texture2D* gpuTexture);
		void CopyFromTexture(ID3D11Texture2D* texture);
		void WriteToCPUTexture(void* data, U64 size);
		void UnmapFromCPU();
		void ReleaseTexture();
		void ReleaseDepth();
		
		ENGINE_API bool IsShaderResourceValid() const;
		ENGINE_API void Release();

		ENGINE_API ID3D11Texture2D* const GetTexture() const;
		ENGINE_API ID3D11ShaderResourceView* const GetShaderResourceView() const;
		ENGINE_API ID3D11ShaderResourceView* MoveShaderResourceView();
		ENGINE_API ID3D11RenderTargetView* const GetRenderTargetView() const;
		ENGINE_API ID3D11DepthStencilView* const GetDepthStencilView() const;
		ENGINE_API D3D11_VIEWPORT* const GetViewport();

	private:
		ID3D11DeviceContext* Context = nullptr;
		ID3D11Texture2D* Texture = nullptr;

		union 
		{
			ID3D11RenderTargetView* RenderTarget = nullptr;
			ID3D11DepthStencilView* Depth;
		};

		ID3D11ShaderResourceView* ShaderResource = nullptr;
		D3D11_VIEWPORT Viewport = {};

		// TODO.NW: Keep this now for debugging
		bool IsFromStaticTexture = false;
		bool IsRenderTexture = true;
		bool CPUAccess = true;
	};
}
