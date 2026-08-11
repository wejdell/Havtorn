// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Shader.h"
#include "RHI/RHI.h"

#include <fstream>

namespace Havtorn
{
	CShader::CShader(const std::string& filepath, const CRHI* rhi, const EShaderType type)
		: Context(rhi->GetContext())
		, ShaderType(type)
	{
		std::ifstream fileData;
		fileData.open(filepath, std::ios::binary);
		const std::string data = { std::istreambuf_iterator<char>(fileData), std::istreambuf_iterator<char>() };
		CompiledData = data;
		
		switch (ShaderType)
		{
		case EShaderType::Vertex:
			ENGINE_HR_MESSAGE(rhi->GetDevice()->CreateVertexShader(CompiledData.data(), CompiledData.size(), nullptr, &VertexShader), "Vertex Shader: %s could not be created.", filepath.c_str());
			break;
		case EShaderType::Compute:
			ENGINE_HR_MESSAGE(rhi->GetDevice()->CreateComputeShader(CompiledData.data(), CompiledData.size(), nullptr, &ComputeShader), "Compute Shader: %s could not be created.", filepath.c_str());
			break;
		case EShaderType::Geometry:
			ENGINE_HR_MESSAGE(rhi->GetDevice()->CreateGeometryShader(CompiledData.data(), CompiledData.size(), nullptr, &GeometryShader), "Geometry Shader: %s could not be created.", filepath.c_str());
			break;
		case EShaderType::Pixel:
			ENGINE_HR_MESSAGE(rhi->GetDevice()->CreatePixelShader(CompiledData.data(), CompiledData.size(), nullptr, &PixelShader), "Pixel Shader: %s could not be created.", filepath.c_str());
			break;
		default:
			HV_LOG_ERROR("CShader constructor failed: shader type '%s' is not handled!", magic_enum::enum_name(ShaderType).data());
			break;
		}

		fileData.close();
	}

	void CShader::Release()
	{
		switch (ShaderType)
		{
		case EShaderType::Vertex:
			VertexShader->Release();
			break;
		case EShaderType::Compute:
			ComputeShader->Release();
			break;
		case EShaderType::Geometry:
			GeometryShader->Release();
			break;
		case EShaderType::Pixel:
			PixelShader->Release();
			break;
		default:
			break;
		}
	}

	std::string CShader::GetCompiledData() const
	{
		return CompiledData;
	}

	void CShader::SetShader() const
	{
		switch (ShaderType)
		{
		case EShaderType::Vertex:
			Context->VSSetShader(VertexShader, nullptr, 0);
			break;
		case EShaderType::Compute:
			Context->CSSetShader(ComputeShader, nullptr, 0);
			break;
		case EShaderType::Geometry:
			Context->GSSetShader(GeometryShader, nullptr, 0);
			break;
		case EShaderType::Pixel:
			Context->PSSetShader(PixelShader, nullptr, 0);
			break;
		default:
			break;
		}
	}

	void CShader::ResetShader(const CRHI* rhi, const EShaderType type)
	{
		switch (type)
		{
		case EShaderType::Vertex:
			rhi->GetContext()->VSSetShader(nullptr, nullptr, 0);
			break;
		case EShaderType::Compute:
			rhi->GetContext()->CSSetShader(nullptr, nullptr, 0);
			break;
		case EShaderType::Geometry:
			rhi->GetContext()->GSSetShader(nullptr, nullptr, 0);
			break;
		case EShaderType::Pixel:
			rhi->GetContext()->PSSetShader(nullptr, nullptr, 0);
			break;
		default:
			break;
		}
	}
}
