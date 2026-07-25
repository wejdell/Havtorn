// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Scene.h"

#include "ECS/ECSInclude.h"
#include "ECS/GUIDManager.h"
#include "Graphics/RenderManager.h"
#include "Assets/AssetFileHeader.h"
#include "World.h"
#include "Assets/AssetRegistry.h"

#include "../Game/GameScript.h"

#include <algorithm>

namespace Havtorn
{
	U64 CScene::MainMenuEntityGUID = std::hash<std::string>{}("MainMenu");

	CScene::CScene()
	{}

	CScene::~CScene()
	{
		ClearScene();
	}

	bool CScene::Init(const std::string& sceneName)
	{
		if (!sceneName.empty())
			SceneName = sceneName;

		TypeHashToTypeID.emplace(typeid(SMetaDataComponent).hash_code(), 0);
		RegisterNonTrivialComponent<STransformComponent>(10, 50);
		RegisterNonTrivialComponent<SStaticMeshComponent>(20, 40);
		RegisterNonTrivialComponent<SSkeletalMeshComponent>(30, 40);
		RegisterTrivialComponent<SCameraComponent>(40, 2);
		RegisterTrivialComponent<SCameraControllerComponent>(50, 2);
		RegisterNonTrivialComponent<SMaterialComponent>(60, 40);
		RegisterNonTrivialComponent<SEnvironmentLightComponent>(70, 1);
		RegisterTrivialComponent<SDirectionalLightComponent>(80, 1);
		RegisterTrivialComponent<SPointLightComponent>(90, 1);
		RegisterTrivialComponent<SSpotLightComponent>(100, 1);
		RegisterTrivialComponent<SVolumetricLightComponent>(110, 3);
		RegisterNonTrivialComponent<SDecalComponent>(120, 2);
		RegisterNonTrivialComponent<SSpriteComponent>(130, 10);
		RegisterTrivialComponent<STransform2DComponent>(140, 10);
		RegisterNonTrivialComponent<SSpriteAnimatorGraphComponent>(150, 2);
		RegisterNonTrivialComponent<SSkeletalAnimationComponent>(160, 40);
		RegisterNonTrivialComponent<SScriptComponent>(170, 10);
		RegisterTrivialComponent<SPhysics2DComponent>(180, 10);
		RegisterTrivialComponent<SPhysics3DComponent>(190, 40);
		RegisterTrivialComponent<SPhysics3DControllerComponent>(200, 1);
		RegisterNonTrivialComponent<SUICanvasComponent>(210, 5);
		RegisterNonTrivialComponent<SLevelStreamingComponent>(220, 10);
		RegisterNonTrivialComponent<SPrefabComponent>(230, 10);
		RegisterNonTrivialComponent<SAbilityComponent>(240, 10);
		RegisterNonTrivialComponent<SHexCommandComponent>(250, 10);
		RegisterNonTrivialComponent<SInputComponent>(260, 10);
		RegisterTrivialComponent<SAudioListenerComponent>(270, 1);
		RegisterNonTrivialComponent<SAudioEmitterComponent>(280, 20);
		//RegisterTrivialComponent<SSequencerComponent>(typeID++, 0);

		return true;
	}

	bool CScene::Init3DDefaults()
	{
		// === Camera ===
		SEntity mainCamera = AddEntity("Camera");

		if (!mainCamera.IsValid())
			return false;

		// Setup entities (create components)
		{
			STransformComponent& transform = *AddComponent<STransformComponent>(mainCamera);
			
			transform.Transform.Translate({ 2.5f, 1.0f, -3.5f });
			transform.Transform.Rotate({ 0.0f, UMath::DegToRad(35.0f), 0.0f });
			transform.Transform.Translate(SVector::Right * 0.25f);

			SCameraComponent& camera = *AddComponent<SCameraComponent>(mainCamera);
			camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 1000.0f);
			camera.IsActive = true;

			SCameraControllerComponent& controllerComp = *AddComponent<SCameraControllerComponent>(mainCamera);
			
			SVector currentEuler = transform.Transform.GetMatrix().GetEuler();
			controllerComp.CurrentPitch = UMath::Clamp(currentEuler.X, -SCameraControllerComponent::MaxPitchDegrees + 0.01f, SCameraControllerComponent::MaxPitchDegrees - 0.01f);;
			controllerComp.CurrentYaw = UMath::WrapAngle(currentEuler.Y);
		}
		// === !Camera ===

		// === Game Camera ===
		{
			SEntity gameCamera = AddEntity("Game Camera");

			// Setup entities (create components)
			STransformComponent& transform = *AddComponent<STransformComponent>(gameCamera);
			
			transform.Transform.Translate({ 2.6f, 1.0f, -1.6f });
			transform.Transform.Rotate({ 0.0f, UMath::DegToRad(70.0f), 0.0f });

			SCameraComponent& camera = *AddComponent<SCameraComponent>(gameCamera);
			camera.ProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 6.0f);
			camera.IsStartingCamera = true;
			camera.FarClip = 6.0f;

			SCameraControllerComponent& controllerComp = *AddComponent<SCameraControllerComponent>(gameCamera);
						
			SVector currentEuler = transform.Transform.GetMatrix().GetEuler();
			controllerComp.CurrentPitch = UMath::Clamp(currentEuler.X, -SCameraControllerComponent::MaxPitchDegrees + 0.01f, SCameraControllerComponent::MaxPitchDegrees - 0.01f);;
			controllerComp.CurrentYaw = UMath::WrapAngle(currentEuler.Y);
		}

		// === Environment light ===
		const SEntity& environmentLightEntity = AddEntity("Environment Light");
		if (!environmentLightEntity.IsValid())
			return false;

		AddComponent<STransformComponent>(environmentLightEntity);
		AddComponent<SEnvironmentLightComponent>(environmentLightEntity, "Resources/DefaultSkybox.hva");
		// === !Environment light ===

		// === Directional light ===
		const SEntity& directionalLightEntity = AddEntity("Directional Light");
		if (!directionalLightEntity.IsValid())
			return false;

		// NR: Add transform to directional light so it can filter environmental lights based on distance
		AddComponent<STransformComponent>(directionalLightEntity);
		
		SDirectionalLightComponent& directionalLight = *AddComponent<SDirectionalLightComponent>(directionalLightEntity);
		directionalLight.Direction = { -1.0f, -1.0f, 1.0f, 0.0f };
		directionalLight.ShadowmapView.ShadowmapViewportIndex = 0;
		directionalLight.ShadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(directionalLight.ShadowViewSize.X, directionalLight.ShadowViewSize.Y, directionalLight.ShadowNearAndFarPlane.X, directionalLight.ShadowNearAndFarPlane.Y);

		SVolumetricLightComponent& volumetricLight = *AddComponent<SVolumetricLightComponent>(directionalLightEntity);
		volumetricLight.IsActive = false;
		// === !Directional light ===

		return true;
	}

	void CScene::OpenDefault()
	{
		if (!Init("3DDemoScene"))
			return;

		Init3DDefaults();
	}

	U32 CScene::GetSize() const
	{
		U32 size = 0;

		// Scene File Size
		size += GetDataSize(U32());

		size += GetDataSize(SceneName);
		size += GetDataSize(Entities);

		// MetaData typeID and blob size 
		size += GetDataSize(U32());
		size += GetDataSize(U32());
		size += DefaultSizeAllocator(GetComponents<SMetaDataComponent>());

		for (auto& [key, val] : ComponentSerializers)
		{
			size += val.SizeAllocator(this);
		}

		return size;
	}

	void CScene::Serialize(char* toData, U64& pointerPosition) const
	{
		// Make sure to add pointerPosition here, to take outer data members into account
		SerializeData(GetSize() + STATIC_U32(pointerPosition), toData, pointerPosition);

		SerializeData(SceneName, toData, pointerPosition);
		SerializeData(Entities, toData, pointerPosition);

		U32 typeID = TypeHashToTypeID.at(typeid(SMetaDataComponent).hash_code());
		SerializeData(typeID, toData, pointerPosition);
		U32 size = DefaultSizeAllocator(GetComponents<SMetaDataComponent>());
		SerializeData(size, toData, pointerPosition);
		DefaultSerializer(GetComponents<SMetaDataComponent>(), toData, pointerPosition);

		for (auto& [key, val] : ComponentSerializers)
		{
			val.Serializer(this, toData, pointerPosition);
		}
	}

	void CScene::Deserialize(const char* fromData, U64& pointerPosition)
	{
		U32 sceneFileSize = 0;
		DeserializeData(sceneFileSize, fromData, pointerPosition);

		DeserializeData(SceneName, fromData, pointerPosition);
		
		std::vector<SEntity> entities;
		DeserializeData(entities, fromData, pointerPosition);

		for (SEntity& entity : entities)
			AddEntity(entity.GUID);

		{
			U32 savedTypeID = 0;
			DeserializeData(savedTypeID, fromData, pointerPosition);
			U32 savedBlobSize = 0;
			DeserializeData(savedBlobSize, fromData, pointerPosition);

			U32 metaDataTypeID = UMath::MaxU32;
			if (TypeHashToTypeID.contains(typeid(SMetaDataComponent).hash_code()))
				metaDataTypeID = TypeHashToTypeID.at(typeid(SMetaDataComponent).hash_code());

			if (metaDataTypeID == savedTypeID) 
			{
				std::vector<SMetaDataComponent> componentVector;
				U32 numberOfComponents = 0;
				DeserializeData(numberOfComponents, fromData, pointerPosition);
				componentVector.resize(numberOfComponents);

				for (U64 index = 0; index < numberOfComponents; index++)
				{
					SMetaDataComponent component;
					DeserializeData(component, fromData, pointerPosition);
					AddComponent(component, component.Owner);
				}
			}
			else
			{
				pointerPosition += savedBlobSize;
			}
		}

		while (pointerPosition < sceneFileSize)
		{
			U32 savedTypeID = 0;
			DeserializeData(savedTypeID, fromData, pointerPosition);
			U32 savedBlobSize = 0;
			DeserializeData(savedBlobSize, fromData, pointerPosition);

			if (!ComponentSerializers.contains(savedTypeID))
			{
				pointerPosition += savedBlobSize;
				continue;
			} 
			// TODO.NW: Add check in VersioningService, which can take the blob and convert it from an old version to a new one

			ComponentSerializers.at(savedTypeID).Deserializer(this, fromData, pointerPosition);
		}

		 // Post pass to set up inter-entity connections
		for (STransformComponent* transformComponent : GetComponents<STransformComponent>())
		{
			for (const SEntity& serializationAttachedEntity : transformComponent->AttachedEntities)
				transformComponent->Attach(GetComponent<STransformComponent>(serializationAttachedEntity));
		}
	}

	U32 CScene::GetEntitySize(const SEntity& entity) const
	{
		U32 size = 0;

		size += GetDataSize(U32());
		size += GetDataSize(SEntity());
		
		size += GetDataSize(U32()); // Metadata type ID and component size
		size += GetDataSize(U32());
		if (SMetaDataComponent* metaData = GetComponent<SMetaDataComponent>(entity))
			size += GetDataSize(*metaData);
		
		for (auto& [key, val] : ComponentSerializers)
		{
			size += val.SingleSizeAllocator(entity, this);
		}
		return size;
	}

	void CScene::SerializeEntity(const SEntity& entity, char* toData) const
	{
		U64 pointerPosition = 0;

		SerializeData(GetEntitySize(entity), toData, pointerPosition);
		SerializeData(entity, toData, pointerPosition);

		const U32 metaDataTypeID = TypeHashToTypeID.at(typeid(SMetaDataComponent).hash_code());
		U32 metaDataComponentSize = 0;
		if (SMetaDataComponent* metaData = GetComponent<SMetaDataComponent>(entity))
			metaDataComponentSize = GetDataSize(*metaData);
		SerializeData(metaDataTypeID, toData, pointerPosition);
		SerializeData(metaDataComponentSize, toData, pointerPosition);
		if (SMetaDataComponent* metaData = GetComponent<SMetaDataComponent>(entity))
			SerializeData(*metaData, toData, pointerPosition);

		for (auto& [key, val] : ComponentSerializers)
		{
			val.SingleSerializer(entity, this, toData, pointerPosition);
		}
	}

	SEntity CScene::DeserializeEntity(const char* fromData, const bool makeUnique)
	{
		U64 pointerPosition = 0;

		U32 bufferSize = 0;
		DeserializeData(bufferSize, fromData, pointerPosition);

		SEntity entity = SEntity::Null;
		DeserializeData(entity, fromData, pointerPosition);
	
		if (makeUnique)
			entity = AddEntity();
		else
			AddEntity(entity.GUID);

		{
			U32 savedTypeID = 0;
			DeserializeData(savedTypeID, fromData, pointerPosition);
			U32 savedBlobSize = 0;
			DeserializeData(savedBlobSize, fromData, pointerPosition);

			U32 metaDataTypeID = UMath::MaxU32;
			if (TypeHashToTypeID.contains(typeid(SMetaDataComponent).hash_code()))
				metaDataTypeID = TypeHashToTypeID.at(typeid(SMetaDataComponent).hash_code());

			if (metaDataTypeID == savedTypeID)
			{
				SMetaDataComponent component;
				DeserializeData(component, fromData, pointerPosition);
				SMetaDataComponent* metaData = AddComponent(component, entity);
					
				if (makeUnique)
				{
					std::string newEntityName = UGeneralUtils::GetNonCollidingString(metaData->Name.AsString(), Entities, [this](const SEntity& entity)
						{
							const SMetaDataComponent* metaDataComp = GetComponent<SMetaDataComponent>(entity);
							return SComponent::IsValid(metaDataComp) ? metaDataComp->Name.AsString() : "UNNAMED";
						}
					);
					metaData->Name = newEntityName;
				}
			}
			else
			{
				pointerPosition += savedBlobSize;
			}
		}

		while (pointerPosition < bufferSize)
		{
			U32 savedTypeID = 0;
			DeserializeData(savedTypeID, fromData, pointerPosition);
			U32 savedBlobSize = 0;
			DeserializeData(savedBlobSize, fromData, pointerPosition);

			if (!ComponentSerializers.contains(savedTypeID))
			{
				pointerPosition += savedBlobSize;
				continue;
			}
			// TODO.NW: Add check in VersioningService, which can take the blob and convert it from an old version to a new one

			ComponentSerializers.at(savedTypeID).SingleDeserializer(entity, this, fromData, pointerPosition);
		}

		return entity;
	}

	std::string CScene::GetSceneName() const
	{
		return SceneName.AsString();
	}

	const SEntity& CScene::AddEntity(U64 guid)
	{
		if (EntityIndices.contains(guid))
		{
			HV_LOG_WARN("__FUNC__: Tried to add entity with GUID: %i that already exists. Returning existing Entity.", guid);
			return Entities[EntityIndices.at(guid)];
		}

		SEntity newEntity = { guid };
		if (!newEntity.IsValid())
			newEntity.GUID = UGUIDManager::Generate();

		EntityIndices.emplace(newEntity.GUID, Entities.size());
		Entities.push_back(newEntity);
		EntityComponentRuntimeHashes.emplace(newEntity.GUID, std::vector<U64>{});

		return Entities.back();
	}

	const SEntity& CScene::AddEntity(const std::string& nameInEditor, U64 guid)
	{
		const SEntity& outEntity = AddEntity(guid);
		AddComponent<SMetaDataComponent>(outEntity, nameInEditor);

		return outEntity;
	}

	bool CScene::HasEntity(U64 guid) const
	{
		return EntityIndices.contains(guid);
	}

	void CScene::RemoveEntity(const SEntity entity)
	{
		if (!entity.IsValid())
		{
			HV_LOG_ERROR("__FUNCTION__: Tried to remove an invalid Entity.");
			return;
		}

		if (!HasEntity(entity.GUID))
		{
			HV_LOG_ERROR("__FUNCTION__: Tried to remove entity with GUID: %u from a scene that does not contain it.", entity.GUID);
			return;
		}

		OnEntityPreDestroy.Broadcast(entity);

		auto removeEntityComponentFromStorage = [&](SComponentStorage& storage, const U64 entityGUID)
		{
			if (!storage.EntityIndices.contains(entityGUID))
				return;
			
			SComponent*& componentToBeRemoved = storage.Components.back();
			if (componentToBeRemoved != nullptr)
			{
				storage.EntityIndices.at(componentToBeRemoved->Owner.GUID) = storage.EntityIndices.at(entityGUID);

				std::swap(storage.Components[storage.EntityIndices.at(entityGUID)], storage.Components.back());

				componentToBeRemoved->IsDeleted(this);
				delete componentToBeRemoved;
				componentToBeRemoved = nullptr;
			}

			storage.Components.pop_back();
			storage.EntityIndices.erase(entityGUID);
		};

		// TODO.NW: Can't we call RemoveComponent instead of doing this work manually?

		// TODO.NW: Build dependency graph to go through storages. Prefab components must be handled before Transforms for example
		const U32 typeID = TypeHashToTypeID.at(typeid(SPrefabComponent).hash_code());
		if (ComponentTypeIndices.contains(typeID)) 
		{
			SComponentStorage& prefabComponentStorage = Storages[ComponentTypeIndices.at(typeID)];
			removeEntityComponentFromStorage(prefabComponentStorage, entity.GUID);
		}

		for (SComponentStorage& storage : Storages)
		{
			removeEntityComponentFromStorage(storage, entity.GUID);
		}

		SEntity& entityAtBack = Entities.back();
		EntityIndices.at(entityAtBack.GUID) = EntityIndices.at(entity.GUID);

		std::swap(Entities[EntityIndices.at(entity.GUID)], entityAtBack);

		Entities.pop_back();
		EntityIndices.erase(entity.GUID);
	}

	void CScene::ClearScene()
	{
		std::vector<SEntity> copy = Entities;
		for (const SEntity entity : copy)
			RemoveEntity(entity);
	}

	void CScene::MoveEntityToScene(const SEntity& entity, CScene* fromScene)
	{
		if (!entity.IsValid() || fromScene == nullptr)
		{
			if (entity.IsValid())
				HV_LOG_ERROR("CScene::MoveEntityToScene: Could not move entity %ull to %s from other scene.", entity.GUID, SceneName.AsString().c_str());
			else
				HV_LOG_ERROR("CScene::MoveEntityToScene: Could not move entity to %s from other scene.", SceneName.AsString().c_str());

			return;
		}

		if (fromScene == this)
			return;

		if (HasEntity(entity.GUID))
		{
			HV_LOG_ERROR("CScene::MoveEntityToScene: %s already has entity %ull", SceneName.AsString().c_str(), entity.GUID);
			return;
		}

		std::vector<SEntity> entitiesToMove;
		
		// TODO.NW: Should this be the default behavior? Not sure. If so, make sure to remove all attached entities from old scene (PrefabComponent logic deals with this now).
		if (SPrefabComponent* prefabComponent = fromScene->GetComponent<SPrefabComponent>(entity))
			fromScene->GetAttachedEntities(entity, entitiesToMove);
		else
			entitiesToMove = { entity };

		for (const SEntity& entityToMove : entitiesToMove)
		{
			AddEntity(entityToMove.GUID);

			if (SMetaDataComponent* metaDataComponent = fromScene->GetComponent<SMetaDataComponent>(entityToMove))
				AddComponent<SMetaDataComponent>(*metaDataComponent, entityToMove);

			for (auto& [typeID, storageIndex] : fromScene->ComponentTypeIndices)
			{
				SComponentStorage& storage = fromScene->Storages[storageIndex];
				if (!storage.EntityIndices.contains(entityToMove.GUID))
					continue;

				if (!ComponentSerializers.contains(typeID))
					continue;

				U32 size = ComponentSerializers.at(typeID).SingleSizeAllocator(entityToMove, fromScene);
				const auto data = new char[size];
				U64 pointerPosition = 0;
				ComponentSerializers.at(typeID).SingleSerializer(entityToMove, fromScene, data, pointerPosition);

				pointerPosition = 0;
				U32 typeIDInBuffer = 0;
				U32 componentSizeInBuffer = 0;
				DeserializeData(typeIDInBuffer, data, pointerPosition);
				DeserializeData(componentSizeInBuffer, data, pointerPosition);
				ComponentSerializers.at(typeID).SingleDeserializer(entityToMove, this, data, pointerPosition);

				delete[] data;
			}
		}
		
		fromScene->RemoveEntity(entity);
	}

	SEntity CScene::CopyEntity(const SEntity& fromEntity, U64 guid /*= 0*/)
	{
		std::string newEntityName = "UNNAMED";
		if (SMetaDataComponent* metaDataComponent = GetComponent<SMetaDataComponent>(fromEntity))
		{
			newEntityName = UGeneralUtils::GetNonCollidingString(metaDataComponent->Name.AsString(), Entities, [this](const SEntity& entity)
				{
					const SMetaDataComponent* metaDataComp = GetComponent<SMetaDataComponent>(entity);
					return SComponent::IsValid(metaDataComp) ? metaDataComp->Name.AsString() : "UNNAMED";
				}
			);
		}

		SEntity newEntity = AddEntity(newEntityName, guid);

		for (auto& [typeID, storageIndex] : ComponentTypeIndices)
		{
			SComponentStorage& storage = Storages[storageIndex];
			if (!storage.EntityIndices.contains(fromEntity.GUID))
				continue;

			if (!ComponentSerializers.contains(typeID))
				continue;

			U32 size = ComponentSerializers.at(typeID).SingleSizeAllocator(fromEntity, this);
			const auto data = new char[size];
			U64 pointerPosition = 0;
			ComponentSerializers.at(typeID).SingleSerializer(fromEntity, this, data, pointerPosition);

			pointerPosition = 0;
			U32 typeIDInBuffer = 0;
			U32 componentSizeInBuffer = 0;
			DeserializeData(typeIDInBuffer, data, pointerPosition);
			DeserializeData(componentSizeInBuffer, data, pointerPosition);
			ComponentSerializers.at(typeID).SingleDeserializer(newEntity, this, data, pointerPosition);

			delete[] data;
		}

		return newEntity;
	}

	std::vector<SEntity> CScene::CopyEntities(CScene* fromScene, std::vector<U64> requestedGUIDs)
	{
		if (fromScene == nullptr)
		{
			HV_LOG_ERROR("CScene::CopyEntities: Could not copy entities to %s from other scene, other scene is null.", SceneName.AsString().c_str());
			return {};
		}

		if (fromScene == this)
		{
			HV_LOG_ERROR("CScene::CopyEntities: Can not copy entities to %s from itself.", SceneName.AsString().c_str());
			return {};
		}

		// TODO.NW: Might want to figure out another way to access these, rather than returning a vector of them. For small scenes this is fine though.
		std::vector<SEntity> copiedEntities;

		// NW: This probably won't be used a lot and definitely not on tick, so it's probably ok that it's provided as a copy
		requestedGUIDs.resize(fromScene->Entities.size(), 0);

		for (U64 i = 0; i < fromScene->Entities.size(); i++)
		{
			const SEntity& otherSceneEntity = fromScene->Entities[i];

			// TODO.NW: Check this name collision resolution, doesn't seem to work.
			std::string newEntityName = "UNNAMED";
			if (SMetaDataComponent* metaDataComponent = fromScene->GetComponent<SMetaDataComponent>(otherSceneEntity))
			{
				newEntityName = UGeneralUtils::GetNonCollidingString(metaDataComponent->Name.AsString(), Entities, [this](const SEntity& entity)
					{
						const SMetaDataComponent* metaDataComp = GetComponent<SMetaDataComponent>(entity);
						return SComponent::IsValid(metaDataComp) ? metaDataComp->Name.AsString() : "UNNAMED";
					}
				);
			}

			SEntity newEntity = AddEntity(newEntityName, requestedGUIDs[i]);

			for (auto& [typeID, storageIndex] : fromScene->ComponentTypeIndices)
			{
				SComponentStorage& storage = fromScene->Storages[storageIndex];
				if (!storage.EntityIndices.contains(otherSceneEntity.GUID))
					continue;

				if (!ComponentSerializers.contains(typeID))
					continue;

				U32 size = ComponentSerializers.at(typeID).SingleSizeAllocator(otherSceneEntity, fromScene);
				const auto data = new char[size];
				U64 pointerPosition = 0;
				ComponentSerializers.at(typeID).SingleSerializer(otherSceneEntity, fromScene, data, pointerPosition);

				pointerPosition = 0;
				U32 typeIDInBuffer = 0;
				U32 componentSizeInBuffer = 0;
				DeserializeData(typeIDInBuffer, data, pointerPosition);
				DeserializeData(componentSizeInBuffer, data, pointerPosition);
				ComponentSerializers.at(typeID).SingleDeserializer(newEntity, this, data, pointerPosition);

				delete[] data;
			}

			copiedEntities.push_back(newEntity);
		}

		return copiedEntities;
	}

	std::string CScene::GetEntityStringBuffer(const SEntity& entity)
	{
		const U32 size = GetEntitySize(entity);
		
		char* buffer = new char[size];
		SerializeEntity(entity, buffer);
		std::string stringBuffer(buffer, size);
		
		delete[] buffer;

		stringBuffer.append("ENTITYBUFFER");
		return stringBuffer;
	}

	SEntity CScene::AddEntityFromStringBuffer(const std::string& buffer, const bool makeUnique)
	{
		return DeserializeEntity(buffer.data(), makeUnique);
	}

	void CScene::GetAttachedEntities(const SEntity& parentEntity, std::vector<SEntity>& outEntities) const
	{
		// TODO.NW: Deal with 2D attachment?

		const STransformComponent* transformComponent = GetComponent<STransformComponent>(parentEntity);
		if (!SComponent::IsValid(transformComponent))
			return;

		for (const SEntity& attachedEntities : transformComponent->AttachedEntities)
			GetAttachedEntities(attachedEntities, outEntities);
		
		outEntities.push_back(parentEntity);
	}
}
