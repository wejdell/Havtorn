// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DepthStencilState.h"
#include "RHI/RHI.h"

#include <fstream>

namespace Havtorn
{
	CDepthStencilState::CDepthStencilState(const CRHI* rhi, const SDepthStencilDescription& description)
		: Context(rhi->GetContext())
	{
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
        depthStencilDesc.DepthEnable = description.EnableDepth;
        depthStencilDesc.DepthWriteMask = static_cast<D3D11_DEPTH_WRITE_MASK>(description.DepthWriteMask);
        depthStencilDesc.DepthFunc = static_cast<D3D11_COMPARISON_FUNC>(description.DepthFunction);
        depthStencilDesc.StencilEnable = description.EnableStencil;
        depthStencilDesc.StencilReadMask = description.StencilReadMask;
        depthStencilDesc.StencilWriteMask = description.StencilWriteMask;
        depthStencilDesc.FrontFace.StencilFailOp = static_cast<D3D11_STENCIL_OP>(description.FrontFaceStencilOperation.StencilFailOperation);
        depthStencilDesc.FrontFace.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(description.FrontFaceStencilOperation.StencilDepthFailOperation);
        depthStencilDesc.FrontFace.StencilPassOp = static_cast<D3D11_STENCIL_OP>(description.FrontFaceStencilOperation.StencilPassOperation);
        depthStencilDesc.FrontFace.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(description.FrontFaceStencilOperation.StencilFunction);
		depthStencilDesc.BackFace.StencilFailOp = static_cast<D3D11_STENCIL_OP>(description.BackFaceStencilOperation.StencilFailOperation);
		depthStencilDesc.BackFace.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(description.BackFaceStencilOperation.StencilDepthFailOperation);
		depthStencilDesc.BackFace.StencilPassOp = static_cast<D3D11_STENCIL_OP>(description.BackFaceStencilOperation.StencilPassOperation);
		depthStencilDesc.BackFace.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(description.BackFaceStencilOperation.StencilFunction);

        ENGINE_HR_MESSAGE(rhi->GetDevice()->CreateDepthStencilState(&depthStencilDesc, &DepthStencilState), "Depth Stencil State could not be created.");
	}

	void CDepthStencilState::Release()
	{
		DepthStencilState->Release();
	}

	void CDepthStencilState::SetDepthStencilState(const U32 stencilRef) const
	{
		Context->OMSetDepthStencilState(DepthStencilState, stencilRef);
	}

	void CDepthStencilState::ResetDepthStencilState(const CRHI* rhi)
	{
		rhi->GetContext()->OMSetDepthStencilState(nullptr, 0);
	}
}
