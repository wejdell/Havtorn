// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "FullscreenTextureFactory.h"
#include "GraphicsFramework.h"
#include "GraphicsUtilities.h"

#include "RenderingPrimitives/FullscreenTexture.h"
#include "RenderingPrimitives/GBuffer.h"

namespace Havtorn
{
	bool CFullscreenTextureFactory::Init(CGraphicsFramework* framework)
	{
		Framework = framework;
		return true;
	}

	CRenderTexture CFullscreenTextureFactory::CreateTexture(SVector2<U16> size, DXGI_FORMAT format, bool cpuAccess)
	{
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = size.X;
		textureDesc.Height = size.Y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = cpuAccess ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = cpuAccess ? 0 : D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = cpuAccess ? D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE : 0;
		textureDesc.MiscFlags = 0;

		ID3D11Texture2D* texture;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &texture), "Could not create Fullscreen Texture2D");

		CRenderTexture returnTexture;
		returnTexture = CreateTexture(texture, cpuAccess);

		if (!cpuAccess)
		{
			ID3D11ShaderResourceView* shaderResource;
			ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateShaderResourceView(texture, nullptr, &shaderResource), "Could not create Fullscreen Shader Resource View.");
			returnTexture.ShaderResource = shaderResource;
		}

		returnTexture.CPUAccess = cpuAccess;
		return returnTexture;
	}

	CRenderTexture CFullscreenTextureFactory::CreateTexture(ID3D11Texture2D* texture, bool cpuAccess)
	{
		D3D11_VIEWPORT* viewport = nullptr;
		if (texture)
		{
			D3D11_TEXTURE2D_DESC textureDescription;
			texture->GetDesc(&textureDescription);
			viewport = new D3D11_VIEWPORT({ 0.0f, 0.0f, STATIC_F32(textureDescription.Width), STATIC_F32(textureDescription.Height), 0.0f, 1.0f });
		}

		CRenderTexture returnTexture;
		returnTexture.Context = Framework->GetContext();
		returnTexture.Texture = texture;

		if (!cpuAccess)
		{
			ID3D11RenderTargetView* renderTarget;
			ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateRenderTargetView(texture, nullptr, &renderTarget), "Could not create Fullcreen Render Target View.");
			returnTexture.RenderTarget = renderTarget;
		}

		returnTexture.Viewport = viewport;

		return returnTexture;
	}

	CRenderTexture CFullscreenTextureFactory::CreateTexture(SVector2<U16> size, DXGI_FORMAT format, const std::string& filePath)
	{
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = size.X;
		textureDesc.Height = size.Y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		ID3D11Texture2D* texture;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &texture), "Could not create Fullscreen Texture2D");

		CRenderTexture returnTexture;
		returnTexture = CreateTexture(texture);

		ID3D11ShaderResourceView* shaderResource = UGraphicsUtils::GetShaderResourceView(Framework->GetDevice(), filePath);

		returnTexture.ShaderResource = shaderResource;
		return returnTexture;
	}

	CRenderTexture CFullscreenTextureFactory::CreateDepth(SVector2<U16> size, DXGI_FORMAT format)
	{
		DXGI_FORMAT stencilViewFormat = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT shaderResourceViewFormat = DXGI_FORMAT_UNKNOWN;

		switch (format)
		{
		case DXGI_FORMAT_R24G8_TYPELESS:
			stencilViewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			shaderResourceViewFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;
		case DXGI_FORMAT_R32_TYPELESS:
			stencilViewFormat = DXGI_FORMAT_D32_FLOAT;
			shaderResourceViewFormat = DXGI_FORMAT_R32_FLOAT;
			break;
		default:
			break;
		}

		D3D11_TEXTURE2D_DESC depthStencilDesc = { 0 };
		depthStencilDesc.Width = size.X;
		depthStencilDesc.Height = size.Y;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = format;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		depthStencilViewDesc.Format = stencilViewFormat;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		depthStencilViewDesc.Flags = 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format = shaderResourceViewFormat;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		ID3D11Texture2D* depthStencilBuffer;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer), "Texture could not be created.");
		ID3D11DepthStencilView* depthStencilView;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView), "Depth could not be created.");
		ID3D11ShaderResourceView* shaderResource;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateShaderResourceView(depthStencilBuffer, &shaderResourceViewDesc, &shaderResource), "Depth Shader Resource could not be created.");

		D3D11_VIEWPORT* viewport = new D3D11_VIEWPORT({ 0.0f, 0.0f, STATIC_F32(size.X), STATIC_F32(size.Y), 0.0f, 1.0f });

		CRenderTexture returnDepth;
		returnDepth.IsRenderTexture = false;
		returnDepth.Context = Framework->GetContext();
		returnDepth.Texture = depthStencilBuffer;
		returnDepth.Depth = depthStencilView;
		returnDepth.ShaderResource = shaderResource;
		returnDepth.Viewport = viewport;
		return returnDepth;
	}

	CGBuffer CFullscreenTextureFactory::CreateGBuffer(SVector2<U16> size)
	{
		std::array<DXGI_FORMAT, static_cast<size_t>(CGBuffer::EGBufferTextures::Count)> textureFormats =
		{
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_R16G16B16A16_SNORM,
			DXGI_FORMAT_R16G16B16A16_SNORM,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_R32G32_UINT,
		};

		//Creating textures, rendertargets, shaderresources and a viewport
		std::array<ID3D11Texture2D*, static_cast<size_t>(CGBuffer::EGBufferTextures::Count)> textures = {};
		std::array<ID3D11RenderTargetView*, static_cast<size_t>(CGBuffer::EGBufferTextures::Count)> renderTargets = {};
		std::array<ID3D11ShaderResourceView*, static_cast<size_t>(CGBuffer::EGBufferTextures::Count)> shaderResources = {};

		for (UINT i = 0; i < static_cast<size_t>(CGBuffer::EGBufferTextures::Count); ++i)
		{
			CRenderTexture texture = CreateTexture(size, textureFormats[i]);
			textures[i] = texture.Texture;
			renderTargets[i] = texture.RenderTarget;
			shaderResources[i] = texture.ShaderResource;
		}
		D3D11_VIEWPORT* viewport = new D3D11_VIEWPORT({ 0.0f, 0.0f, STATIC_F32(size.X), STATIC_F32(size.Y), 0.0f, 1.0f });

		CGBuffer returnGBuffer;
		returnGBuffer.Context = Framework->GetContext();
		returnGBuffer.Textures = std::move(textures);
		returnGBuffer.RenderTargets = std::move(renderTargets);
		returnGBuffer.ShaderResources = std::move(shaderResources);
		returnGBuffer.Viewport = viewport;
		return returnGBuffer;
	}
}
