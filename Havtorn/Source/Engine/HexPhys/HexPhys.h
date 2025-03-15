// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/System.h"

#include <box2cpp.h>
//#include <debug_imgui_renderer.h>

#include <PxPhysicsAPI.h>

namespace b2
{
	class World;
	class Body;
	//class DebugImguiRenderer;
}

namespace physx
{
	class PxFoundation;
	class PxPhysics;
	class PxScene;
	class PxMaterial;
	class PxPvd;

	class PxDefaultAllocator;
	class PxDefaultErrorCallback;
	class PxDefaultCpuDispatcher;
	class PxTolerancesScale;
}

namespace Havtorn
{
	class CScene;
	struct SEntity;
	struct STransformComponent;
	struct SPhysics2DComponent;
	struct SPhysics3DComponent;
	struct SPhysics3DControllerComponent;

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
			// NW: Make debug renderer through GUI project? Nice if we can avoid an ImGui include in this Project
			//b2::DebugImguiRenderer DebugRenderer;
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
		// NR: Might want to move these out to another file. I think it's fine for now.
		class CErrorCallback : public physx::PxErrorCallback
		{
		public:
			void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
		};

		class CSimulationEventCallback : public physx::PxSimulationEventCallback
		{
		public:
			void onWake(physx::PxActor** actors, physx::PxU32 count) override;
			void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
			void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
			void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;
			void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
			void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;

			void OnTriggerEnter(const physx::PxActor* trigger, const physx::PxActor* otherActor, const physx::PxScene* physicsScene);
			void OnTriggerExit(const physx::PxActor* trigger, const physx::PxActor* otherActor, const physx::PxScene* physicsScene);
		};


		// NR: Keep these for now even though they are empty, they're good to keep as reference.
		class CSimulationFilterCallback : public physx::PxSimulationFilterCallback
		{
		public:
			physx::PxFilterFlags pairFound(physx::PxU64 pairID, physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, 
				const physx::PxActor* a0, const physx::PxShape* s0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, 
				const physx::PxActor* a1, const physx::PxShape* s1, physx::PxPairFlags& pairFlags) override;

			void pairLost(physx::PxU64 pairID, physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, 
				physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, bool objectRemoved) override;

			bool statusChange(physx::PxU64& pairID, physx::PxPairFlags& pairFlags, physx::PxFilterFlags& filterFlags) override;
		};

		class CUserControllerHitReport : public physx::PxUserControllerHitReport
		{
		public:

			void onShapeHit(const physx::PxControllerShapeHit& hit) override;
			void onControllerHit(const physx::PxControllersHit& hit) override;
			void onObstacleHit(const physx::PxControllerObstacleHit& hit) override;
		};

		class CPhysicsWorld3D
		{
		public:
			CPhysicsWorld3D();
			~CPhysicsWorld3D();

			void CreateScene(CScene* havtornScene);

			void Update();

			void InitializePhysicsData(STransformComponent* transform, SPhysics3DComponent* component) const;
			void InitializePhysicsData(STransformComponent* transform, SPhysics3DControllerComponent* controller) const;
			void SetPhysicsDataOnComponents(STransformComponent* transform, SPhysics3DComponent* component) const;

			void UpdatePhysicsData(STransformComponent* transform, SPhysics3DComponent* component) const;

		private:
			physx::PxActor* MakeRigidStatic(const STransformComponent* transform, const SPhysics3DComponent* component) const;
			physx::PxActor* MakeRigidKinematic(const STransformComponent* transform, const SPhysics3DComponent* component) const;
			physx::PxActor* MakeRigidDynamic(const STransformComponent* transform, const SPhysics3DComponent* component) const;
			physx::PxShape* CreateShapeFromComponent(const SPhysics3DComponent* component) const;
			physx::PxMaterial* GetMaterialFromComponent(const SPhysics3DComponent* component) const;

			static physx::PxVec3T<F32> Convert(const SVector& from);
			static SVector Convert(const physx::PxVec3T<F32>& from);
			static physx::PxQuatT<F32> Convert(const SQuaternion& from);
			static SQuaternion Convert(const physx::PxQuatT<F32>& from);

		private:
			physx::PxFoundation* Foundation = nullptr;
			physx::PxPhysics* Physics = nullptr;
			physx::PxPvd* PVD = nullptr;

			physx::PxDefaultAllocator DefaultAllocatorCallback;

			CErrorCallback ErrorCallback;

			physx::PxDefaultCpuDispatcher* DefaultCPUDispatcher = nullptr;

			CSimulationEventCallback* SimulationEventCallback = nullptr;
			CSimulationFilterCallback* SimulationFilterCallback = nullptr;
			CUserControllerHitReport* UserControllerHitReport = nullptr;

			physx::PxTolerancesScale TolerancesScale = physx::PxTolerancesScale();
			physx::PxScene* CurrentScene = nullptr;

			// TODO.NR: Do we need to store different materials?
			physx::PxMaterial* MainMaterial = nullptr;

			physx::PxControllerManager* ControllerManager = nullptr;
		};

		class CPhysics3DSystem : public ISystem
		{
		public:
			CPhysics3DSystem(CPhysicsWorld3D* physicsWorld);
			~CPhysics3DSystem() override = default;

			void Update(CScene* scene) override;

		private:
			CPhysicsWorld3D* PhysicsWorld = nullptr;
		};
	}
}
