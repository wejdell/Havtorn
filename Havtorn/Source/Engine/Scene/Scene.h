// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/HavtornString.h"
#include "ECS/Entity.h"
#include "ECS/Component.h"
#include "ECS/ComponentView.h"

#include <unordered_map>
#include <map>
#include <tuple>

namespace Havtorn
{
	struct SComponentStorage
	{
		std::unordered_map<U64, U64> EntityIndices;
		std::vector<SComponent*> Components;
	};

	class CRenderManager;
	class CAssetRegistry;

	class CScene
	{
	public:
		CScene();
		~CScene();

		bool Init(CRenderManager* renderManager, const std::string& sceneName);

		bool Init3DDemoScene(CRenderManager* renderManager);
		bool Init2DDemoScene(CRenderManager* renderManager);

		// TODO.NR: Rework serialization to decrease amount of boilerplate
		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition, CAssetRegistry* assetRegistry);

		HAVTORN_API std::string GetSceneName() const;
		HAVTORN_API U64 GetSceneIndex(const SEntity& entity) const;
		HAVTORN_API U64 GetSceneIndex(const U64 entityGUID) const;
		
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
				std::string templateName = typeid(T).name();
				 HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to add component that already existed for entity with GUID : %i. Overwriting data on component.", templateName.c_str(), toEntity.GUID);
				*(dynamic_cast<T*>(componentStorage.Components[componentStorage.EntityIndices.at(toEntity.GUID)])) = T(toEntity, params...);
			}
			else
			{
				componentStorage.EntityIndices.emplace(toEntity.GUID, componentStorage.Components.size());
				componentStorage.Components.emplace_back(new T(toEntity, params...));
			}

			return dynamic_cast<T*>(componentStorage.Components.back());
		}

		template<typename... Ts>
		std::tuple<Ts*...> AddComponents(const SEntity& toEntity)
		{
			return std::make_tuple(AddComponent<Ts>(toEntity) ...);
		}

		template<typename T>
		void RemoveComponent(const SEntity& fromEntity)
		{
			const U64 typeIDHashCode = typeid(T).hash_code();
			if (!ComponentTypeIndices.contains(typeIDHashCode))
			{
				std::string templateName = typeid(T).name();
				// HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to remove component that does not have any storage. Doing nothing instead.", templateName.c_str());
				return;
			}

			SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeIDHashCode)];

			if (!componentStorage.EntityIndices.contains(fromEntity.GUID))
			{
				std::string templateName = typeid(T).name();
				// HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to remove component that the entity with GUID: %i did not have registered. Doing nothing instead.", templateName.c_str(), fromEntity.GUID);
				return;
			}

			auto& endValue = *(--componentStorage.EntityIndices.end());
			componentStorage.EntityIndices.at(endValue.first) = componentStorage.EntityIndices.at(fromEntity.GUID);

			std::swap(componentStorage.Components[componentStorage.EntityIndices.at(fromEntity.GUID)], componentStorage.Components.back());

			SComponent*& componentToBeRemoved = componentStorage.Components.back();
			delete componentToBeRemoved;
			componentToBeRemoved = nullptr;

			componentStorage.Components.pop_back();
			componentStorage.EntityIndices.erase(fromEntity.GUID);
		}

		template<typename... Ts>
		void RemoveComponents(const SEntity& fromEntity)
		{
			([&] { RemoveComponent<Ts>(fromEntity); } (), ...);
		}

		HAVTORN_API const SEntity& AddEntity(U64 guid = 0);
		HAVTORN_API const SEntity& AddEntity(const std::string& nameInEditor, U64 guid = 0);
		HAVTORN_API void RemoveEntity(SEntity& entity);

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
			{
				std::string templateName = typeid(T).name();
				HV_LOG_TRACE(__FUNCTION__" with T=[%s]: Tried to get a component that does not have any storage. Returning nullptr.", templateName.c_str());
				return nullptr;
			}

			const SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeIDHashCode)];

			if (!componentStorage.EntityIndices.contains(fromEntity.GUID))
			{
				std::string templateName = typeid(T).name();
				HV_LOG_TRACE(__FUNCTION__" with T=[%s]: Tried to remove component that the entity with GUID: %i did not have registered. Returning nullptr.", templateName.c_str(), fromEntity.GUID);
				return nullptr;
			}

			return dynamic_cast<T*>(componentStorage.Components[componentStorage.EntityIndices.at(fromEntity.GUID)]);
		}

		template<typename T>
		T* GetComponent(const SComponent* fromOtherComponent) const
		{
			return GetComponent<T>(fromOtherComponent->EntityOwner);
		}

		template<typename... Ts>
		std::tuple<Ts*...> GetComponents(const SEntity& fromEntity) const
		{
			return std::make_tuple(GetComponent<Ts>(fromEntity) ...);
		}

		template<typename... Ts>
		std::tuple<Ts*...> GetComponents(const SComponent* fromOtherComponent) const
		{
			return std::make_tuple(GetComponent<Ts>(fromOtherComponent->EntityOwner) ...);
		}

		template<typename T>
		std::vector<T*> GetComponents() const
		{
			const U64 typeIDHashCode = typeid(T).hash_code();
			if (!ComponentTypeIndices.contains(typeIDHashCode))
			{
				std::string templateName = typeid(T).name();
				 HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to get all components of a type that does not have any storage. Returning empty vector.", templateName.c_str());
				return {};
			}

			// NR: This looks problematic but works because we know we only fill buckets with the same component type. 
			// This would be a bad idea if we kept different derived components in the same vectors.

			const SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeIDHashCode)];
			std::vector<T*> specializedComponents;
			specializedComponents.resize(componentStorage.Components.size());
			memcpy(&specializedComponents[0], componentStorage.Components.data(), sizeof(T*) * componentStorage.Components.size());

			return specializedComponents;
		}

		template<typename T>
		void AddView(const SEntity& entityOwner)
		{
			if (!ComponentViews.contains(entityOwner.GUID))
				ComponentViews.emplace(entityOwner.GUID, {});

			auto& viewMap = ComponentViews.at(entityOwner.GUID);

			const U64 typeIDHashCode = typeid(T).hash_code();
			if (viewMap.contains(typeIDHashCode))
				return;

			viewMap.emplace(typeIDHashCode, new T());
		}

		template<typename T>
		void RemoveView(const SEntity& entityOwner)
		{
			if (!ComponentViews.contains(entityOwner.GUID))
				return;

			auto& viewMap = ComponentViews.at(entityOwner.GUID);
			
			const U64 typeIDHashCode = typeid(T).hash_code();
			if (!viewMap.contains(typeIDHashCode))
				return;

			auto& view = viewMap.at(typeIDHashCode);

			delete view;
			view = nullptr;

			viewMap.erase(typeIDHashCode);
		}

		HAVTORN_API void RemoveViews(const SEntity& entityOwner);
		HAVTORN_API std::vector<struct SComponentView*> GetViews(const SEntity& entityOwner);
		
		std::unordered_map<U64, std::map<U64, struct SComponentView*>> ComponentViews;

		std::unordered_map<U64, U64> EntityIndices;
		std::vector<SEntity> Entities;

		std::unordered_map<U64, U64> ComponentTypeIndices;

		// NR: How do we serialize SComponentStorage? Maybe we need one master hash table of component types that would be the only place we need to update.
		// Then that could be separated into engine types at the top and game types at the bottom, with some spacing in between. Need to solve the serialization.
		std::vector<SComponentStorage> Storages;

		CHavtornStaticString<255> SceneName = std::string("SceneName");
		
		//U64 FirstUnusedEntityIndex = 0;
		SEntity MainCameraEntity = SEntity::Null;

		// TODO.NR/AG: Try to remove this
		CRenderManager* RenderManager = nullptr;


	};

	//template<class T>
	//void CScene::UpdateComponentVector(std::vector<T>& components, I64 index)
	//{
	//	I64 lastUsedEntityIndex = FirstUnusedEntityIndex - 1;

	//	if (index < 0 || index == lastUsedEntityIndex)
	//		return;

	//	if (!components[lastUsedEntityIndex].IsInUse)
	//		return;

	//	std::swap(components[index], components[lastUsedEntityIndex]);
	//	components[lastUsedEntityIndex].IsInUse = false;
	//}
}
