// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct D3D11_VIEWPORT;
class CFullscreenTextureFactory;

namespace Havtorn
{
	class CFullscreenTexture
	{
	public:
		CFullscreenTexture() = default;
		~CFullscreenTexture() = default;
		void ClearTexture(SVector4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f });
		void ClearDepth(F32 clearDepth = 1.0f, U32 clearStencil = 0);
		void SetAsActiveTarget(CFullscreenTexture* depth = nullptr);
		void SetAsDepthTarget();
		void SetAsDepthTarget(CFullscreenTexture* intermediateRenderTarget);
		void SetAsResourceOnSlot(U16 slot);
		void ReleaseTexture();
		void ReleaseDepth();
		
		HAVTORN_API ID3D11Texture2D* const GetTexture() const;
		HAVTORN_API ID3D11ShaderResourceView* const GetShaderResourceView() const;

		friend CFullscreenTextureFactory;
		friend class CGBuffer;

	private:
		ID3D11DeviceContext* Context = nullptr;
		ID3D11Texture2D* Texture = nullptr;

		union {
			ID3D11RenderTargetView* RenderTarget = nullptr;
			ID3D11DepthStencilView* Depth;
		};

		ID3D11ShaderResourceView* ShaderResource = nullptr;
		D3D11_VIEWPORT* Viewport = nullptr;
	};
}
