// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>

#include <d3d11.h>

namespace Havtorn
{
    class CRHI;

    class CRenderViewport
    {
    public:
        PLATFORM_API CRenderViewport(CRHI* rhi, const F32 topLeftX, const F32 topLeftY, const F32 width, const F32 height, const F32 minDepth, const F32 maxDepth);
        CRenderViewport() = default;
        ~CRenderViewport() = default;
    
        PLATFORM_API void SetViewport() const;

        F32 TopLeftX = 0.0f;
        F32 TopLeftY = 0.0f;
        F32 Width = 0.0f;
        F32 Height = 0.0f;
        F32 MinDepth = 0.0f;
        F32 MaxDepth = 0.0f;

    private:
        ID3D11DeviceContext* Context = nullptr;
    };
}
