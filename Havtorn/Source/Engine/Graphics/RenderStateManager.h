// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <array>

namespace Havtorn
{
	class CRenderStateManager
	{
	public:
		enum class EBlendStates
		{
			Disable,
			AlphaBlend,
			AdditiveBlend,
			GBufferAlphaBlend,
			Count
		};

		enum class EDepthStencilStates
		{
			Default,
			OnlyRead,
			StencilWrite,
			StencilMask,
			DepthFirst,
			Count
		};

		enum class ERasterizerStates
		{
			Default,//Uses backface culling.
			Wireframe,
			FrontFaceCulling,
			NoFaceCulling,
			Count
		};

		enum class ESamplerStates
		{
			Trilinear,
			Point,
			Wrap,
			Count
		};

	public:
		CRenderStateManager() = default;
		~CRenderStateManager();

		bool Init(class CGraphicsFramework* framework);

		void SetBlendState(EBlendStates blendstate);
		void SetDepthStencilState(EDepthStencilStates depthStencilState, U32 stencilRef = 0);
		void SetRasterizerState(ERasterizerStates rasterizerState);
		void SetSamplerState(ESamplerStates samplerState);
		void SetAllStates(EBlendStates blendState, EDepthStencilStates depthStencilState, ERasterizerStates rasterizerState, ESamplerStates samplerState);
		void SetAllDefault();

		void Release();

	private:
		bool CreateBlendStates(ID3D11Device* device);
		bool CreateDepthStencilStates(ID3D11Device* device);
		bool CreateRasterizerStates(ID3D11Device* device);
		bool CreateSamplerStates(ID3D11Device* device);

	private:
		ID3D11DeviceContext* Context = nullptr;
		std::array<ID3D11BlendState*, (U64)EBlendStates::Count> BlendStates;
		std::array<ID3D11DepthStencilState*, (U64)EDepthStencilStates::Count> DepthStencilStates;
		std::array<ID3D11RasterizerState*, (U64)ERasterizerStates::Count> RasterizerStates;
		std::array<ID3D11SamplerState*, (U64)ESamplerStates::Count> SamplerStates;
	};
}
