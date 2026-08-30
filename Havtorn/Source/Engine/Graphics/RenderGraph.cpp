// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderGraph.h"
#include "RenderManager.h"

namespace Havtorn
{
	void CRenderGraph::AddPass(CHavtornStaticString<RenderDebugNameMaxSize> name, const std::function<SRenderPassResourceDeclaration()> setup, std::function<void(CRenderManager*)>&& execution)
	{
		std::vector<SRenderResourceHandle> inputs;
		std::vector<SRenderResourceHandle> outputs;

		SRenderPassResourceDeclaration passResourceDeclaration = setup();
		for (SRenderResourceDeclaration& resourceDeclaration : passResourceDeclaration.Inputs)
		{
			resourceDeclaration.Description.ID = UGeneralUtils::HashString(resourceDeclaration.Description.Name.AsString());
			inputs.emplace_back(ResourceRegistry.DeclareResource(resourceDeclaration));
		}
		for (SRenderResourceDeclaration& resourceDeclaration : passResourceDeclaration.Outputs)
		{
			resourceDeclaration.Description.ID = UGeneralUtils::HashString(resourceDeclaration.Description.Name.AsString());
			outputs.emplace_back(ResourceRegistry.DeclareResource(resourceDeclaration));
		}
		
		RenderPasses.emplace_back(name, inputs, outputs, std::move(execution));
	}

	void CRenderGraph::Compile()
	{
		// Sort passes

		for (const SRenderPass& pass : RenderPasses)
		{
			ResourceRegistry.TouchResources(pass.Inputs);
			ResourceRegistry.TouchResources(pass.Outputs);
		}
	}

	void CRenderGraph::Execute(CRenderManager* renderManager)
	{
		Compile();

		ResourceRegistry.Allocate();

		for (const SRenderPass& pass : RenderPasses)
		{
			ResourceRegistry.Bind(pass.Inputs, &renderManager->RenderStateManager);
			ResourceRegistry.Bind(pass.Outputs, &renderManager->RenderStateManager);
			pass.ExecutionFunction(renderManager);
		}

		RenderPasses.clear();
		
		ResourceRegistry.Deallocate();
	}
}
