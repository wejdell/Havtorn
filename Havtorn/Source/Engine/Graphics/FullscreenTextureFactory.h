// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

struct ID3D11Texture2D;

namespace Havtorn
{
	class CGraphicsFramework;
	class CRenderTexture;
	
	class CFullscreenTextureFactory 
	{
	public:
		CFullscreenTextureFactory() = default;
		~CFullscreenTextureFactory() = default;
		bool Init(CGraphicsFramework* framework);
		CRenderTexture CreateTexture(SVector2<U16> size, DXGI_FORMAT format, bool cpuAccess = false);
		CRenderTexture CreateTexture(ID3D11Texture2D* texture, bool cpuAccess = false);
		CRenderTexture CreateTexture(SVector2<U16> size, DXGI_FORMAT format, const std::string& filePath);
		CRenderTexture CreateDepth(SVector2<U16> size, DXGI_FORMAT format);
		class CGBuffer CreateGBuffer(SVector2<U16> size);

	private:
		CGraphicsFramework* Framework = nullptr;
	};
}
