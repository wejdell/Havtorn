// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>

#include <d3d11.h>

namespace Havtorn
{
	class CRHI;

    enum class EBlendFactor : U8
    {
        Zero = 1,
        One = 2,
        SourceColor = 3,
        InverseSourceColor = 4,
        SourceAlpha = 5,
        InverseSourceAlpha = 6,
        DestinationAlpha = 7,
        InverseDestinationAlpha = 8,
        DestinationColor = 9,
        InverseDestinationColor = 10,
        SourceAlphaSaturated = 11,
        CustomBlendFactor = 14,
        InverseCustomBlendFactor = 15,
        SourceOneColor = 16,
        InverseSourceOneColor = 17,
        SourceOneAlpha = 18,
        InverseSourceOneAlpha = 19
    };

    enum class EBlendOperation : U8
    {
        Add = 1,
        Subtract = 2,
        ReverseSubtract = 3,
        Minimum = 4,
        Maximum = 5
    };

    enum class EBlendColorWriteEnable : U8
    {
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8,
        All = (((Red | Green) | Blue) | Alpha)
    };

    struct SRenderTargetBlendDescription
    {
        bool EnableBlend = false;
        EBlendFactor SourceBlend = EBlendFactor::One;
        EBlendFactor DestinationBlend = EBlendFactor::Zero;
        EBlendOperation BlendOperation = EBlendOperation::Add;
        EBlendFactor SourceAlphaBlend = EBlendFactor::One;
        EBlendFactor DestinationAlphaBlend = EBlendFactor::Zero;
        EBlendOperation AlphaBlendOperation = EBlendOperation::Add;
        U8 RenderTargetWriteMask = STATIC_U8(EBlendColorWriteEnable::All);
    };

    struct SBlendStateDescription
    {
        bool EnableAlphaToCoverage = false;
        bool EnableIndependentBlend = false;
        SRenderTargetBlendDescription RenderTargetDescriptions[8];
    };

	class CBlendState
	{
	public:
		PLATFORM_API CBlendState(const CRHI* rhi, const SBlendStateDescription& description);
		CBlendState() = delete;
		~CBlendState() = default;

		PLATFORM_API void Release();
		PLATFORM_API void SetBlendState() const;
		PLATFORM_API static void ResetBlendState(const CRHI* rhi);

	private:
		ID3D11DeviceContext* Context = nullptr;
        ID3D11BlendState* BlendState = nullptr;
	};
}
