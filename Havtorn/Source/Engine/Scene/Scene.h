// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#ifdef _DEBUG
#include "Debug/DebugShape.h"
#endif

namespace Havtorn
{
#define COMPONENT_ADDER_DECLARATION(x) Ref<S##x> Add##x##ToEntity(Ref<SEntity> entity);
#define COMPONENT_ADDER_DEFINITION(x) Ref<S##x> CScene::Add##x##ToEntity(Ref<SEntity> entity){ entity->AddComponent(EComponentType::##x, x##s.size()); return x##s.emplace_back(std::make_shared<S##x>(entity, EComponentType::##x));}

#define COMPONENT_VECTOR_DECLARATION(x) std::vector<Ref<S##x>> x##s;
#define COMPONENT_VECTOR_GETTER(x) std::vector<Ref<S##x>>& Get##x##s() { return x##s; }

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

	class ISystem;
	class CRenderManager;

	class CScene final
	{
	public:
		CScene() = default;
		~CScene() = default;

		bool Init(CRenderManager* renderManager);
		void Update();

		void InitDemoScene(CRenderManager* renderManager);

		std::vector<Ref<STransformComponent>>& GetTransformComponents() { return TransformComponents; }
		std::vector<Ref<SStaticMeshComponent>>& GetStaticMeshComponents() { return StaticMeshComponents; }
		std::vector<Ref<SCameraComponent>>& GetCameraComponents() { return CameraComponents; }
		COMPONENT_VECTOR_GETTER(CameraControllerComponent)
		COMPONENT_VECTOR_GETTER(MaterialComponent)
		COMPONENT_VECTOR_GETTER(EnvironmentLightComponent)
		std::vector<Ref<SDirectionalLightComponent>>& GetDirectionalLightComponents() { return DirectionalLightComponents; }
		COMPONENT_VECTOR_GETTER(PointLightComponent)
		std::vector<Ref<SSpotLightComponent>>& GetSpotLightComponents() { return SpotLightComponents; }
		COMPONENT_VECTOR_GETTER(VolumetricLightComponent)
		COMPONENT_VECTOR_GETTER(DecalComponent)

		std::vector<Ref<SEntity>>& GetEntities() { return Entities; }

#ifdef _DEBUG
		 void AddDebugShapes(std::vector<const Debug::SDebugShape*>&& shapes) { DebugShapes = std::move(shapes); }
#endif
		Ref<SEntity> CreateEntity(const std::string& name);
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

	private:
		std::vector<Ref<SEntity>> Entities;
		std::vector<Ref<STransformComponent>> TransformComponents;
		std::vector<Ref<SStaticMeshComponent>> StaticMeshComponents;
		std::vector<Ref<SCameraComponent>> CameraComponents;
		COMPONENT_VECTOR_DECLARATION(CameraControllerComponent)
		COMPONENT_VECTOR_DECLARATION(MaterialComponent)
		COMPONENT_VECTOR_DECLARATION(EnvironmentLightComponent)
		std::vector<Ref<SDirectionalLightComponent>> DirectionalLightComponents;
		COMPONENT_VECTOR_DECLARATION(PointLightComponent)
		std::vector<Ref<SSpotLightComponent>> SpotLightComponents;
		COMPONENT_VECTOR_DECLARATION(VolumetricLightComponent)
		COMPONENT_VECTOR_DECLARATION(DecalComponent)
		std::vector<Ptr<ISystem>> Systems;

#ifdef _DEBUG
		std::vector<const Debug::SDebugShape*> DebugShapes;
#endif
	};
}
