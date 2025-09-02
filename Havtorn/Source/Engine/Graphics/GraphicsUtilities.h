// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <DirectXTex/DirectXTex.h>
#include <fstream>
#include "GraphicsFramework.h"
#include "GraphicsEnums.h"
#include "Engine.h"
#include "FileSystem/FileHeaderDeclarations.h"
#include <EngineException.h>
#include <FileSystem.h>

#define ASSETPATH(path) std::string(path).c_str()

namespace Havtorn
{
	namespace UGraphicsUtils
	{
		ENGINE_API bool CreateVertexShader(const std::string& filepath, const CGraphicsFramework* framework, ID3D11VertexShader** outVertexShader, std::string& outShaderData);

		ENGINE_API bool CreatePixelShader(const std::string& filepath, const CGraphicsFramework* framework, ID3D11PixelShader** outPixelShader);

		ENGINE_API bool CreateGeometryShader(const std::string& filepath, const CGraphicsFramework* framework, ID3D11GeometryShader** outGeometryShader);
	
		ENGINE_API HRESULT CreateShaderResourceView(ID3D11Device* device, const std::string& filePath, ID3D11ShaderResourceView** outShaderResourceView);

		ENGINE_API HRESULT CreateShaderResourceViewFromSource(ID3D11Device* device, const std::string& filePath, ID3D11ShaderResourceView** outShaderResourceView);

		ENGINE_API HRESULT CreateShaderResourceView(ID3D11Device* device, const void* data, U64 size, ETextureFormat format, ID3D11ShaderResourceView** outShaderResourceView);

		ENGINE_API ID3D11ShaderResourceView* TryGetShaderResourceView(ID3D11Device* device, const std::string& texturePath);

		ENGINE_API ID3D11ShaderResourceView* TryGetShaderResourceView(ID3D11Device* device, const void* data, U64 size, ETextureFormat format);

		ENGINE_API ID3D11ShaderResourceView* GetShaderResourceView(ID3D11Device* device, const std::string& texturePath);
	};
}
