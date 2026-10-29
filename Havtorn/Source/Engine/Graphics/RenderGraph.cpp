// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderGraph.h"
#include "RenderManager.h"

#include <CoreTypes.h>
#include <MathTypes/MathUtilities.h>

#include <algorithm>

namespace Havtorn
{
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

	bool CRenderGraph::HasDependency(const Ref<IRenderPass>& renderPassFrom, const Ref<IRenderPass>& renderPassTo)
	{
		const SVector2<U64> dependencyHashes = { renderPassFrom->PassParamHash, renderPassTo->PassParamHash };
		return std::ranges::contains(DependencyHashes, dependencyHashes);
	}

	void CRenderGraph::Compile()
	{
		// Sort passes
		const U64 numPasses = RenderPasses.size();
		std::vector<SVector2<U64>> dependencyIndices;
		
		for (const SVector2<U64>& dependency : DependencyHashes)
			dependencyIndices.emplace_back(PassParamHashToPassIndex.at(dependency.X), PassParamHashToPassIndex.at(dependency.Y));

		std::vector<U64> sortedIndices = UMathUtilities::TopologicalSortKahn(dependencyIndices, numPasses);
		if (sortedIndices.empty())
			return;

		std::vector<Ref<IRenderPass>> defaultRootPasses;
		for (const U64 sortedIndex : sortedIndices)
		{
			Ref<IRenderPass> pass = RenderPasses[sortedIndex];
			SortedPassParamHashToPassIndex.emplace(pass->PassParamHash, SortedPasses.size());
			SortedPasses.emplace_back(pass);
			
			for (const SVector2<U64>& dependencyHashes : DependencyHashes)
			{
				if (dependencyHashes.Y == pass->PassParamHash)
					pass->Dependencies.push_back(dependencyHashes.X);
			}

			if (pass->IsRootPass)
				defaultRootPasses.emplace_back(pass);
		}

		for (Ref<IRenderPass> defaultRootPass : defaultRootPasses)
			TraverseDependencies(defaultRootPass, SortedPasses, SortedPassParamHashToPassIndex);

		std::erase_if(SortedPasses, [](const Ref<IRenderPass>& pass) { return !pass->IsRootPass; });

		for (const Ref<IRenderPass>& pass : SortedPasses)
		{
			ResourceRegistry->TouchResources(pass->Inputs);
			ResourceRegistry->TouchResources(pass->Outputs);
		}
	}

	void CRenderGraph::Execute(CRenderManager* renderManager, SRenderView& renderView)
	{
		SortedPasses.clear();

		Compile();

		ResourceRegistry->Allocate();

		for (const Ref<IRenderPass>& pass : SortedPasses)
			pass->Execute(*ResourceRegistry, renderManager, renderView);

		RenderPasses.clear();
		DependencyHashes.clear();
	}

	const std::vector<Ref<IRenderPass>>& CRenderGraph::GetAllPasses()
	{
		return RenderPasses;
	}

	const std::vector<Ref<IRenderPass>>& CRenderGraph::GetSortedPasses()
	{
		return SortedPasses;
	}

	SRenderResourceHandle SRenderPassParams::AddInput(const SRenderResourceHandle& handle)
	{
		return Inputs.emplace_back(handle);
	}

	SRenderResourceHandle SRenderPassParams::AddOutput(const SRenderResourceHandle& handle)
	{
		return Outputs.emplace_back(handle);
	}

	void SRenderPassParams::SetIsRootPass()
	{
		ShouldSetRootPass = true;
	}
}
