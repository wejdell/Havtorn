#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <typeindex>
#include <initializer_list>

using U64 = std::uint64_t;
using U32 = std::uint32_t;
using U8 = std::uint8_t;
using I64 = std::int64_t;
using I32 = std::int32_t;

struct SEntity
{
	U64 GUID = 0;
};

struct SComponent
{
	SComponent() = delete;
	SComponent(const SEntity& entity)
		: EntityOwner(entity)
	{}
	virtual ~SComponent() = default;

	SEntity EntityOwner = SEntity();
};

struct STransform : public SComponent
{
	STransform(const SEntity& entity)
		: SComponent(entity)
	{};

	STransform(const SEntity& entity, float x, float y)
		: SComponent(entity)
		, X(x)
		, Y(y)
	{};

	float X = 0.f; 
	float Y = 0.f;
};

struct SMaterial : public SComponent
{
	SMaterial(const SEntity& entity)
		: SComponent(entity)
	{};

	SMaterial(const SEntity& entity, U8 id)
		: SComponent(entity)
		, ID(id)
	{};

	U8 ID = 0;
};

struct SComponentStorage
{
	std::unordered_map<U64, U64> EntityIndices;
	std::vector<SComponent*> Components;
};

struct SScene
{
	template<typename T, typename... Params>
	T* AddComponent(const SEntity& toEntity, Params... params)
	{
		const U64 typeIDHashCode = typeid(T).hash_code();
		if (!ComponentTypeIndices.contains(typeIDHashCode))
		{
			ComponentTypeIndices.emplace(typeIDHashCode, Storages.size());
			Storages.emplace_back();
		}

		SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeIDHashCode)];

		if (componentStorage.EntityIndices.contains(toEntity.GUID))
		{
			*(dynamic_cast<T*>(componentStorage.Components[componentStorage.EntityIndices.at(toEntity.GUID)])) = T(toEntity, params...);
		}
		else
		{
			componentStorage.EntityIndices.emplace(toEntity.GUID, componentStorage.Components.size());
			componentStorage.Components.emplace_back(new T(toEntity, params...));
		}
		
		return dynamic_cast<T*>(componentStorage.Components.back());
	}

	template<typename T>
	const SEntity& GetEntity(const T* fromComponent) const
	{
		return fromComponent->EntityOwner;
	}

	template<typename T>
	T* GetComponent(const SEntity& fromEntity) const
	{
		const U64 typeIDHashCode = typeid(T).hash_code();
		if (!ComponentTypeIndices.contains(typeIDHashCode))
			return nullptr;

		const SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeIDHashCode)];

		if (!componentStorage.EntityIndices.contains(fromEntity.GUID))
			return nullptr;

		return dynamic_cast<T*>(componentStorage.Components[componentStorage.EntityIndices.at(fromEntity.GUID)]);
	}

	template<typename... Ts>
	std::tuple<Ts*...> GetComponents(const SEntity& fromEntity) const
	{
		return std::make_tuple(GetComponent<Ts>(fromEntity) ...);
	}

	template<typename T>
	std::vector<T*> GetComponents()
	{
		const U64 typeIDHashCode = typeid(T).hash_code();
		if (!ComponentTypeIndices.contains(typeIDHashCode))
			return {};

		// NR: This looks problematic but works because we know we only fill buckets with the same component type. 
		// This would be a bad idea if we kept different derived components in the same vectors.

		SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeIDHashCode)];
		std::vector<T*> specializedComponents;
		specializedComponents.resize(componentStorage.Components.size());
		memcpy(&specializedComponents[0], componentStorage.Components.data(), sizeof(T*) * componentStorage.Components.size());

		return specializedComponents;
	}

	std::vector<SEntity> Entities;
	std::unordered_map<U64, U64> ComponentTypeIndices;
	std::vector<SComponentStorage> Storages;
};

struct A {};

int main()
{
	SScene scene;
	SEntity main = {111};
	SEntity second = {222};
	scene.Entities.emplace_back(main);
	scene.Entities.emplace_back(second);

	STransform* addedTransformComponent = scene.AddComponent<STransform>(main, 1.f, 2.f);
	STransform* transform = scene.GetComponent<STransform>(main);
	scene.AddComponent<SMaterial>(main);
	SMaterial* material = scene.GetComponent<SMaterial>(main);
	scene.AddComponent<STransform>(second, 0.f, 5.f);
	STransform* transform2 = scene.GetComponent<STransform>(second);
	scene.AddComponent<SMaterial>(second, 1);
	SMaterial* material2 = scene.GetComponent<SMaterial>(second);

	const SEntity& query1 = scene.GetEntity(addedTransformComponent);
	const SEntity& query2 = scene.GetEntity(transform);

	const std::vector<STransform*>& transformComponents = scene.GetComponents<STransform>();
	const SEntity& query3 = transformComponents[0]->EntityOwner;

	auto [transformBinding, materialBinding] = scene.GetComponents<STransform, SMaterial>(second);

	// TODO.NR: Try fix so that you can replace component for an entity
	// TODO.NR: Check memory contiguousness, force it somehow?

	return 0;
}