// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "RenderResourceRegistry.h"

namespace Havtorn
{
	class CRHI;

	struct SRenderPass
	{
		CHavtornStaticString<32> Name;
		std::vector<SRenderResourceHandle> Inputs;
		std::vector<SRenderResourceHandle> Outputs;
		std::function<void(CRHI*, const std::vector<SRenderResource*>&)> Function;
	};

	class CRenderGraph
	{
	public:
		CRenderGraph() = default;
		~CRenderGraph() = default;
		
		void AddPass(CHavtornStaticString<32> name, const std::function<void()> setup, const std::function<void(CRHI*, const std::vector<SRenderResource*>&)> execution);

		void Compile();
		void Execute();

	private:
		std::vector<SRenderPass> RenderPasses;
		CRenderResourceRegistry ResourceRegistry;
	};
}
