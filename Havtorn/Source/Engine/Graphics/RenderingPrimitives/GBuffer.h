// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class CRenderTexture;

	class CGBuffer
	{
	public:
		friend class CRenderTextureFactory;

	public:
		enum class EGBufferTextures
		{
			Albedo,
			Normal,
			VertexNormal,
			Material,
			EditorData,
			Count
		};

	public:
		CGBuffer() = default;
		~CGBuffer();

		void ClearTextures(SVector4 clearColor = (0.0f, 0.0f, 0.0f, 0.0f));
		void ReleaseRenderTargets();
		void SetAsActiveTarget(CRenderTexture* depth = nullptr, bool isUsingEditor = false);
		void SetAsPSResourceOnSlot(EGBufferTextures resource, U16 slot);
		void SetAllAsResources(U16 startSlot);

		ID3D11RenderTargetView* GetEditorDataRenderTarget() const;
		ID3D11Texture2D* GetEditorDataTexture() const;
		const D3D11_VIEWPORT& GetViewport() const;

		void ReleaseResources();

	private:
		ID3D11DeviceContext* Context = nullptr;
		std::array<ID3D11Texture2D*, static_cast<U64>(EGBufferTextures::Count)> Textures;
		std::array<ID3D11RenderTargetView*, static_cast<U64>(EGBufferTextures::Count)> RenderTargets;
		std::array<ID3D11ShaderResourceView*, static_cast<U64>(EGBufferTextures::Count)> ShaderResources;
		D3D11_VIEWPORT Viewport = {};
	};
}
