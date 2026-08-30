// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "RenderResourceRegistry.h"

namespace Havtorn
{
	class CRHI;
	class CRenderManager;

	struct SRenderPassResourceDeclaration
	{
		std::vector<SRenderResourceDeclaration> Inputs;
		std::vector<SRenderResourceDeclaration> Outputs;
	};

	struct SRenderPass
	{
		CHavtornStaticString<RenderDebugNameMaxSize> Name;
		std::vector<SRenderResourceHandle> Inputs;
		std::vector<SRenderResourceHandle> Outputs;
		std::function<void(CRenderManager*)> ExecutionFunction;
	};

	class CRenderGraph
	{
	public:
		CRenderGraph() = default;
		~CRenderGraph() = default;
		
		void AddPass(CHavtornStaticString<RenderDebugNameMaxSize> name, const std::function<SRenderPassResourceDeclaration()> setup, std::function<void(CRenderManager*)>&& execution);

		void Compile();
		void Execute(CRenderManager* renderManager);

	private:
		std::vector<SRenderPass> RenderPasses;
		CRenderResourceRegistry ResourceRegistry;
	};
}
