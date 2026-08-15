// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>

#include <d3d11.h>

namespace Havtorn
{
	/*enum class EShaderType
	{
		Vertex,
		Compute,
		Geometry,
		Pixel
	};

	class CRHI;

	class CShader
	{
	public:
		PLATFORM_API CShader(const std::string& filepath, const CRHI* rhi, const EShaderType type);
		CShader() = delete;
		~CShader() = default;

		PLATFORM_API void Release();
		PLATFORM_API std::string GetCompiledData() const;
		PLATFORM_API void SetShader() const;
		PLATFORM_API static void ResetShader(const CRHI* rhi, const EShaderType type);

	private:
		ID3D11DeviceContext* Context = nullptr;

		union 
		{
			ID3D11VertexShader* VertexShader = nullptr;
			ID3D11ComputeShader* ComputeShader;
			ID3D11GeometryShader* GeometryShader;
			ID3D11PixelShader* PixelShader;
		};

		EShaderType ShaderType = EShaderType::Vertex;
		std::string CompiledData = "";
	};*/
}
