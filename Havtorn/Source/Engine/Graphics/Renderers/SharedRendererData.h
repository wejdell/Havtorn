// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	class CRenderStateManager;
	struct SRenderResource;

	struct SMainTargetRenderData
	{
		SRenderResource* MainRenderTarget = nullptr;
		SRenderResource* DepthTexture = nullptr;
	};

	struct SGeometryTargetRenderData
	{
		std::vector<SRenderResource*> GetRenderTargets() const
		{
			return { AlbedoRenderTarget, WorldNormalRenderTarget, VertexNormalRenderTarget, MaterialRenderTarget };
		}

		SRenderResource* AlbedoRenderTarget = nullptr;
		SRenderResource* WorldNormalRenderTarget = nullptr;
		SRenderResource* VertexNormalRenderTarget = nullptr;
		SRenderResource* MaterialRenderTarget = nullptr;
		SRenderResource* DepthTexture = nullptr;
	};

	struct SGeometryConstantRenderData
	{
		SRenderResource* ViewDataBuffer = nullptr;
	};

	struct SEditorRenderData
	{
		std::vector<SRenderResource*> GetRenderTargets() const
		{
			return { WorldPositionRenderTarget, EditorDataRenderTarget };
		}

		SRenderResource* WorldPositionRenderTarget = nullptr;
		SRenderResource* EditorDataRenderTarget = nullptr;
		SRenderResource* EntityBuffer = nullptr;
	};
}