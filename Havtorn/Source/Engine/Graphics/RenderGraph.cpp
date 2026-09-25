// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderGraph.h"
#include "RenderManager.h"

#include <MathTypes/MathUtilities.h>

namespace Havtorn
{
	CRenderGraph::CRenderGraph(CRenderManager* manager)
		: RenderManager(manager)
	{}

	//void CRenderGraph::AddPass(CHavtornStaticString<RenderDebugNameMaxSize> name, const std::function<SRenderPassResourceDeclaration()> setup, std::function<void(CRenderManager*)>&& execution)
	//{
	//	std::vector<SRenderResourceHandle> inputs;
	//	std::vector<SRenderResourceHandle> outputs;

	//	SRenderPassResourceDeclaration passResourceDeclaration = setup();
	//	for (SRenderResourceDeclaration& resourceDeclaration : passResourceDeclaration.Inputs)
	//	{
	//		resourceDeclaration.Description.ID = UGeneralUtils::HashString(resourceDeclaration.Description.Name.AsString());
	//		inputs.emplace_back(ResourceRegistry.DeclareResource(resourceDeclaration));
	//	}
	//	for (SRenderResourceDeclaration& resourceDeclaration : passResourceDeclaration.Outputs)
	//	{
	//		resourceDeclaration.Description.ID = UGeneralUtils::HashString(resourceDeclaration.Description.Name.AsString());
	//		outputs.emplace_back(ResourceRegistry.DeclareResource(resourceDeclaration));
	//	}
	//	
	//	RenderPasses.emplace_back(name, inputs, outputs, std::move(execution));
	//}

	void CRenderGraph::Compile()
	{
		// Sort passes
		const U64 numPasses = RenderPasses.size();
		std::vector<SVector2<U64>> dependencyEdges(numPasses);
		
		//for (U64 passIndex = 0; passIndex < numPasses; passIndex++)
		//{
		//	if (RenderPasses[passIndex]->Dependencies.empty())
		//		continue;

		//	for (const U64 dependencyHash : PassParamHashToDependencies.at(RenderPasses[passIndex]->PassParamHash))
		//	{
		//		for (U64 dependencyIndex = 0; dependencyIndex < PassParamHashToPassIndices.at(dependencyHash).size(); dependencyIndex++)
		//			dependencyEdges[passIndex] = SVector2<U64>(dependencyIndex, passIndex);
		//	}
		//}

		Make dependency edges from Dependecies vector

		std::vector<U64> sortedIndices = UMathUtilities::TopologicalSortKahn(dependencyEdges, numPasses);
		for (const U64 sortedIndex : sortedIndices)
			SortedPasses.emplace_back(RenderPasses[sortedIndex]);

		for (auto it = SortedPasses.rbegin(); it != SortedPasses.rend(); ++it)
		{

		}

		for (const Ref<IRenderPass>& pass : RenderPasses)
		{
			ResourceRegistry.TouchResources(pass.Inputs);
			ResourceRegistry.TouchResources(pass.Outputs);
		}
	}

	void CRenderGraph::Execute(CRenderManager* renderManager)
	{
		Compile();

		ResourceRegistry.Allocate();

		//for (const SRenderPass& pass : RenderPasses)
		//{
		//	ResourceRegistry.Bind(pass.Inputs, &renderManager->RenderStateManager);
		//	ResourceRegistry.Bind(pass.Outputs, &renderManager->RenderStateManager);
		//	pass.ExecutionFunction(renderManager);
		//}

		//RenderPasses.clear();

		for (const Ref<IRenderPass>& pass : RenderPasses)
		{
			pass->Execute(ResourceRegistry, RenderManager);
		}

		ResourceRegistry.Deallocate();
	}
}
