// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>

#include <d3d11.h>

namespace Havtorn
{
    class CRHI;

    enum class EDepthStencilComparisonFunction : U8
    {
        Never = 1,
        Less = 2,
        Equal = 3,
        LessOrEqual = 4,
        Greater = 5,
        NotEqual = 6,
        GreaterOrEqual = 7,
        Always = 8
    };

    enum class EDepthWriteMask : U8
    {
        Zero = 0,
        All = 1
    };

    enum class EDepthStencilOperation : U8
    {
        Keep = 1,
        Zero = 2,
        Replace = 3,
        IncrementSaturate = 4,
        DecrementSaturate = 5,
        Invert = 6,
        Increment = 7,
        Decrement = 8
    };

    struct SStencilOperationDescription
    {
        EDepthStencilOperation StencilFailOperation = EDepthStencilOperation::Keep;
        EDepthStencilOperation StencilDepthFailOperation = EDepthStencilOperation::Keep;
        EDepthStencilOperation StencilPassOperation = EDepthStencilOperation::Keep;
        EDepthStencilComparisonFunction StencilFunction = EDepthStencilComparisonFunction::Always;
    };

    struct SDepthStencilDescription
    {
        bool EnableDepth = true;
        EDepthWriteMask DepthWriteMask = EDepthWriteMask::All;
        EDepthStencilComparisonFunction DepthFunction = EDepthStencilComparisonFunction::Less;
        bool EnableStencil = false;
        U8 StencilReadMask = 255; // D3D11_DEFAULT_STENCIL_READ_MASK
        U8 StencilWriteMask = 255; // D3D11_DEFAULT_STENCIL_WRITE_MASK
        SStencilOperationDescription FrontFaceStencilOperation = {};
        SStencilOperationDescription BackFaceStencilOperation = {};
    };

    class CDepthStencilState
    {
    public:
        PLATFORM_API CDepthStencilState(const CRHI* rhi, const SDepthStencilDescription& description);
        CDepthStencilState() = delete;
        ~CDepthStencilState() = default;

        PLATFORM_API void Release();
        PLATFORM_API void SetDepthStencilState(const U32 stencilRef) const;
        PLATFORM_API static void ResetDepthStencilState(const CRHI* rhi);

    private:
        ID3D11DeviceContext* Context = nullptr;
        ID3D11DepthStencilState* DepthStencilState = nullptr;
    };
}
