// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>
#include <MathTypes/HavtornMath.h>

#include "RHI/RHIEnums.h"

#include <d3d11.h>

namespace Havtorn
{
    class CRHI;

    // Ordered in terms of minification (Min), magnification (Mag), and mip-level sampling (Mip).
    // Each of these may be point sampled (e.g. MinPoint) or linearly interpolated (e.g. MinLinear), or they may all be anisotropically interpolated (e.g. Anisotropic).
    // Additionally, the sampled result may be compared against the comparison value (e.g. ComparisonMinMagMipPoint) or we may forego filtering and instead return
    // the minimum or maximum texel value (e.g.MinimumMinMagMipPoint, MaximumMinMagMipPoint). Support for the latter options may be queried from feature data.
    enum class ESamplerFilter : U16
    {
        MinMagMipPoint = 0, // D3D11_FILTER_MIN_MAG_MIP_POINT
        MinMagPointMipLinear = 0x1, // D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR
        MinPointMagLinearMipPoint = 0x4, // D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT
        MinPointMagMipLinear = 0x5, // D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR
        MinLinearMagMipPoint = 0x10, // D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT
        MinLinearMagPointMipLinear = 0x11, // D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR
        MinMagLinearMipPoint = 0x14, // D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT
        MinMagMipLinear = 0x15, // D3D11_FILTER_MIN_MAG_MIP_LINEAR
        Anisotropic = 0x55, // D3D11_FILTER_ANISOTROPIC
        ComparisonMinMagMipPoint = 0x80, // D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT
        ComparisonMinMagPointMipLinear = 0x81, // D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR
        ComparisonMinPointMagLinearMipPoint = 0x84, // D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT
        ComparisonMinPointMagMipLinear = 0x85, // D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR
        ComparisonMinLinearMagMipPoint = 0x90, // D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT
        ComparisonMinLinearMagPointMipLinear = 0x91, // D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR
        ComparisonMinMagLinearMipPoint = 0x94, // D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT
        ComparisonMinMagMipLinear = 0x95, // D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR
        ComparisonAnisotropic = 0xd5, // D3D11_FILTER_COMPARISON_ANISOTROPIC
        MinimumMinMagMipPoint = 0x100, // D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT
        MinimumMinMagPointMipLinear = 0x101, // D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR
        MinimumMinPointMagLinearMipPoint = 0x104, // D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT
        MinimumMinPointMagMipLinear = 0x105, // D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR
        MinimumMinLinearMagMipPoint = 0x110, // D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT
        MinimumMinLinearMagPointMipLinear = 0x111, // D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR
        MinimumMinMagLinearMipPoint = 0x114, // D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT
        MinimumMinMagMipLinear = 0x115, // D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR
        MinimumAnisotropic = 0x155, // D3D11_FILTER_MINIMUM_ANISOTROPIC
        MaximumMinMagMipPoint = 0x180, // D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT
        MaximumMinMagPointMipLinear = 0x181, // D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR
        MaximumMinPointMagLinearMipPoint = 0x184, // D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT
        MaximumMinPointMagMipLinear = 0x185, // D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR
        MaximumMinLinearMagMipPoint = 0x190, // D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT
        MaximumMinLinearMagPointMipLinear = 0x191, // D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR
        MaximumMinMagLinearMipPoint = 0x194, // D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT
        MaximumMinMagMipLinear = 0x195, // D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR
        MaximumAnisotropic = 0x1d5 // D3D11_FILTER_MAXIMUM_ANISOTROPIC
    };

    enum class ESamplerFilterType : U8
    {
        Point = 0, // D3D11_FILTER_TYPE_POINT
        Linear = 1 // D3D11_FILTER_TYPE_LINEAR
    };

    enum class ESamplerFilterReductionType : U8
    {
        Standard = 0, // D3D11_FILTER_REDUCTION_TYPE_STANDARD
        Comparison = 1, // D3D11_FILTER_REDUCTION_TYPE_COMPARISON
        Minimum = 2, // D3D11_FILTER_REDUCTION_TYPE_MINIMUM
        Maximum = 3 // D3D11_FILTER_REDUCTION_TYPE_MAXIMUM
    };

    enum class ETextureAddressMode : U8
    {
        Wrap = 1, // D3D11_TEXTURE_ADDRESS_WRAP
        Mirror = 2, // D3D11_TEXTURE_ADDRESS_MIRROR
        Clamp = 3, // D3D11_TEXTURE_ADDRESS_CLAMP
        Border = 4, // D3D11_TEXTURE_ADDRESS_BORDER
        MirrorOnce = 5 // D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
    };

    struct SSamplerDescription
    {
        ESamplerFilter Filter = ESamplerFilter::MinMagMipLinear;
        ETextureAddressMode AddressU = ETextureAddressMode::Clamp;
        ETextureAddressMode AddressV = ETextureAddressMode::Clamp;
        ETextureAddressMode AddressW = ETextureAddressMode::Clamp;
        F32 MipLODBias = 0.0f;
        U32 MaxAnisotropy = 1;
        ERenderComparisonFunction ComparisonFunction = ERenderComparisonFunction::Never;
        F32 BorderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        F32 MinLOD = -UMath::MaxFloat;
        F32 MaxLOD = UMath::MaxFloat;
    };

    class CSamplerState
    {
    public:
        PLATFORM_API CSamplerState(const CRHI* rhi, const SSamplerDescription& description);
        CSamplerState() = delete;
        ~CSamplerState() = default;

        PLATFORM_API void Release();
        PLATFORM_API void SetSamplerState(const U8 slot) const;
        PLATFORM_API static void ResetSamplerState(const CRHI* rhi);

    private:
        ID3D11DeviceContext* Context = nullptr;
        ID3D11SamplerState* SamplerState = nullptr;
    };
}
