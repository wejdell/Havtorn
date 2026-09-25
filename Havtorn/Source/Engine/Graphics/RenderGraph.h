// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "RenderResourceRegistry.h"

// 2026-09-01 //Ta in ResourceRegistry och handles i passData vi får efter setup, plocka ut en färdig buffer av rätt storlek (DirectionalLightBuffer) som inte lagras här, 
		//	och fyll den med resten av datat i passData. PassData måste då lagra varje instance av basically ett render command. RenderGraph får ansvaret att hålla rätt på buffer objekten, 
		//	men passDatat skickas också över till render thread och är kopierat. får kolla hur vi gör med importerade texturer. kan bli knas att automatiskt bind-a buffrar (i RenderGraph::Execute)
		//	innan vi fyllt dem med passData, men kanske är ok? kort sagt ska passData innehålla datan vi behöver, och resource handles. varje stateless render module funktion (som RenderDirectionalLight)
		//	behöver tillgång till resource registry, passdata och render state manager (den som kan rendera). bufferdatat måste leva nånstans.
		//	Från render graph talken: "setup lambda may capture everything by reference, but execute must capture by value"

		//	kan vi istället göra så att alla render module funktioner tar resource handles bara? då får vi lagra datan vi ska binda i resource registryn på nåt vis? mellanlagra den. istället för att 
		//	den blir captured i execute

		//	man kan göra så att man bygger upp ett litet bibliotek av modulära render module functions, och att man bindar resurserna till dem i execute lambdas, där datan får lagras. så bindar man 
		//	bufferdatat i executen innan man kör render module funktionen! eventuellt döper vi om de här funktionerna, decentraliserar passDatat något. eventuellt gör static function library som modul?
		//	kan vara så att vi i så fall vill göra samma grej som frostbite, och asserta sizen på execute-lambdas, iom att riktiga datan lagras där

namespace Havtorn
{
	class CRHI;
	class CRenderManager;

	class IRenderPass
	{
	public:
		virtual void Execute(CRenderResourceRegistry& registry, CRenderManager* renderManager) = 0;
		std::vector<SRenderResourceHandle> Inputs;
		std::vector<SRenderResourceHandle> Outputs;
		std::vector<U64> Dependencies;
		U64 PassParamHash = 0;
	};

	struct SRenderPassParams
	{
		// These are used for setting up automatic sync points in the graph.
		// Whenever we can't assume what data should be bound, it should be bound in 
		// the execute function instead.
		std::vector<SRenderResourceHandle> Inputs;
		std::vector<SRenderResourceHandle> Outputs;
		std::vector<U64> Dependencies;
	};

	template<typename T>
	concept RenderPassParamType = std::derived_from<T, SRenderPassParams>;

	template<RenderPassParamType TParamStruct>
	class CRenderPass : public IRenderPass
	{
	public:
		using TRenderPassExecuteSignature = std::function<void(const TParamStruct&, CRenderResourceRegistry&, CRenderManager*)>;

		CRenderPass() = default;
		CRenderPass(const std::string_view debugName, const TParamStruct& data, TRenderPassExecuteSignature&& execution);
		CRenderPass() = delete;

		virtual void Execute(CRenderResourceRegistry& registry, CRenderManager* renderManager) override;
		
		CHavtornStaticString<RenderDebugNameMaxSize> Name;
		TParamStruct Data;
	
	private:
		TRenderPassExecuteSignature ExecutionFunction;
		HV_STATIC_ASSERT(sizeof(ExecutionFunction) < 1024, "Execution Function is binding too much data!");
	};

	class CRenderGraph
	{
	public:
		CRenderGraph(CRenderManager* manager);
		~CRenderGraph() = default;
		
		template<RenderPassParamType TParamStruct>
		TParamStruct* AllocateParameters();

		template<RenderPassParamType TParamStruct>
		IRenderPass* GetPassFromPassParams() const;

		template<RenderPassParamType TDependerPassData, RenderPassParamType TDependeePassData>
		TDependeePassData* GetPassData() const;

		template<RenderPassParamType TParamStruct>
		void AddPass(const std::string_view debugName, const std::function<TParamStruct(CRenderResourceRegistry&)> setup, std::function<void(const TParamStruct&, CRenderResourceRegistry&, CRenderManager*)>&& execution);

		void Compile();
		void Execute(CRenderManager* renderManager);

	private:
		//std::unordered_map<U64, std::vector<U64>> PassParamHashToPassIndices;
		//std::unordered_map<U64, std::unordered_set<U64>> PassParamHashToDependencies;
		std::unordered_map<U64, U64> PassParamHashToPassIndex;

		std::vector<Ref<IRenderPass>> RenderPasses;
		std::vector<Ref<IRenderPass>> SortedPasses;
		CRenderResourceRegistry ResourceRegistry;
		CRenderManager* RenderManager = nullptr;
	};

	template<RenderPassParamType TParamStruct>
	inline CRenderPass<TParamStruct>::CRenderPass(const std::string_view debugName, const TParamStruct& data, TRenderPassExecuteSignature&& execution)
		: ExecutionFunction(std::move(execution))
		, Data(data)
	{	
		PassParamHash = typeid(TParamStruct).hash_code();
		Inputs = Data.Inputs;
		Outputs = Data.Outputs;
		Dependencies = Data.Dependencies;
		const U64 nameLength = UMath::Min(debugName.size(), RenderDebugNameMaxSize);
		Name = debugName.substr(0, nameLength).data();
	}

	template<RenderPassParamType TParamStruct>
	inline void CRenderPass<TParamStruct>::Execute(CRenderResourceRegistry& registry, CRenderManager* renderManager)
	{
		ExecutionFunction(Data, registry, renderManager);
	}

	template<RenderPassParamType TParamStruct>
	inline TParamStruct* CRenderGraph::AllocateParameters()
	{
		Ref<CRenderPass<TParamStruct>> pass = std::make_shared(CRenderPass<TParamStruct>());
		PassParamHashToPassIndex[typeid(TParamStruct).hash_code()] = RenderPasses.size();
		RenderPasses.emplace_back(pass);
		return &pass->Data;
	}

	template<RenderPassParamType TParamStruct>
	inline IRenderPass* CRenderGraph::GetPassFromPassParams() const
	{
		return RenderPasses[PassParamHashToPassIndex.at(typeid(TParamStruct).hash_code())];
	}

	template<RenderPassParamType TDependerPassData, RenderPassParamType TDependeePassData>
	inline TDependeePassData* CRenderGraph::GetPassData() const
	{
		std::vector<U64>& dependencies = GetPassFromPassParams<TDependerPassData>()->Dependencies;

		if (std::ranges::find(dependencies, typeid(TDependeePassData).hash_code()) == dependencies.end())
			dependencies.push_back(typeid(TDependeePassData).hash_code());
		
		return &GetPassFromPassParams<TDependeePassData>()->Data;
	}

	template<RenderPassParamType TParamStruct>
	inline void CRenderGraph::AddPass(const std::string_view debugName, const std::function<TParamStruct(CRenderResourceRegistry&)> setup, std::function<void(const TParamStruct&, CRenderResourceRegistry&, CRenderManager*)>&& execution)
	{
		const U64 passParamHash = typeid(TParamStruct).hash_code();
		if (!PassParamHashToPassIndices.contains(passParamHash))
		{
			PassParamHashToPassIndices.emplace(passParamHash);
			PassParamHashToDependencies.emplace(passParamHash);
		}

		PassParamHashToPassIndices.at(passParamHash).push_back(RenderPasses.size());
		RenderPasses.emplace_back(std::make_shared(CRenderPass<TParamStruct>(debugName, setup(ResourceRegistry), std::move(execution))));
		
		for (const U64 dependency : RenderPasses.back()->Dependencies)
			PassParamHashToDependencies.at(passParamHash).insert(dependency);
	}
}
