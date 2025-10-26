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

	struct SComponentSerializer
	{
		std::function<U32(const CScene*)> SizeAllocator;
		std::function<void(const CScene*, char*, U64&)> Serializer;
		std::function<void(CScene*, const char*, U64&)> Deserializer;
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

		ENGINE_API virtual [[nodiscard]] U32 GetSize() const;
		ENGINE_API virtual void Serialize(char* toData, U64& pointerPosition) const;
		ENGINE_API virtual void Deserialize(const char* fromData, U64& pointerPosition);

		ENGINE_API std::string GetSceneName() const;
		
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
		void DefaultSerializer(const std::vector<T*>& componentVector, char* toData, U64& pointerPosition) const
		{
			SerializeData(STATIC_U32(componentVector.size()), toData, pointerPosition);
			for (const auto component : componentVector)
			{
				auto& componentRef = *component;
				SerializeData(componentRef, toData, pointerPosition);
			}
		}

		template<typename TComponent, typename TComponentEditorContext>
		void RegisterTrivialComponent(U32 typeID, U32 startingNumberOfInstances = 10)
		{
			if (ContextIndices.contains(typeID))
			{
				//HV_LOG_WARN("%s is already registered in the %s scene.", typeid(T).name(), SceneName.AsString().c_str());
				return;
			}

			if (ComponentTypeIndices.contains(typeID))
			{
				ComponentTypeIndices.emplace(typeID, Storages.size());
				Storages.emplace_back();
				Storages.back().Components.resize(startingNumberOfInstances, nullptr);
			}

			ContextIndices.emplace(typeID, RegisteredComponentEditorContexts.size());
			RegisteredComponentEditorContexts.emplace_back(&TComponentEditorContext::Context);

			std::sort(RegisteredComponentEditorContexts.begin(), RegisteredComponentEditorContexts.end(), [](const SComponentEditorContext* a, const SComponentEditorContext* b) { return a->GetSortingPriority() < b->GetSortingPriority(); });

			TypeHashToTypeID.emplace(typeid(TComponent).hash_code(), typeID);

			SComponentSerializer serializer;
			serializer.SizeAllocator =
				[](const CScene* scene)
				{
					const std::vector<TComponent*>& componentVector = scene->GetComponents<TComponent>();

					U32 size = 0;

					// Type ID and blob size
					size += GetDataSize(U32());
					size += GetDataSize(U32());

					size += GetDataSize(STATIC_U32(componentVector.size()));
					for (const auto component : componentVector)
					{
						auto& componentRef = *component;
						size += GetDataSize(componentRef);
					}
					return size;
				};

			serializer.Serializer =
				[](const CScene* scene, char* toData, U64& pointerPosition)
				{
					const std::vector<TComponent*>& componentVector = scene->GetComponents<TComponent>();

					// Scene header
					SerializeData(scene->TypeHashToTypeID.at(typeid(TComponent).hash_code()), toData, pointerPosition);
					U32 size = 0;
					size += GetDataSize(STATIC_U32(componentVector.size()));
					for (const auto component : componentVector)
					{
						auto& componentRef = *component;
						size += GetDataSize(componentRef);
					}
					SerializeData(size, toData, pointerPosition);
					// !Scene header

					SerializeData(STATIC_U32(componentVector.size()), toData, pointerPosition);
					for (const auto component : componentVector)
					{
						auto& componentRef = *component;
						SerializeData(componentRef, toData, pointerPosition);
					}
				};

			serializer.Deserializer =
				[](CScene* scene, const char* fromData, U64& pointerPosition)
				{
					U32 numberOfComponents = 0;
					DeserializeData(numberOfComponents, fromData, pointerPosition);

					for (U64 index = 0; index < numberOfComponents; index++)
					{
						TComponent component;
						DeserializeData(component, fromData, pointerPosition);
						scene->AddComponent(component, component.Owner);
						scene->AddComponentEditorContext(component.Owner, &TComponentEditorContext::Context);
					}
				};				
			
			ComponentSerializers[typeID] = serializer;
		}

		template<typename TComponent, typename TComponentEditorContext>
		void RegisterNonTrivialComponent(U32 typeID, U32 startingNumberOfInstances = 10)
		{
			if (ContextIndices.contains(typeID))
			{
				//HV_LOG_WARN("%s is already registered in the %s scene.", typeid(T).name(), SceneName.AsString().c_str());
				return;
			}

			if (ComponentTypeIndices.contains(typeID))
			{
				ComponentTypeIndices.emplace(typeID, Storages.size());
				Storages.emplace_back();
				Storages.back().Components.resize(startingNumberOfInstances, nullptr);
			}

			ContextIndices.emplace(typeID, RegisteredComponentEditorContexts.size());
			RegisteredComponentEditorContexts.emplace_back(&TComponentEditorContext::Context);

			std::sort(RegisteredComponentEditorContexts.begin(), RegisteredComponentEditorContexts.end(), [](const SComponentEditorContext* a, const SComponentEditorContext* b) { return a->GetSortingPriority() < b->GetSortingPriority(); });

			SComponentSerializer serializer;
			serializer.SizeAllocator =
				[](const CScene* scene)
				{
					const std::vector<TComponent*>& componentVector = scene->GetComponents<TComponent>();

					U32 size = 0;

					// Type ID and blob size
					size += GetDataSize(U32());
					size += GetDataSize(U32());

					size += GetDataSize(STATIC_U32(componentVector.size()));
					for (auto component : componentVector)
						size += component->GetSize();

					return size;
				};

			serializer.Serializer =
				[](const CScene* scene, char* toData, U64& pointerPosition)
				{
					const std::vector<TComponent*>& componentVector = scene->GetComponents<TComponent>();

					// Scene header
					SerializeData(scene->TypeHashToTypeID.at(typeid(TComponent).hash_code()), toData, pointerPosition);
					U32 size = 0;
					size += GetDataSize(STATIC_U32(componentVector.size()));
					for (auto component : componentVector)
						size += component->GetSize();
					SerializeData(size, toData, pointerPosition);
					// !Scene header

					SerializeData(STATIC_U32(componentVector.size()), toData, pointerPosition);
					for (auto component : componentVector)
						component->Serialize(toData, pointerPosition);
				};

			serializer.Deserializer =
				[](CScene* scene, const char* fromData, U64& pointerPosition)
				{
					U32 numberOfComponents = 0;
					DeserializeData(numberOfComponents, fromData, pointerPosition);

					for (U64 index = 0; index < numberOfComponents; index++)
					{
						TComponent component;
						component.Deserialize(fromData, pointerPosition);
						scene->AddComponent(component, component.Owner);
						scene->AddComponentEditorContext(component.Owner, &TComponentEditorContext::Context);
					}
				};
			
			TypeHashToTypeID.emplace(typeid(TComponent).hash_code(), typeID);
			ComponentSerializers[typeID] = serializer;
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
			const U32 typeID = TypeHashToTypeID.at(typeid(T).hash_code());
			if (!ComponentTypeIndices.contains(typeID))
			{
				ComponentTypeIndices.emplace(typeID, Storages.size());
				Storages.emplace_back();
			}

			SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeID)];

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
			const U32 typeID = TypeHashToTypeID.at(typeid(T).hash_code());
			if (!ComponentTypeIndices.contains(typeID))
			{
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				// HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to remove component that does not have any storage. Doing nothing instead.", templateName.c_str());
				return;
			}

			SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeID)];
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
			const U32 typeID = TypeHashToTypeID.at(typeid(T).hash_code());
			if (!ComponentTypeIndices.contains(typeID))
			{
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				//HV_LOG_TRACE(__FUNCTION__" with T=[%s]: Tried to get a component that does not have any storage. Returning nullptr.", templateName.c_str());
				return nullptr;
			}

			const SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeID)];

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
			const U32 typeID = TypeHashToTypeID.at(typeid(T).hash_code());
			if (!ComponentTypeIndices.contains(typeID))
			{
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				//HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to get all components of a type that does not have any storage. Returning empty vector.", templateName.c_str());
				return {};
			}

			// NR: This looks problematic but works because we know we only fill buckets with the same component type. 
			// This would be a bad idea if we kept different derived components in the same vectors.

			const SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeID)];
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
			const U32 typeID = TypeHashToTypeID.at(typeid(T).hash_code());
			if (!ComponentTypeIndices.contains(typeID))
			{
				// TODO.NR: Make a toggle for this, keep for now
				//std::string templateName = typeid(T).name();
				//HV_LOG_WARN(__FUNCTION__" with T=[%s]: Tried to get all components of a type that does not have any storage. Returning empty vector.", templateName.c_str());
				return {};
			}

			const SComponentStorage& componentStorage = Storages[ComponentTypeIndices.at(typeID)];
			return componentStorage.Components;
		}

		ENGINE_API void AddComponentEditorContext(const SEntity& owner, SComponentEditorContext* context);
		ENGINE_API void RemoveComponentEditorContext(const SEntity& owner, SComponentEditorContext* context);

		ENGINE_API void RemoveComponentEditorContexts(const SEntity& owner);
		ENGINE_API std::vector<SComponentEditorContext*> GetComponentEditorContexts(const SEntity& owner);

		// TODO.NW: Move this to World, so that it persists over scenes. Same as with Nodes
		ENGINE_API const std::vector<SComponentEditorContext*>& GetComponentEditorContexts() const;
	
		std::unordered_map<U32, SComponentSerializer> ComponentSerializers;
		std::unordered_map<U64, U32> TypeHashToTypeID;

		std::unordered_map<U64, std::vector<SComponentEditorContext*>> EntityComponentEditorContexts;

		std::unordered_map<U64, U64> EntityIndices;
		std::vector<SEntity> Entities;

		std::unordered_map<U32, U64> ComponentTypeIndices;
		std::vector<SComponentStorage> Storages;

		std::unordered_map<U32, U64> ContextIndices;
		std::vector<SComponentEditorContext*> RegisteredComponentEditorContexts;

		CHavtornStaticString<255> SceneName = std::string("SceneName");

		SEntity MainCameraEntity = SEntity::Null;
		SEntity PreviewEntity = SEntity::Null;
	};
}
