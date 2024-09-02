// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderStateManager.h"
#include "GraphicsFramework.h"

namespace Havtorn
{
    CRenderStateManager::~CRenderStateManager()
    {
        Context = nullptr;
    }

    bool CRenderStateManager::Init(CGraphicsFramework* framework)
    {
        Context = framework->GetContext();
        ID3D11Device* device = framework->GetDevice();

        ENGINE_ERROR_BOOL_MESSAGE(Context, "Could not bind context.");
        ENGINE_ERROR_BOOL_MESSAGE(device, "Device is null.");

        ENGINE_ERROR_BOOL_MESSAGE(CreateBlendStates(device), "Could not create Blend States.");
        ENGINE_ERROR_BOOL_MESSAGE(CreateDepthStencilStates(device), "Could not create Depth Stencil States.");
        ENGINE_ERROR_BOOL_MESSAGE(CreateRasterizerStates(device), "Could not create Rasterizer States.");
        ENGINE_ERROR_BOOL_MESSAGE(CreateSamplerStates(device), "Could not create Sampler States.");

        return true;
    }

    void CRenderStateManager::SetBlendState(EBlendStates blendState)
    {
        std::array<F32, 4> blendFactors = { 0.5f, 0.5f, 0.5f, 0.5f };
        Context->OMSetBlendState(BlendStates[(U64)blendState], blendFactors.data(), 0xFFFFFFFFu);
    }

    void CRenderStateManager::SetDepthStencilState(EDepthStencilStates depthStencilState, UINT stencilRef)
    {
        Context->OMSetDepthStencilState(DepthStencilStates[(U64)depthStencilState], stencilRef);
    }

    void CRenderStateManager::SetRasterizerState(ERasterizerStates rasterizerState)
    {
        Context->RSSetState(RasterizerStates[(size_t)rasterizerState]);
    }

    void CRenderStateManager::SetSamplerState(ESamplerStates samplerState)
    {
        Context->VSSetSamplers(0, 1, &SamplerStates[(U64)samplerState]);
        Context->PSSetSamplers(0, 1, &SamplerStates[(U64)samplerState]);
    }

    void CRenderStateManager::SetAllStates(EBlendStates blendState, EDepthStencilStates depthStencilState, ERasterizerStates rasterizerState, ESamplerStates samplerState)
    {
        SetBlendState(blendState);
        SetDepthStencilState(depthStencilState);
        SetRasterizerState(rasterizerState);
        SetSamplerState(samplerState);
    }

    void CRenderStateManager::SetAllDefault()
    {
        SetBlendState(EBlendStates::Disable);
        SetDepthStencilState(EDepthStencilStates::Default);
        SetRasterizerState(ERasterizerStates::Default);
        SetSamplerState(ESamplerStates::Trilinear);
    }

    void CRenderStateManager::Release()
    {
        for (U8 i = 0; i < static_cast<U8>(EBlendStates::Count); ++i)
        {
            BlendStates[i]->Release();
        }

        for (U8 i = 0; i < static_cast<U8>(EDepthStencilStates::Count); ++i)
        {
            DepthStencilStates[i]->Release();
        }

        for (U8 i = 0; i < static_cast<U8>(ERasterizerStates::Count); ++i)
        {
            RasterizerStates[i]->Release();
        }

        for (U8 i = 0; i < static_cast<U8>(ESamplerStates::Count); ++i)
        {
            SamplerStates[i]->Release();
        }
    }

    bool CRenderStateManager::CreateBlendStates(ID3D11Device* device)
    {
        D3D11_BLEND_DESC alphaBlendDesc = { 0 };
        alphaBlendDesc.RenderTarget[0].BlendEnable = true;
        alphaBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        alphaBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        alphaBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        alphaBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        alphaBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        alphaBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
        alphaBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        D3D11_BLEND_DESC additiveBlendDesc = { 0 };
        additiveBlendDesc.RenderTarget[0].BlendEnable = true;
        additiveBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        additiveBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        additiveBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        additiveBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        additiveBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        additiveBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
        additiveBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        CD3D11_BLEND_DESC gbufferBlendDesc{ D3D11_DEFAULT };
        gbufferBlendDesc.IndependentBlendEnable = true;
        for (unsigned int i = 0; i < 4; ++i) // 4 targets in GBuffer
        {
            gbufferBlendDesc.RenderTarget[i].BlendEnable = TRUE;
            gbufferBlendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            gbufferBlendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            gbufferBlendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
            gbufferBlendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
            gbufferBlendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
            gbufferBlendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            gbufferBlendDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
        }

        ID3D11BlendState* alphaBlendState;
        ENGINE_HR_MESSAGE(device->CreateBlendState(&alphaBlendDesc, &alphaBlendState), "Alpha Blend State could not be created.");

        ID3D11BlendState* additiveBlendState;
        ENGINE_HR_MESSAGE(device->CreateBlendState(&additiveBlendDesc, &additiveBlendState), "Additive Blend State could not be created");

        ID3D11BlendState* gbufferBlendState;
        ENGINE_HR_MESSAGE(device->CreateBlendState(&gbufferBlendDesc, &gbufferBlendState), "GBuffer Alpha Blend State could not be created");

        BlendStates[(U64)EBlendStates::Disable] = nullptr;
        BlendStates[(U64)EBlendStates::AlphaBlend] = alphaBlendState;
        BlendStates[(U64)EBlendStates::AdditiveBlend] = additiveBlendState;
        BlendStates[(U64)EBlendStates::GBufferAlphaBlend] = gbufferBlendState;

        return true;
    }

    bool CRenderStateManager::CreateDepthStencilStates(ID3D11Device* device)
    {
        D3D11_DEPTH_STENCIL_DESC onlyreadDepthDesc = { 0 };
        onlyreadDepthDesc.DepthEnable = true;
        onlyreadDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        onlyreadDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
        onlyreadDepthDesc.StencilEnable = false;

        D3D11_DEPTH_STENCIL_DESC stencilWriteDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
        stencilWriteDesc.DepthEnable = true;
        stencilWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        stencilWriteDesc.DepthFunc = D3D11_COMPARISON_LESS;
        stencilWriteDesc.StencilEnable = TRUE;
        stencilWriteDesc.StencilReadMask = 0xFF;
        stencilWriteDesc.StencilWriteMask = 0xFF;
        stencilWriteDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        stencilWriteDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        stencilWriteDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        stencilWriteDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        D3D11_DEPTH_STENCIL_DESC stencilMaskDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
        stencilMaskDesc.DepthEnable = FALSE;
        stencilMaskDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        stencilMaskDesc.StencilEnable = TRUE;
        stencilMaskDesc.StencilReadMask = 0xFF;
        stencilMaskDesc.StencilWriteMask = 0xFF;
        stencilMaskDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        stencilMaskDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        stencilMaskDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

        D3D11_DEPTH_STENCIL_DESC depthFirstDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
        depthFirstDesc.DepthEnable = TRUE;
        depthFirstDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        depthFirstDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        depthFirstDesc.StencilEnable = FALSE;

        ID3D11DepthStencilState* onlyreadDepthStencilState;
        ENGINE_HR_MESSAGE(device->CreateDepthStencilState(&onlyreadDepthDesc, &onlyreadDepthStencilState), "OnlyRead Depth Stencil State could not be created.");

        ID3D11DepthStencilState* writeDepthStencilState;
        ENGINE_HR_MESSAGE(device->CreateDepthStencilState(&stencilWriteDesc, &writeDepthStencilState), "Write Stencil State could not be created.");

        ID3D11DepthStencilState* maskDepthStencilState;
        ENGINE_HR_MESSAGE(device->CreateDepthStencilState(&stencilMaskDesc, &maskDepthStencilState), "Mask Stencil State could not be created.");

        ID3D11DepthStencilState* depthFirstStencilState;
        ENGINE_HR_MESSAGE(device->CreateDepthStencilState(&depthFirstDesc, &depthFirstStencilState), "Depth First Stencil State could not be created.");

        DepthStencilStates[(U64)EDepthStencilStates::Default] = nullptr;
        DepthStencilStates[(U64)EDepthStencilStates::OnlyRead] = onlyreadDepthStencilState;
        DepthStencilStates[(U64)EDepthStencilStates::StencilWrite] = writeDepthStencilState;
        DepthStencilStates[(U64)EDepthStencilStates::StencilMask] = maskDepthStencilState;
        DepthStencilStates[(U64)EDepthStencilStates::DepthFirst] = depthFirstStencilState;

        return true;
    }

    bool CRenderStateManager::CreateRasterizerStates(ID3D11Device* device)
    {
        D3D11_RASTERIZER_DESC wireframeRasterizerDesc = {};
        wireframeRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
        wireframeRasterizerDesc.CullMode = D3D11_CULL_BACK;
        wireframeRasterizerDesc.DepthClipEnable = true;

        D3D11_RASTERIZER_DESC frontFaceDesc = {};
        frontFaceDesc.FillMode = D3D11_FILL_SOLID;
        frontFaceDesc.CullMode = D3D11_CULL_FRONT;
        frontFaceDesc.DepthClipEnable = true;

        D3D11_RASTERIZER_DESC noCullDesc = {};
        noCullDesc.FillMode = D3D11_FILL_SOLID;
        noCullDesc.CullMode = D3D11_CULL_NONE;
        noCullDesc.DepthClipEnable = true;

        ID3D11RasterizerState* wireframeRasterizerState;
        ENGINE_HR_MESSAGE(device->CreateRasterizerState(&wireframeRasterizerDesc, &wireframeRasterizerState), "Wireframe Rasterizer State could not be created.");

        ID3D11RasterizerState* frontFaceState;
        ENGINE_HR_MESSAGE(device->CreateRasterizerState(&frontFaceDesc, &frontFaceState), "Front face Rasterizer State could not be created.");

        ID3D11RasterizerState* noCullState;
        ENGINE_HR_MESSAGE(device->CreateRasterizerState(&noCullDesc, &noCullState), "No Face culling Rasterizer State could not be created.");

        RasterizerStates[(U64)ERasterizerStates::Default] = nullptr;
        RasterizerStates[(U64)ERasterizerStates::Wireframe] = wireframeRasterizerState;
        RasterizerStates[(U64)ERasterizerStates::FrontFaceCulling] = frontFaceState;
        RasterizerStates[(U64)ERasterizerStates::NoFaceCulling] = noCullState;

        return true;
    }

    bool CRenderStateManager::CreateSamplerStates(ID3D11Device* device)
    {
        D3D11_SAMPLER_DESC pointSamplerDesc = {};
        pointSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        pointSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        pointSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        pointSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        pointSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        pointSamplerDesc.MinLOD = -FLT_MAX;
        pointSamplerDesc.MaxLOD = FLT_MAX;

        D3D11_SAMPLER_DESC wrapSamplerDesc = {};
        wrapSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        wrapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        wrapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        wrapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        wrapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        wrapSamplerDesc.MinLOD = -FLT_MAX;
        wrapSamplerDesc.MaxLOD = FLT_MAX;

        ID3D11SamplerState* pointSamplerState;
        ENGINE_HR_MESSAGE(device->CreateSamplerState(&pointSamplerDesc, &pointSamplerState), "Point Sampler could not be created.");

        ID3D11SamplerState* wrapSamplerState;
        ENGINE_HR_MESSAGE(device->CreateSamplerState(&wrapSamplerDesc, &wrapSamplerState), "Wrap Sampler could not be created.");

        SamplerStates[(U64)ESamplerStates::Trilinear] = nullptr;
        SamplerStates[(U64)ESamplerStates::Point] = pointSamplerState;
        SamplerStates[(U64)ESamplerStates::Wrap] = wrapSamplerState;

        return true;
    }
}
