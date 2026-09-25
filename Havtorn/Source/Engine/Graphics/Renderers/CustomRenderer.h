// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"

// TODO.NW: Figure out what to do with this, would like to easily expose a way to set up a 
// custom render pass on the game side.

namespace Havtorn
{
	class CRenderStateManager;
	struct SRenderResource;

	struct SCustomPassData
	{
		SRenderResource* Resource = nullptr;
	};
	
	class CCustomRenderer
	{
	public:
		CCustomRenderer(CRenderStateManager* stateManager);
		~CCustomRenderer() = default;

		void RenderCustomPass(const SCustomPassData& passData);

	private:
		U16 CustomPassPSOIndex = 0;

		CRenderStateManager* RenderStateManager = nullptr;
	};
}
