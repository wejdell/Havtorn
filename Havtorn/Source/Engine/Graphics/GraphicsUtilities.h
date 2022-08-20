// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <DirectXTex/DirectXTex.h>
#include <fstream>
#include "GraphicsFramework.h"
#include "GraphicsEnums.h"
#include "Engine.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileHeaderDeclarations.h"
#include "Core/EngineException.h"

#define ASSETPATH(path) std::string(path).c_str()

namespace Havtorn
{
	namespace UGraphicsUtils
	{
		HAVTORN_API bool CreateVertexShader(const std::string& filepath, const CGraphicsFramework* framework, ID3D11VertexShader** outVertexShader, std::string& outShaderData);

		HAVTORN_API bool CreatePixelShader(const std::string& filepath, const CGraphicsFramework* framework, ID3D11PixelShader** outPixelShader);

		HAVTORN_API bool CreateGeometryShader(const std::string& filepath, const CGraphicsFramework* framework, ID3D11GeometryShader** outGeometryShader);
	
		HAVTORN_API HRESULT CreateShaderResourceView(ID3D11Device* device, const std::string& filePath, ID3D11ShaderResourceView** outShaderResourceView);

		HAVTORN_API HRESULT CreateShaderResourceViewFromResource(ID3D11Device* device, const std::string& filePath, ID3D11ShaderResourceView** outShaderResourceView);

		HAVTORN_API HRESULT CreateShaderResourceView(ID3D11Device* device, const void* data, U64 size, ETextureFormat format, ID3D11ShaderResourceView** outShaderResourceView);

		HAVTORN_API ID3D11ShaderResourceView* TryGetShaderResourceView(ID3D11Device* device, const std::string& texturePath);

		HAVTORN_API ID3D11ShaderResourceView* TryGetShaderResourceView(ID3D11Device* device, const void* data, U64 size, ETextureFormat format);

		HAVTORN_API ID3D11ShaderResourceView* GetShaderResourceView(ID3D11Device* device, const std::string& texturePath);
	};
}