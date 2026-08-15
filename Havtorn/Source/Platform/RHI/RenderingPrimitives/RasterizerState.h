// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>

#include <d3d11.h>

namespace Havtorn
{
    class CRHI;

    enum class ERasterizerFillMode
    {
        Wireframe = 2, // D3D11_FILL_WIREFRAME
        Solid = 3 // D3D11_FILL_SOLID
    };

    enum class ERasterizerCullMode
    {
        None = 1, // D3D11_CULL_NONE
        Frontface = 2, // D3D11_CULL_FRONT 
        Backface = 3 // D3D11_CULL_BACK
    };

    struct SRasterizerDescription
    {
        ERasterizerFillMode FillMode = ERasterizerFillMode::Solid;
        ERasterizerCullMode CullMode = ERasterizerCullMode::Backface;
        bool FrontCounterClockwise = false;
        I32 DepthBias = 0;
        F32 DepthBiasClamp = 0.0f;
        F32 SlopeScaledDepthBias = 0.0f;
        bool EnableDepthClip = true;
        bool EnableScissor = false;
        bool EnableMultisample = false;
        bool EnableAntialiasedLine = false;
    };

    class CRasterizerState
    {
    public:
        PLATFORM_API CRasterizerState(const CRHI* rhi, const SRasterizerDescription& description);
        CRasterizerState() = delete;
        ~CRasterizerState() = default;

        PLATFORM_API void Release();
        PLATFORM_API void SetRasterizerState() const;
        PLATFORM_API static void ResetRasterizerState(const CRHI* rhi);

    private:
        ID3D11DeviceContext* Context = nullptr;
        ID3D11RasterizerState* RasterizerState = nullptr;
    };
}
