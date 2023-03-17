// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <unordered_map>
#include <tuple>

namespace Havtorn
{
#define COMPONENT_ADDER_DECLARATION(x) S##x& Add##x##ToEntity(SEntity& entity);
#define COMPONENT_ADDER_DEFINITION(x) S##x& CScene::Add##x##ToEntity(SEntity& entity){ entity.AddComponent(EComponentType::##x); S##x& newComponent = x##s[EntityVectorIndices[entity.GUID]]; newComponent.IsInUse = true; return newComponent;}

#define COMPONENT_REMOVER_DECLARATION(x) void Remove##x##FromEntity(SEntity& entity);
#define COMPONENT_REMOVER_DEFINITION(x) void CScene::Remove##x##FromEntity(SEntity& entity){ entity.RemoveComponent(EComponentType::##x); x##s[EntityVectorIndices[entity.GUID]].IsInUse = false;}

#define COMPONENT_VECTOR_DECLARATION(x) std::vector<S##x> x##s;

// TODO: Filter out using FirstUnusedEntityIndex? Make view instead
#define COMPONENT_VECTOR_GETTER(x) std::vector<S##x>& Get##x##s() { return x##s; }

#define ALLOCATE_COMPONENTS(x) {S##x* components = new S##x[ENTITY_LIMIT]; memmove(&##x##s[0], components, sizeof(S##x) * ENTITY_LIMIT);}

	struct SEntity;
	struct STransformComponent;
	struct SStaticMeshComponent;
	struct SCameraComponent;
	struct SCameraControllerComponent;
	struct SMaterialComponent;
	struct SEnvironmentLightComponent;
	struct SDirectionalLightComponent;
	struct SPointLightComponent;
	struct SSpotLightComponent;
	struct SVolumetricLightComponent;
	struct SDecalComponent;
	struct SDebugShapeComponent;
	struct SMetaDataComponent;

	class CRenderManager;
	class CAssetRegistry;

	static U64 gEntityGUID = 0;

	class CScene final
	{
	public:
		CScene();
		~CScene();

		bool Init(CRenderManager* renderManager);

		bool InitDemoScene(CRenderManager* renderManager);

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition, CAssetRegistry* assetRegistry);

		std::vector<SEntity>& GetEntities();
		SEntity* GetNewEntity();
		SEntity* GetNewEntity(const std::string& nameInEditor);
		bool TryRemoveEntity(SEntity& entity);

		__declspec(dllexport) U64 GetSceneIndex(const SEntity& entity) const;
		__declspec(dllexport) U64 GetMainCameraIndex() const;
		__declspec(dllexport) U64 GetNumberOfValidEntities() const;

		__declspec(dllexport) void AddComponentToEntity(EComponentType componentType, SEntity& entity);
		__declspec(dllexport) void RemoveComponentFromEntity(EComponentType componentType, SEntity& entity);

		COMPONENT_VECTOR_GETTER(TransformComponent)
		COMPONENT_VECTOR_GETTER(StaticMeshComponent)
		COMPONENT_VECTOR_GETTER(CameraComponent)
		COMPONENT_VECTOR_GETTER(CameraControllerComponent)
		COMPONENT_VECTOR_GETTER(MaterialComponent)
		COMPONENT_VECTOR_GETTER(EnvironmentLightComponent)
		COMPONENT_VECTOR_GETTER(DirectionalLightComponent)
		COMPONENT_VECTOR_GETTER(PointLightComponent)
		COMPONENT_VECTOR_GETTER(SpotLightComponent)
		COMPONENT_VECTOR_GETTER(VolumetricLightComponent)
		COMPONENT_VECTOR_GETTER(DecalComponent)
		COMPONENT_VECTOR_GETTER(DebugShapeComponent)
		COMPONENT_VECTOR_GETTER(MetaDataComponent)

		COMPONENT_ADDER_DECLARATION(TransformComponent)
		COMPONENT_ADDER_DECLARATION(StaticMeshComponent)
		COMPONENT_ADDER_DECLARATION(CameraComponent)
		COMPONENT_ADDER_DECLARATION(CameraControllerComponent)
		COMPONENT_ADDER_DECLARATION(MaterialComponent)
		COMPONENT_ADDER_DECLARATION(EnvironmentLightComponent)
		COMPONENT_ADDER_DECLARATION(DirectionalLightComponent)
		COMPONENT_ADDER_DECLARATION(PointLightComponent)
		COMPONENT_ADDER_DECLARATION(SpotLightComponent)
		COMPONENT_ADDER_DECLARATION(VolumetricLightComponent)
		COMPONENT_ADDER_DECLARATION(DecalComponent)
		COMPONENT_ADDER_DECLARATION(DebugShapeComponent)
		COMPONENT_ADDER_DECLARATION(MetaDataComponent)

		COMPONENT_REMOVER_DECLARATION(TransformComponent)
		COMPONENT_REMOVER_DECLARATION(StaticMeshComponent)
		COMPONENT_REMOVER_DECLARATION(CameraComponent)
		COMPONENT_REMOVER_DECLARATION(CameraControllerComponent)
		COMPONENT_REMOVER_DECLARATION(MaterialComponent)
		COMPONENT_REMOVER_DECLARATION(EnvironmentLightComponent)
		COMPONENT_REMOVER_DECLARATION(DirectionalLightComponent)
		COMPONENT_REMOVER_DECLARATION(PointLightComponent)
		COMPONENT_REMOVER_DECLARATION(SpotLightComponent)
		COMPONENT_REMOVER_DECLARATION(VolumetricLightComponent)
		COMPONENT_REMOVER_DECLARATION(DecalComponent)
		COMPONENT_REMOVER_DECLARATION(DebugShapeComponent)
		COMPONENT_REMOVER_DECLARATION(MetaDataComponent)

	private:
		template<class T>
		void UpdateComponentVector(std::vector<T>& components, I64 index);

	private:
		std::unordered_map<I64, U64> EntityVectorIndices;
		std::vector<SEntity> Entities;
		COMPONENT_VECTOR_DECLARATION(TransformComponent)
		COMPONENT_VECTOR_DECLARATION(StaticMeshComponent)
		COMPONENT_VECTOR_DECLARATION(CameraComponent)
		COMPONENT_VECTOR_DECLARATION(CameraControllerComponent)
		COMPONENT_VECTOR_DECLARATION(MaterialComponent)
		COMPONENT_VECTOR_DECLARATION(EnvironmentLightComponent)
		COMPONENT_VECTOR_DECLARATION(DirectionalLightComponent)
		COMPONENT_VECTOR_DECLARATION(PointLightComponent)
		COMPONENT_VECTOR_DECLARATION(SpotLightComponent)
		COMPONENT_VECTOR_DECLARATION(VolumetricLightComponent)
		COMPONENT_VECTOR_DECLARATION(DecalComponent)
		COMPONENT_VECTOR_DECLARATION(DebugShapeComponent)
		COMPONENT_VECTOR_DECLARATION(MetaDataComponent)
		U64 FirstUnusedEntityIndex = 0;
		U64 MainCameraIndex = 0;

		// TODO.NR/AG: Try to remove this
		CRenderManager* RenderManager = nullptr;
	};

	template<class T>
	void CScene::UpdateComponentVector(std::vector<T>& components, I64 index)
	{
		I64 lastUsedEntityIndex = FirstUnusedEntityIndex - 1;

		if (index < 0 || index == lastUsedEntityIndex)
			return;

		if (!components[lastUsedEntityIndex].IsInUse)
			return;

		std::swap(components[index], components[lastUsedEntityIndex]);
		components[lastUsedEntityIndex].IsInUse = false;
	}
}
