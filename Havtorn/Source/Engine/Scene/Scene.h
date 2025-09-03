// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <HavtornString.h>
#include <HavtornDelegate.h>
#include <GeneralUtilities.h>
#include "ECS/Entity.h"
#include "ECS/Component.h"
#include "ECS/ComponentEditorContext.h"

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

	class CAssetRegistry;

	class CScene
	{
	public:
		ENGINE_API CScene();
		ENGINE_API ~CScene();

		ENGINE_API virtual bool Init(const std::string& sceneName);
		ENGINE_API virtual bool Init3DDefaults();
		ENGINE_API virtual bool Init3DDemoScene();
		ENGINE_API virtual bool Init2DDemoScene();

		// TODO.NR: Rework serialization to decrease amount of boilerplate
		ENGINE_API virtual [[nodiscard]] U32 GetSize() const;
		ENGINE_API virtual void Serialize(char* toData, U64& pointerPosition) const;
		ENGINE_API virtual void Deserialize(const char* fromData, U64& pointerPosition);

		ENGINE_API std::string GetSceneName() const;
		ENGINE_API U64 GetSceneIndex(const SEntity& entity) const;
		ENGINE_API U64 GetSceneIndex(const U64 entityGUID) const;
		
		CMulticastDelegate<SEntity> OnEntityPreDestroy;

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
		void DefaultDeserializer(SComponentEditorContext* context, const char* fromData, U64& pointerPosition)
		{
			U32 numberOfComponents = 0;
			DeserializeData(numberOfComponents, fromData, pointerPosition);

			for (U64 index = 0; index < numberOfComponents; index++)
			{
				T component;
				DeserializeData(component, fromData, pointerPosition);
				AddComponent(component, component.Owner);
				AddComponentEditorContext(component.Owner, context);
			}
		}

		template<typename T>
		void SpecializedDeserializer(SComponentEditorContext* context, const char* fromData, U64& pointerPosition)
		{
			U32 numberOfComponents = 0;
			DeserializeData(numberOfComponents, fromData, pointerPosition);

			for (U64 index = 0; index < numberOfComponents; index++)
			{
				T component;
				component.Deserialize(fromData, pointerPosition);
				AddComponent(component, component.Owner);
				AddComponentEditorContext(component.Owner, context);
			}
		}

		template<typename T>
		void RegisterComponent(U32 startingNumberOfInstances, SComponentEditorContext* context)
		{
			const U64 typeIDHashCode = typeid(T).hash_code();
			if (ContextIndices.contains(typeIDHashCode))
			{
				HV_LOG_WARN("%s is already registered in the %s scene.", typeid(T).name(), SceneName.AsString().c_str());
				return;
			}

			if (ComponentTypeIndices.contains(typeIDHashCode))
			{
				ComponentTypeIndices.emplace(typeIDHashCode, Storages.size());
				Storages.emplace_back();
				Storages.back().Components.resize(startingNumberOfInstances, nullptr);
			}

			ContextIndices.emplace(typeIDHashCode, RegisteredComponentEditorContexts.size());
			RegisteredComponentEditorContexts.emplace_back(context);

			std::sort(RegisteredComponentEditorContexts.begin(), RegisteredComponentEditorContexts.end(), [](const SComponentEditorContext* a, const SComponentEditorContext* b) { return a->GetSortingPriority() < b->GetSortingPriority(); });
		}

		template<typename T>
		const SComponentEditorContext& GetEditorContext() const
		{
			const U64 typeIDHashCode = typeid(T).hash_code();
			if (ContextIndices.contains(typeIDHashCode))
			{
				HV_LOG_WARN("% s is not registered in the %s scene. Returning an empty editor context.", typeid(T).name(), SceneName.AsString().c_str());
				return SComponentEditorContext();
			}

			return RegisteredComponentEditorContexts[ContextIndices.at(typeIDHashCode)];
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
			std::unordered_map<U64, U64>& entityIndices = componentStorage.EntityIndices;
			std::vector<SComponent*>& components = componentStorage.Components;

			if (!entityIndices.contains(fromEntity.GUID))
			{
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				// HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to remove component that the entity with GUID: %i did not have registered. Doing nothing instead.", templateName.c_str(), fromEntity.GUID);
				return;
			}

			const std::pair<U64, U64>& maxIndexEntry = *std::ranges::find_if(entityIndices, 
				[components](const auto& entry) { return entry.second == components.size() - 1; });

			std::swap(components[entityIndices.at(fromEntity.GUID)], components[entityIndices.at(maxIndexEntry.first)]);
			std::swap(entityIndices.at(maxIndexEntry.first), entityIndices.at(fromEntity.GUID));

			T* componentToBeRemoved = reinterpret_cast<T*>(components[entityIndices.at(fromEntity.GUID)]);
			componentToBeRemoved->IsDeleted(this);
			delete componentToBeRemoved;
			componentToBeRemoved = nullptr;

			components.erase(components.begin() + entityIndices.at(fromEntity.GUID));
			entityIndices.erase(fromEntity.GUID);
		}

		template<typename... Ts>
		void RemoveComponents(const SEntity& fromEntity)
		{
			([&] { RemoveComponent<Ts>(fromEntity); } (), ...);
		}

		ENGINE_API const SEntity& AddEntity(U64 guid = 0);
		ENGINE_API const SEntity& AddEntity(const std::string& nameInEditor, U64 guid = 0);
		ENGINE_API void RemoveEntity(const SEntity entity);
		ENGINE_API void ClearScene();

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

			U64 index = componentStorage.EntityIndices.at(fromEntity.GUID);
			return dynamic_cast<T*>(componentStorage.Components[index]);
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
			if (componentStorage.Components.empty())
				return {};

			std::vector<T*> specializedComponents;
			specializedComponents.resize(componentStorage.Components.size());
			memcpy(&specializedComponents[0], componentStorage.Components.data(), sizeof(T*) * componentStorage.Components.size());

			return specializedComponents;
		}
		
		template<typename T>
		std::vector<SComponent*> GetBaseComponents()
		{
			const U64 typeIDHashCode = typeid(T).hash_code();
			if (!ComponentTypeIndices.contains(typeIDHashCode))
			{
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				//HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to get all components of a type that does not have any storage. Returning empty vector.", templateName.c_str());
				return {};
			}

			const SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeIDHashCode)];
			return componentStorage.Components;
		}

		ENGINE_API void AddComponentEditorContext(const SEntity& owner, SComponentEditorContext* context);
		ENGINE_API void RemoveComponentEditorContext(const SEntity& owner, SComponentEditorContext* context);

		ENGINE_API void RemoveComponentEditorContexts(const SEntity& owner);
		ENGINE_API std::vector<SComponentEditorContext*> GetComponentEditorContexts(const SEntity& owner);

		// TODO.NW: Move this to World, so that it persists over scenes. Same as with Nodes
		ENGINE_API const std::vector<SComponentEditorContext*>& GetComponentEditorContexts() const;
	
		std::unordered_map<U64, std::vector<SComponentEditorContext*>> EntityComponentEditorContexts;

		std::unordered_map<U64, U64> EntityIndices;
		std::vector<SEntity> Entities;

		std::unordered_map<U64, U64> ComponentTypeIndices;
		std::vector<SComponentStorage> Storages;

		std::unordered_map<U64, U64> ContextIndices;
		std::vector<SComponentEditorContext*> RegisteredComponentEditorContexts;

		CHavtornStaticString<255> SceneName = std::string("SceneName");

		SEntity MainCameraEntity = SEntity::Null;
		SEntity PreviewEntity = SEntity::Null;
	};
}
