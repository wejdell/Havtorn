// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{


//#define COMPONENT_ADDER_DECLARATION(x) Ref<S##x> Add##x##ToEntity(Ref<SEntity> entity);
//#define COMPONENT_ADDER_DEFINITION(x) Ref<S##x> CScene::Add##x##ToEntity(Ref<SEntity> entity){ entity->AddComponent(EComponentType::##x, x##s.size()); return x##s[entity->GetComponentIndex(EComponentType::##x)] = std::make_shared<S##x>(entity, EComponentType::##x);}

#define COMPONENT_VECTOR_DECLARATION(x) std::array<S##x*, ENTITY_LIMIT> x##s;
#define COMPONENT_VECTOR_GETTER(x) std::array<S##x*, ENTITY_LIMIT>& Get##x##s() { return x##s; }

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

	class CRenderManager;
	class CAssetRegistry;

	class CScene final
	{
	public:
		CScene();
		~CScene() = default;

		bool Init(CRenderManager* renderManager, CAssetRegistry* assetRegistry, U8 sceneIndex = 0);

		bool InitDemoScene(CRenderManager* renderManager);

		void SaveScene(const std::string& destinationPath);
		void LoadScene(const std::string& destinationPath);
		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U32& pointerPosition) const;
		void Deserialize(const char* fromData, U32& pointerPosition);

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

		//std::vector<Ref<SEntity>>& GetEntities() { return Entities; }
		std::vector<SEntity*>& GetEntities() { return Entities; }

		//Ref<SEntity> CreateEntity(const std::string& name);
		SEntity* CreateEntity(const std::string& name);
		//COMPONENT_ADDER_DECLARATION(TransformComponent)
		//COMPONENT_ADDER_DECLARATION(StaticMeshComponent)
		//COMPONENT_ADDER_DECLARATION(CameraComponent)
		//COMPONENT_ADDER_DECLARATION(CameraControllerComponent)
		//COMPONENT_ADDER_DECLARATION(MaterialComponent)
		//COMPONENT_ADDER_DECLARATION(EnvironmentLightComponent)
		//COMPONENT_ADDER_DECLARATION(DirectionalLightComponent)
		//COMPONENT_ADDER_DECLARATION(PointLightComponent)
		//COMPONENT_ADDER_DECLARATION(SpotLightComponent)
		//COMPONENT_ADDER_DECLARATION(VolumetricLightComponent)
		//COMPONENT_ADDER_DECLARATION(DecalComponent)
		//COMPONENT_ADDER_DECLARATION(DebugShapeComponent)

	private:
		//std::vector<Ref<SEntity>> Entities;
		std::vector<SEntity*> Entities;
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
		U64 FirstUnusedEntityIndex = 0;
	};
}
