#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <typeindex>

using U64 = std::uint64_t;
using U32 = std::uint32_t;
using I64 = std::int64_t;
using I32 = std::int32_t;

struct SComponent
{
	SComponent() = default;
	virtual ~SComponent() = default;
};

struct STransform : public SComponent
{
	STransform() = default;
	STransform(float x, float y)
		: X(x)
		, Y(y)
	{};

	float X = 0.f; 
	float Y = 0.f;
};

struct SMaterial : public SComponent
{
	int ID = 0;
};

struct SEntity
{
	U64 GUID;
};

using SComponentTypeID = std::size_t;

struct SComponentMapNode
{
	std::pair<SComponent*, const SEntity&> ComponentNode;
};

struct SComponentStorage
{
	std::unordered_map<U64, U64> SparseTypeIndices;
	std::vector<SComponentMapNode> ComponentNodes;
};

struct SComponentMap
{
	SComponentMap() = default;
};

struct SScene
{
	template<typename T, typename... Params>
	void AddComponent(const SEntity& toEntity, Params... params)
	{
		U64 typeIDHashCode = typeid(T).hash_code();
		if (!SparseTypeIndices.contains(typeIDHashCode))
		{
			SparseTypeIndices.emplace(typeIDHashCode, Storages.size());
			Storages.emplace_back();
		}

		SComponentStorage& componentStorage = Storages[SparseTypeIndices.at(typeIDHashCode)];

		if (componentStorage.SparseTypeIndices.contains(toEntity.GUID))
		{
			componentStorage.ComponentNodes[componentStorage.SparseTypeIndices.at(toEntity.GUID)].ComponentNode
		}
		
		componentStorage.SparseTypeIndices.emplace(toEntity.GUID, componentStorage.ComponentNodes.size());
		componentStorage.ComponentNodes.emplace_back(SComponentMapNode{ std::pair<SComponent*, const SEntity&>(new T(params...), toEntity) });
	}

	template<typename T>
	T* GetComponent(const SEntity& fromEntity) const
	{
		U64 typeIDHashCode = typeid(T).hash_code();
		if (!SparseTypeIndices.contains(typeIDHashCode))
			return nullptr;

		const SComponentStorage& componentStorage = Storages[SparseTypeIndices.at(typeIDHashCode)];

		if (!componentStorage.SparseTypeIndices.contains(fromEntity.GUID))
			return nullptr;

		return dynamic_cast<T*>(componentStorage.ComponentNodes[componentStorage.SparseTypeIndices.at(fromEntity.GUID)].ComponentNode.first);
	}

	template<typename T>
	T* GetComponents(const SEntity& fromEntity) const
	{
		U64 typeIDHashCode = typeid(T).hash_code();
		if (!SparseTypeIndices.contains(typeIDHashCode))
			return nullptr;

		const SComponentStorage& componentStorage = Storages[SparseTypeIndices.at(typeIDHashCode)];

		if (!componentStorage.SparseTypeIndices.contains(fromEntity.GUID))
			return nullptr;

		return dynamic_cast<T*>(componentStorage.ComponentNodes[componentStorage.SparseTypeIndices.at(fromEntity.GUID)].ComponentNode.first);
	}

	std::vector<SEntity> Entities;
	std::unordered_map<U64, U64> SparseTypeIndices;
	std::vector<SComponentStorage> Storages;
};

struct A {};

int main()
{
	SScene scene;
	SEntity main;
	SEntity second;
	scene.Entities.emplace_back(main);

	scene.AddComponent<STransform>(main, 1.f, 2.f);
	STransform* transform = scene.GetComponent<STransform>(main);
	scene.AddComponent<SMaterial>(main);
	SMaterial* material = scene.GetComponent<SMaterial>(main);
	scene.AddComponent<STransform>(second, 0.f, 5.f);
	STransform* transform = scene.GetComponent<STransform>(main);

	// TODO.NR: Remove entity ref in pair? 
	// TODO.NR: Try fix so that you can replace component for an entity
	// TODO.NR: Check memory contiguousness, force it somehow?
	// TODO.NR: How to get full array of components? would be nice to figure out casting

	return 0;
}