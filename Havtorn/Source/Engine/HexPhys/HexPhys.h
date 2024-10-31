// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/System.h"

#include <box2cpp.h>
#include <debug_imgui_renderer.h>

namespace b2
{
	class World;
	class Body;
	class DebugImguiRenderer;
}

namespace Havtorn
{
	class CScene;
	struct SEntity;
	struct STransformComponent;
	struct SPhysics2DComponent;

	namespace HexPhys2D
	{
		class CPhysicsWorld2D
		{
		public:
			CPhysicsWorld2D();
			~CPhysicsWorld2D() = default;

			void Update(CScene* scene);

			void InitializePhysicsData(STransformComponent* transform, SPhysics2DComponent* component);
			void SetPhysicsDataOnComponents(STransformComponent* transform, SPhysics2DComponent* component) const;

			void UpdatePhysicsData(STransformComponent* transform, SPhysics2DComponent* component) const;

		private:
			b2::Body::Params MakeBodyParamsFromComponents(STransformComponent* transform, SPhysics2DComponent* component);
			b2::Shape::Params MakeShapeParamsFromComponent(SPhysics2DComponent* component);

			void CreateShape(SPhysics2DComponent* component);

		private:
			b2::World World;
			b2::DebugImguiRenderer DebugRenderer;
			std::map<SEntity, b2::Body> Bodies;
			std::map<U64, SEntity> BodyIDMap;
		};

		class CPhysics2DSystem : public ISystem
		{
		public:
			CPhysics2DSystem(CPhysicsWorld2D* physicsWorld);
			~CPhysics2DSystem() override = default;

			void Update(CScene* scene) override;

		private:
			CPhysicsWorld2D* PhysicsWorld = nullptr;
		};
	}

	namespace HexPhys3D
	{

	}
}