// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/HavtornString.h"
#include "ECS/Entity.h"
#include "ECS/Component.h"
#include "ECS/ComponentView.h"

#include <unordered_map>
#include <map>
#include <tuple>
#include <functional>

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
		using SViewFunctionPointer = std::function<SComponentViewResult(const SEntity&, CScene*)>;

		HAVTORN_API CScene();
		HAVTORN_API ~CScene();

		bool Init(CRenderManager* renderManager, const std::string& sceneName);

		HAVTORN_API virtual bool Init3DDemoScene(CRenderManager* renderManager);
		HAVTORN_API virtual bool Init2DDemoScene(CRenderManager* renderManager);

		// TODO.NR: Rework serialization to decrease amount of boilerplate
		HAVTORN_API virtual [[nodiscard]] U32 GetSize() const;
		HAVTORN_API virtual void Serialize(char* toData, U64& pointerPosition) const;
		HAVTORN_API virtual void Deserialize(const char* fromData, U64& pointerPosition, CAssetRegistry* assetRegistry);

		HAVTORN_API std::string GetSceneName() const;
		HAVTORN_API U64 GetSceneIndex(const SEntity& entity) const;
		HAVTORN_API U64 GetSceneIndex(const U64 entityGUID) const;
		
		template<typename T>
		U32 DefaultSizeAllocator(const std::vector<T*>& componentVector) const
		{
			U32 size = 0;
			size += GetDataSize(STATIC_U32(componentVector.size()));
			for (const auto component : componentVector)
			{
				auto& componentRef = *component;
				size += GetDataSize(componentRef);
			}
			return size;
		}

		template<typename T>
		U32 SpecializedSizeAllocator(const std::vector<T*>& componentVector) const
		{
			U32 size = 0;
			size += GetDataSize(STATIC_U32(componentVector.size()));
			for (auto component : componentVector)
				size += component->GetSize();

			return size;
		}

		template<typename T>
		void DefaultSerializer(const std::vector<T*>& componentVector, char* toData, U64& pointerPosition) const
		{
			SerializeData(STATIC_U32(componentVector.size()), toData, pointerPosition);
			for (const auto component : componentVector)
			{
				auto& componentRef = *component;
				SerializeData(componentRef, toData, pointerPosition);
			}
		}

		template<typename T>
		void SpecializedSerializer(const std::vector<T*>& componentVector, char* toData, U64& pointerPosition) const
		{
			SerializeData(STATIC_U32(componentVector.size()), toData, pointerPosition);
			for (auto component : componentVector)
				component->Serialize(toData, pointerPosition);
		}

		template<typename T>
		void DefaultDeserializer(std::vector<T>& componentVector, SViewFunctionPointer viewFunction, const char* fromData, U64& pointerPosition)
		{
			U32 numberOfComponents = 0;
			DeserializeData(numberOfComponents, fromData, pointerPosition);
			componentVector.resize(numberOfComponents);

			for (U64 index = 0; index < numberOfComponents; index++)
			{
				T component;
				DeserializeData(component, fromData, pointerPosition);
				AddComponent(component, component.Owner);
				AddView(component.Owner, viewFunction);
			}
		}

		template<typename T>
		void SpecializedDeserializer(std::vector<T>& componentVector, SViewFunctionPointer viewFunction, const char* fromData, U64& pointerPosition)
		{
			U32 numberOfComponents = 0;
			DeserializeData(numberOfComponents, fromData, pointerPosition);
			componentVector.resize(numberOfComponents);

			for (U64 index = 0; index < numberOfComponents; index++)
			{
				T component;
				component.Deserialize(fromData, pointerPosition);
				AddComponent(component, component.Owner);
				AddView(component.Owner, viewFunction);
			}
		}

		template<typename T>
		T* AddComponent(const T& componentCopy, const SEntity& toEntity)
		{
			T& newComponent = (*AddComponent<T>(toEntity));
			newComponent = componentCopy;
			return &newComponent;
		}

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
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				 //HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to add component that already existed for entity with GUID : %i. Overwriting data on component.", templateName.c_str(), toEntity.GUID);
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
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				// HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to remove component that does not have any storage. Doing nothing instead.", templateName.c_str());
				return;
			}

			SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeIDHashCode)];

			if (!componentStorage.EntityIndices.contains(fromEntity.GUID))
			{
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
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
			return fromComponent->Owner;
		}

		template<typename T>
		T* GetComponent(const SEntity& fromEntity) const
		{
			const U64 typeIDHashCode = typeid(T).hash_code();
			if (!ComponentTypeIndices.contains(typeIDHashCode))
			{
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				//HV_LOG_TRACE(__FUNCTION__" with T=[%s]: Tried to get a component that does not have any storage. Returning nullptr.", templateName.c_str());
				return nullptr;
			}

			const SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeIDHashCode)];

			if (!componentStorage.EntityIndices.contains(fromEntity.GUID))
			{
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				//HV_LOG_TRACE(__FUNCTION__" with T=[%s]: Tried to remove component that the entity with GUID: %i did not have registered. Returning nullptr.", templateName.c_str(), fromEntity.GUID);
				return nullptr;
			}

			return dynamic_cast<T*>(componentStorage.Components[componentStorage.EntityIndices.at(fromEntity.GUID)]);
		}

		template<typename T>
		T* GetComponent(const SComponent* fromOtherComponent) const
		{
			return GetComponent<T>(fromOtherComponent->Owner);
		}

		template<typename... Ts>
		std::tuple<Ts*...> GetComponents(const SEntity& fromEntity) const
		{
			return std::make_tuple(GetComponent<Ts>(fromEntity) ...);
		}

		template<typename... Ts>
		std::tuple<Ts*...> GetComponents(const SComponent* fromOtherComponent) const
		{
			return std::make_tuple(GetComponent<Ts>(fromOtherComponent->Owner) ...);
		}

		template<typename T>
		std::vector<T*> GetComponents() const
		{
			const U64 typeIDHashCode = typeid(T).hash_code();
			if (!ComponentTypeIndices.contains(typeIDHashCode))
			{
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				//HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to get all components of a type that does not have any storage. Returning empty vector.", templateName.c_str());
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
		
		HAVTORN_API void AddView(const SEntity& owner, SViewFunctionPointer function);
		HAVTORN_API void RemoveView(const SEntity& owner, SViewFunctionPointer function);

		HAVTORN_API void RemoveViews(const SEntity& owner);
		HAVTORN_API std::vector<SViewFunctionPointer> GetViews(const SEntity& owner);
		
		std::unordered_map<U64, std::vector<SViewFunctionPointer>> ComponentViews;

		std::unordered_map<U64, U64> EntityIndices;
		std::vector<SEntity> Entities;

		std::unordered_map<U64, U64> ComponentTypeIndices;
		std::vector<SComponentStorage> Storages;

		CHavtornStaticString<255> SceneName = std::string("SceneName");

		SEntity MainCameraEntity = SEntity::Null;

		// TODO.NR/AG: Try to remove this
		CRenderManager* RenderManager = nullptr;
	};
}
