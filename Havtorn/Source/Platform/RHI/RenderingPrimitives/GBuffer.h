// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>

#include <d3d11.h>

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
			WorldPosition,
			EditorData,
			Count
		};

	public:
		CGBuffer() = default;
		PLATFORM_API ~CGBuffer();

		PLATFORM_API void ClearTextures(SVector4 clearColor = (0.0f, 0.0f, 0.0f, 0.0f), const bool includingEditorData = false);
		PLATFORM_API void ReleaseRenderTargets();
		PLATFORM_API void SetAsActiveTarget(CRenderTexture* depth = nullptr, bool isUsingEditor = false);
		PLATFORM_API void SetAsPSResourceOnSlot(EGBufferTextures resource, U16 slot);
		PLATFORM_API void SetAllAsResources(U16 startSlot);

		PLATFORM_API ID3D11RenderTargetView* GetEditorDataRenderTarget() const;
		PLATFORM_API ID3D11Texture2D* GetEditorDataTexture() const;
		PLATFORM_API ID3D11RenderTargetView* GetEditorWorldPositionRenderTarget() const;
		PLATFORM_API ID3D11Texture2D* GetEditorWorldPositionTexture() const;
		PLATFORM_API const D3D11_VIEWPORT& GetViewport() const;

		PLATFORM_API void ReleaseResources();

	private:
		ID3D11DeviceContext* Context = nullptr;
		std::array<ID3D11Texture2D*, static_cast<U64>(EGBufferTextures::Count)> Textures;
		std::array<ID3D11RenderTargetView*, static_cast<U64>(EGBufferTextures::Count)> RenderTargets;
		std::array<ID3D11ShaderResourceView*, static_cast<U64>(EGBufferTextures::Count)> ShaderResources;
		D3D11_VIEWPORT Viewport = {};
	};
}
