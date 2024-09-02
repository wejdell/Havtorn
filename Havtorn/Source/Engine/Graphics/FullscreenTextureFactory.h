// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

struct ID3D11Texture2D;

namespace Havtorn
{
	class CGraphicsFramework;
	class CFullscreenTexture;
	
	class CFullscreenTextureFactory 
	{
	public:
		CFullscreenTextureFactory() = default;
		~CFullscreenTextureFactory() = default;
		bool Init(CGraphicsFramework* framework);
		CFullscreenTexture CreateTexture(SVector2<U16> size, DXGI_FORMAT format);
		CFullscreenTexture CreateTexture(ID3D11Texture2D* texture);
		CFullscreenTexture CreateTexture(SVector2<U16> size, DXGI_FORMAT format, const std::string& filePath);
		CFullscreenTexture CreateDepth(SVector2<U16> size, DXGI_FORMAT format);
		class CGBuffer CreateGBuffer(SVector2<U16> size);

	private:
		CGraphicsFramework* Framework = nullptr;
	};
}
