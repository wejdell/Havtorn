// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderGraph.h"
#include "RenderManager.h"

#include <CoreTypes.h>
#include <MathTypes/MathUtilities.h>

namespace Havtorn
{
	CRenderGraph::CRenderGraph(CRenderManager* manager)
		: RenderManager(manager)
	{}

	void TraverseDependencies(const Ref<IRenderPass>& pass, const std::vector<Ref<IRenderPass>>& sortedPasses, const std::unordered_map<U64, U64>& sortedDependencies)
	{
		for (U64 dependencyHash : pass->Dependencies)
		{
			if (!sortedDependencies.contains(dependencyHash))
				return; // If one of the dependencies hasn't been added to the list, this pass should be culled

			TraverseDependencies(sortedPasses[sortedDependencies.at(dependencyHash)], sortedPasses, sortedDependencies);
		}
		// TODO.NW: Double check that this works. The intent is to mark all dependencies of root passes as another root pass, to then cull the rest
		pass->IsRootPass = true;
	}

	void CRenderGraph::Compile()
	{
		// Sort passes
		const U64 numPasses = RenderPasses.size();
		std::vector<SVector2<U64>> dependencyEdges;
		
		for (U64 passIndex = 0; passIndex < numPasses; passIndex++)
		{
			for (const U64 dependencyHash : RenderPasses[passIndex]->Dependencies)
			{
				if (!PassParamHashToPassIndex.contains(dependencyHash))
					continue; // NW: Dependency pass was not added this frame, this pass will be culled

				dependencyEdges.emplace_back(PassParamHashToPassIndex.at(dependencyHash), passIndex);
			}
		}

		std::vector<U64> sortedIndices = UMathUtilities::TopologicalSortKahn(dependencyEdges, numPasses);
		if (sortedIndices.empty())
			return;

		std::vector<Ref<IRenderPass>> defaultRootPasses;
		for (const U64 sortedIndex : sortedIndices)
		{
			Ref<IRenderPass> pass = RenderPasses[sortedIndex];
			SortedPassParamHashToPassIndex.emplace(pass->PassParamHash, SortedPasses.size());
			SortedPasses.emplace_back(pass);
			
			if (pass->IsRootPass)
				defaultRootPasses.emplace_back(pass);
		}

		// TODO.NW: Should we have separate graphs for each render view? Would be nice for debugging
		
		for (Ref<IRenderPass> defaultRootPass : defaultRootPasses)
			TraverseDependencies(defaultRootPass, SortedPasses, SortedPassParamHashToPassIndex);

		std::ranges::remove_if(SortedPasses, [](const Ref<IRenderPass>& pass) { return !pass->IsRootPass; });

		for (const Ref<IRenderPass>& pass : SortedPasses)
		{
			ResourceRegistry.TouchResources(pass->Inputs);
			ResourceRegistry.TouchResources(pass->Outputs);
		}
	}

	void CRenderGraph::Execute(CRenderManager* renderManager)
	{
		Compile();

		ResourceRegistry.Allocate();

		for (const Ref<IRenderPass>& pass : SortedPasses)
			pass->Execute(ResourceRegistry, RenderManager);

		ResourceRegistry.Deallocate();
	}
}
