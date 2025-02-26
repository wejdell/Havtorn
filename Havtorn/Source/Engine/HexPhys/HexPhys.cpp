// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "HexPhys.h"

#include "ECS/Components/Physics2DComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

#include <pvd/PxPvd.h>

#include "ECS/Components/MetaDataComponent.h"
#include "ECS/Components/Physics3DComponent.h"
#include "ECS/Components/Physics3DControllerComponent.h"

namespace Havtorn
{
	namespace HexPhys2D
	{
		CPhysicsWorld2D::CPhysicsWorld2D()
		{
			b2::World::Params worldParams = {};
			worldParams.gravity = { 0.f, -9.8f };
			World = b2::World(worldParams);
			//DebugRenderer = b2::DebugImguiRenderer();
		}

		void CPhysicsWorld2D::Update(CScene* scene)
		{
			World.Step(1.f / 60.f, 4);

			// TODO.NR: b2BodyEvents doesn't define iterators, so we can't use a foreach loop here. Might be worth to add.
			b2BodyEvents events = World.GetBodyEvents();
			for (U64 index = 0; index < events.moveCount; index++)
			{
				U64 id = b2StoreBodyId(events.moveEvents[index].bodyId);
				if (BodyIDMap.contains(id))
				{
					const SEntity& movedEntity = BodyIDMap.at(id);
					SetPhysicsDataOnComponents(scene->GetComponent<STransformComponent>(movedEntity), scene->GetComponent<SPhysics2DComponent>(movedEntity));
				}
				else
				{
					HV_LOG_ERROR("__FUNCTION__: Physics2D Body had move event but wasn't registered in the scene!");
				}
			}

			// TODO.NR: Move debug drawing to imgui layer
		}

		void CPhysicsWorld2D::InitializePhysicsData(STransformComponent* transform, SPhysics2DComponent* component)
		{
			if (!Bodies.contains(component->Owner))
			{
				Bodies.emplace(component->Owner, World.CreateBody(b2::OwningHandle, MakeBodyParamsFromComponents(transform, component)));
				BodyIDMap.emplace(b2StoreBodyId(Bodies.at(component->Owner).Handle()), component->Owner);
			}
		
			// TODO.NR: Support multiple shapes on same body
			b2::BodyRef body = Bodies.at(component->Owner);
			
			if (body.GetShapeCount() == 0)
				CreateShape(component);
		}

		void CPhysicsWorld2D::SetPhysicsDataOnComponents(STransformComponent* transform, SPhysics2DComponent* component) const
		{
			if (transform == nullptr || component == nullptr)
				return;

			b2::BodyRef body = Bodies.at(component->Owner);

			const b2Transform& physicsTransform = body.GetTransform();
			const b2Vec2& position = physicsTransform.p;
			const b2Rot& rotation = physicsTransform.q;

			SMatrix matrix = transform->Transform.GetMatrix();
			matrix.SetTranslation({ position.x, position.y, matrix.GetTranslation().Z });
			SVector eulerAngles = matrix.GetEuler();
			matrix.SetRotation({ eulerAngles.X, eulerAngles.Y, UMath::RadToDeg(b2Rot_GetAngle(rotation)) });
			transform->Transform.SetMatrix(matrix);

			b2Vec2 bodyVelocity = body.GetLinearVelocity();
			component->Velocity = { bodyVelocity.x, bodyVelocity.y };
		}

		void CPhysicsWorld2D::UpdatePhysicsData(STransformComponent* /*transform*/, SPhysics2DComponent* component) const
		{
			b2::BodyRef body = Bodies.at(component->Owner);
			body.SetLinearVelocity({ component->Velocity.X, component->Velocity.Y });
		}

		b2::Body::Params CPhysicsWorld2D::MakeBodyParamsFromComponents(STransformComponent* transform, SPhysics2DComponent* component)
		{
			b2::Body::Params params;
			if (transform == nullptr || component == nullptr)
				return params;

			switch (component->BodyType)
			{
			case EPhysics2DBodyType::Kinematic:
				params.type = b2_kinematicBody;
				break;
			case EPhysics2DBodyType::Dynamic:
				params.type = b2_dynamicBody;
				break;
			case EPhysics2DBodyType::Static:
			default:
				params.type = b2_staticBody;
			}
			
			const SMatrix& matrix = transform->Transform.GetMatrix();
			const SVector& translation = matrix.GetTranslation();
			params.position = { translation.X, translation.Y };
			params.rotation = b2MakeRot(UMath::DegToRad(matrix.GetEuler().Z));

			params.fixedRotation = component->ConstrainRotation;

			return params;
		}

		b2::Shape::Params CPhysicsWorld2D::MakeShapeParamsFromComponent(SPhysics2DComponent* /*component*/)
		{
			return b2::Shape::Params();
		}

		void CPhysicsWorld2D::CreateShape(SPhysics2DComponent* component)
		{
			b2::BodyRef body = Bodies.at(component->Owner);

			switch (component->ShapeType)
			{
			case EPhysics2DShapeType::Capsule:
			{
				const F32 minDimension = UMath::Min(component->ShapeLocalExtents.X, component->ShapeLocalExtents.Y);
				const F32 maxDimension = UMath::Max(component->ShapeLocalExtents.X, component->ShapeLocalExtents.Y);

				SVector2<F32> extentDirection = UMath::NearlyEqual(maxDimension, component->ShapeLocalExtents.X) ? SVector2<F32>::Right : SVector2<F32>::Up;
				F32 radius = minDimension;
				F32 extentSize = (maxDimension - minDimension);

				SVector2<F32> firstCenter = component->ShapeLocalOffset + extentDirection * extentSize;
				SVector2<F32> secondCenter = component->ShapeLocalOffset + extentDirection * -extentSize;

				body.CreateShape(b2::DestroyWithParent, MakeShapeParamsFromComponent(component), b2Capsule{ .center1 = { firstCenter.X, firstCenter.Y }, .center2 = { secondCenter.X, secondCenter.Y }, .radius = radius });
			}
			break;
			case EPhysics2DShapeType::Segment:
			{
				SVector2<F32> firstPoint = component->ShapeLocalOffset + SVector2<F32>::Up * component->ShapeLocalExtents.Y * 0.5f;
				SVector2<F32> secondPoint = component->ShapeLocalOffset + SVector2<F32>::Down * component->ShapeLocalExtents.Y * 0.5f;
				b2Vec2 point1 = { firstPoint.X, firstPoint.Y };
				b2Vec2 point2 = { secondPoint.X, secondPoint.Y };

				body.CreateShape(b2::DestroyWithParent, MakeShapeParamsFromComponent(component), b2Segment{ .point1 = point1, .point2 = point2 });
			}
			break;
			case EPhysics2DShapeType::Polygon:
			{
				// TODO.NR: Support polygon shapes
				body.CreateShape(b2::DestroyWithParent, MakeShapeParamsFromComponent(component), b2Polygon{  });
			}
			break;
			case EPhysics2DShapeType::ChainSegment:
				// TODO.NR: Support chain collision shapes
				//body.CreateChain(b2::DestroyWithParent, MakeShapeParamsFromComponent(component), b2ChainSegment{  });
				break;
			case EPhysics2DShapeType::Circle:
			default:
			{
				b2Vec2 point1 = { component->ShapeLocalOffset.X, component->ShapeLocalOffset.Y };
				body.CreateShape(b2::DestroyWithParent, MakeShapeParamsFromComponent(component), b2Circle{ .center = point1, .radius = UMath::Max(component->ShapeLocalExtents.X, component->ShapeLocalExtents.Y) });
			}
			break;
			}
		}

		CPhysics2DSystem::CPhysics2DSystem(CPhysicsWorld2D* physicsWorld)
			: PhysicsWorld(physicsWorld)
		{}

		void CPhysics2DSystem::Update(CScene* scene)
		{
			if (GTime::FixedTimeStep())
				PhysicsWorld->Update(scene);
		}
	}

	namespace HexPhys3D
	{
		using namespace physx;

		void CErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
		{
			switch (code)
			{
			case PxErrorCode::eNO_ERROR:
				HV_LOG_TRACE("PhysX report in file %s at line %i: %s", file, line, message);
				break;
			case PxErrorCode::eDEBUG_INFO:
				HV_LOG_INFO("PhysX report in file %s at line %i: %s", file, line, message);
				break;
			case PxErrorCode::eDEBUG_WARNING:
				HV_LOG_WARN("PhysX report in file %s at line %i: %s", file, line, message);
				break;
			case PxErrorCode::eINVALID_PARAMETER:
			case PxErrorCode::eINVALID_OPERATION:
			case PxErrorCode::eOUT_OF_MEMORY:
			case PxErrorCode::eINTERNAL_ERROR:
			case PxErrorCode::eABORT:
				HV_LOG_ERROR("PhysX report in file %s at line %i: %s", file, line, message);
				break;
			case PxErrorCode::ePERF_WARNING:
				HV_LOG_WARN("PhysX report in file %s at line %i: %s", file, line, message);
				break;
			case PxErrorCode::eMASK_ALL:
				break;
			}
		}

		void CSimulationEventCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
		{
			// TODO.NR: Doesn't seem to get called, look up docs
			if (count > 0)
				HV_LOG_TRACE("Physics actor woke: %s", actors[0]->getName());
		}

		void CSimulationEventCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
		{
			// TODO.NR: Doesn't seem to get called, look up docs
			if (count > 0)
				HV_LOG_TRACE("Physics actor slept: %s", actors[0]->getName());
		}

		void CSimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
		{
			for (physx::PxU32 i = 0; i < count; i++)
			{
				if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
					continue;

				if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
					OnTriggerEnter(pairs[i].triggerActor, pairs[i].otherActor, pairs[i].triggerActor->getScene());
				else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST) 
					OnTriggerExit(pairs[i].triggerActor, pairs[i].otherActor, pairs[i].triggerActor->getScene());
			}
		}

		void CSimulationEventCallback::onAdvance(const physx::PxRigidBody* const* /*bodyBuffer*/, const physx::PxTransform* /*poseBuffer*/, const physx::PxU32 /*count*/)
		{
		}

		void CSimulationEventCallback::onConstraintBreak(physx::PxConstraintInfo* /*constraints*/, physx::PxU32 /*count*/)
		{
		}

		void CSimulationEventCallback::onContact(const physx::PxContactPairHeader& /*pairHeader*/, const physx::PxContactPair* /*pairs*/, physx::PxU32 /*nbPairs*/)
		{
		}

		void CSimulationEventCallback::OnTriggerEnter(const physx::PxActor* trigger, const physx::PxActor* otherActor, const physx::PxScene* physicsScene)
		{
			CScene* havtornScene = static_cast<CScene*>(physicsScene->userData);
			if (havtornScene == nullptr)
				return;

			SEntity* triggerEntity = static_cast<SEntity*>(trigger->userData);
			if (triggerEntity == nullptr)
				return;

			SEntity* otherEntity = static_cast<SEntity*>(otherActor->userData);
			if (otherEntity == nullptr)
				return;

			SMetaDataComponent* triggerMetaData = havtornScene->GetComponent<SMetaDataComponent>(*triggerEntity);
			if (triggerMetaData == nullptr)
				return;

			SMetaDataComponent* otherMetaData = havtornScene->GetComponent<SMetaDataComponent>(*otherEntity);
			if (otherEntity == nullptr)
				return;

			// TODO.NR: Probably want to notify a System here, to deal with the trigger in gameplay code
			HV_LOG_INFO("%s entered trigger volume with name %s", otherMetaData->Name.Data(), triggerMetaData->Name.Data());
		}

		void CSimulationEventCallback::OnTriggerExit(const physx::PxActor* trigger, const physx::PxActor* otherActor, const physx::PxScene* physicsScene)
		{
			CScene* havtornScene = static_cast<CScene*>(physicsScene->userData);
			if (havtornScene == nullptr)
				return;

			SEntity* triggerEntity = static_cast<SEntity*>(trigger->userData);
			if (triggerEntity == nullptr)
				return;

			SEntity* otherEntity = static_cast<SEntity*>(otherActor->userData);
			if (otherEntity == nullptr)
				return;

			SMetaDataComponent* triggerMetaData = havtornScene->GetComponent<SMetaDataComponent>(*triggerEntity);
			if (triggerMetaData == nullptr)
				return;

			SMetaDataComponent* otherMetaData = havtornScene->GetComponent<SMetaDataComponent>(*otherEntity);
			if (otherEntity == nullptr)
				return;

			// TODO.NR: Probably want to notify a System here, to deal with the trigger in gameplay code
			HV_LOG_INFO("%s exited trigger volume with name %s", otherMetaData->Name.Data(), triggerMetaData->Name.Data());
		}

		physx::PxFilterFlags CSimulationFilterCallback::pairFound(physx::PxU64 /*pairID*/,
			physx::PxFilterObjectAttributes /*attributes0*/, physx::PxFilterData /*filterData0*/, const physx::PxActor* /*a0*/,
			const physx::PxShape* /*s0*/, physx::PxFilterObjectAttributes /*attributes1*/, physx::PxFilterData /*filterData1*/,
			const physx::PxActor* /*a1*/, const physx::PxShape* /*s1*/, physx::PxPairFlags& /*pairFlags*/)
		{
			return {};
		}

		void CSimulationFilterCallback::pairLost(physx::PxU64 /*pairID*/, physx::PxFilterObjectAttributes /*attributes0*/,
			physx::PxFilterData /*filterData0*/, physx::PxFilterObjectAttributes /*attributes1*/,
			physx::PxFilterData /*filterData1*/, bool /*objectRemoved*/)
		{
		}

		bool CSimulationFilterCallback::statusChange(physx::PxU64& /*pairID*/, physx::PxPairFlags& /*pairFlags*/,
			physx::PxFilterFlags& /*filterFlags*/)
		{
			return false;
		}

		void CUserControllerHitReport::onShapeHit(const physx::PxControllerShapeHit& /*hit*/)
		{
		}

		void CUserControllerHitReport::onControllerHit(const physx::PxControllersHit& /*hit*/)
		{
		}

		void CUserControllerHitReport::onObstacleHit(const physx::PxControllerObstacleHit& /*hit*/)
		{
		}

		CPhysicsWorld3D::CPhysicsWorld3D()
		{
			Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, DefaultAllocatorCallback, ErrorCallback);

			// TODO.NR: Add support for remote debugging?
			static const char* pvdPort = "127.0.0.1";
			PVD = PxCreatePvd(*Foundation);
			PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(pvdPort, 5425, 10);
			PVD->connect(*transport, PxPvdInstrumentationFlag::eALL);

			Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, PxTolerancesScale(), true, PVD);

			DefaultCPUDispatcher = PxDefaultCpuDispatcherCreate(2);
			SimulationEventCallback = new CSimulationEventCallback();
			SimulationFilterCallback = new CSimulationFilterCallback();
			UserControllerHitReport = new CUserControllerHitReport();

			MainMaterial = Physics->createMaterial(0.5f, 0.5f, 0.6f);
		}

		CPhysicsWorld3D::~CPhysicsWorld3D()
		{
			SAFE_DELETE(SimulationEventCallback)
			SAFE_DELETE(SimulationFilterCallback)
			SAFE_DELETE(UserControllerHitReport)

			PX_RELEASE(MainMaterial)
			PX_RELEASE(ControllerManager)
			PX_RELEASE(CurrentScene)

			PX_RELEASE(DefaultCPUDispatcher)
			PX_RELEASE(Physics)
			if (PVD)
			{
				PxPvdTransport* transport = PVD->getTransport();
				PX_RELEASE(PVD)
				PX_RELEASE(transport)
			}
			PX_RELEASE(Foundation)
		}

		void CPhysicsWorld3D::CreateScene(CScene* havtornScene)
		{
			PxSceneDesc sceneDesc(Physics->getTolerancesScale());
			sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
			sceneDesc.cpuDispatcher = DefaultCPUDispatcher;
			sceneDesc.filterShader = PxDefaultSimulationFilterShader; // TODO.NR: Do we need a custom filter?
			sceneDesc.simulationEventCallback = SimulationEventCallback;
			sceneDesc.filterCallback = SimulationFilterCallback;
			sceneDesc.flags = PxSceneFlag::eENABLE_ACTIVE_ACTORS | PxSceneFlag::eENABLE_CCD;

			// TODO.NR: Need to make a new scene for each new Havtorn Scene
			CurrentScene = Physics->createScene(sceneDesc);

			CurrentScene->userData = havtornScene;

			if (PxPvdSceneClient* pvdClient = CurrentScene->getScenePvdClient())
			{
				pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
				pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
				pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
			}

			ControllerManager = PxCreateControllerManager(*CurrentScene);
		}

		void CPhysicsWorld3D::Update()
		{
			if (CurrentScene == nullptr)
				return;

			CurrentScene->simulate(1.0f / 60.0f);
			CurrentScene->fetchResults(true);

			PxU32 numActiveActors = 0;
			PxActor** activeActors = CurrentScene->getActiveActors(numActiveActors);

			if (numActiveActors == 0)
				return;

			CScene* havtornScene = static_cast<CScene*>(CurrentScene->userData);
			if (havtornScene == nullptr)
				return;

			for (U32 i = 0; i < numActiveActors; ++i)
			{
				PxRigidActor* rigidActor = activeActors[i]->is<PxRigidActor>();
				if (rigidActor == nullptr)
					continue;

				const SEntity& entity = *static_cast<SEntity*>(rigidActor->userData);

				STransformComponent* transform = havtornScene->GetComponent<STransformComponent>(entity);
				if (transform == nullptr)
					continue;

				SVector translation = SVector::Zero;
				SVector eulerAngles = SVector::Zero;
				SVector scale = SVector::Zero;
				SMatrix matrix = transform->Transform.GetMatrix();

				SMatrix::Decompose(matrix, translation, eulerAngles, scale);
				translation = Convert(rigidActor->getGlobalPose().p);
				eulerAngles = Convert(rigidActor->getGlobalPose().q).ToEuler();
				SMatrix::Recompose(translation, eulerAngles, scale, matrix);

				transform->Transform.SetMatrix(matrix);

				SPhysics3DComponent* component = havtornScene->GetComponent<SPhysics3DComponent>(entity);
				SPhysics3DControllerComponent* controller = havtornScene->GetComponent<SPhysics3DControllerComponent>(entity);
				if (component != nullptr)
				{
				}
				else if (controller != nullptr)
				{
				}
			}
		}

		void CPhysicsWorld3D::InitializePhysicsData(STransformComponent* transform, SPhysics3DComponent* component) const
		{
			std::function<PxActor* (const STransformComponent*, const SPhysics3DComponent*)> creationFunction;

			switch (component->BodyType)
			{
			case EPhysics3DBodyType::Static:
				creationFunction = [this](const STransformComponent* transform, const SPhysics3DComponent* component)
					{
						return MakeRigidStatic(std::forward<const STransformComponent*>(transform), std::forward<const SPhysics3DComponent*>(component));
					};
				break;
			case EPhysics3DBodyType::Kinematic:
				creationFunction = [this](const STransformComponent* transform, const SPhysics3DComponent* component)
					{
						return MakeRigidKinematic(std::forward<const STransformComponent*>(transform), std::forward<const SPhysics3DComponent*>(component));
					};
				break;
			case EPhysics3DBodyType::Dynamic:
				creationFunction = [this](const STransformComponent* transform, const SPhysics3DComponent* component)
					{
						return MakeRigidDynamic(std::forward<const STransformComponent*>(transform), std::forward<const SPhysics3DComponent*>(component));
					};
				break;
			}

			PxActor* newActor = creationFunction(transform, component);
			newActor->userData = &component->Owner;
			CurrentScene->addActor(*newActor);
		}

		void CPhysicsWorld3D::InitializePhysicsData(STransformComponent* transform, SPhysics3DControllerComponent* controller) const
		{
			PxVec3T<F32> position = Convert(transform->Transform.GetMatrix().GetTranslation());
			PxVec3T<F32> upDirection = Convert(transform->Transform.GetMatrix().GetUp());

			switch (controller->ControllerType)
			{
			case EPhysics3DControllerType::Box:
			{
				PxBoxControllerDesc desc;
				desc.userData = controller;
				desc.halfSideExtent = controller->ShapeLocalExtents.X * 0.5f;
				desc.halfHeight = controller->ShapeLocalExtents.Y * 0.5f;
				desc.halfForwardExtent = controller->ShapeLocalExtents.Z * 0.5f;
				desc.reportCallback = UserControllerHitReport;
				desc.density = 10.0f;
				desc.position = { position.x, position.y, position.z };
				desc.upDirection = { upDirection.x, upDirection.y, upDirection.z };
				desc.material = MainMaterial;
				ControllerManager->createController(desc);
			}
				break;
			case EPhysics3DControllerType::Capsule:
			{
				PxCapsuleControllerDesc desc;
				desc.userData = controller;
				desc.height = controller->ShapeLocalExtents.Y;
				desc.radius = UMath::Max(controller->ShapeLocalExtents.X, controller->ShapeLocalExtents.Z);
				desc.reportCallback = UserControllerHitReport;
				desc.density = 10.0f;
				desc.position = { position.x, position.y, position.z };
				desc.upDirection = { upDirection.x, upDirection.y, upDirection.z };
				desc.material = MainMaterial;
				ControllerManager->createController(desc);
			}
				break;
			}
		}

		void CPhysicsWorld3D::SetPhysicsDataOnComponents(STransformComponent* /*transform*/, SPhysics3DComponent* /*component*/) const
		{

		}

		void CPhysicsWorld3D::UpdatePhysicsData(STransformComponent* /*transform*/, SPhysics3DComponent* /*component*/) const
		{
		}

		physx::PxActor* CPhysicsWorld3D::MakeRigidStatic(const STransformComponent* transform, const SPhysics3DComponent* component) const
		{
			const SMatrix& transformMatrix = transform->Transform.GetMatrix();
			PxTransform worldTransform(Convert(transformMatrix.GetTranslation()), Convert(SQuaternion(transformMatrix.GetEuler())));
			PxTransform localTransform(Convert(component->ShapeLocalOffset), PxQuatT<F32>(PxIdentity));

			PxRigidStatic* body = Physics->createRigidStatic(worldTransform.transform(localTransform));

			PxShape* shape = CreateShapeFromComponent(component);
			if (component->IsTrigger)
			{
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			}

			body->attachShape(*shape);

			return body;
		}

		physx::PxActor* CPhysicsWorld3D::MakeRigidKinematic(const STransformComponent* transform, const SPhysics3DComponent* component) const
		{
			const SMatrix& transformMatrix = transform->Transform.GetMatrix();
			PxTransform worldTransform(Convert(transformMatrix.GetTranslation()), Convert(SQuaternion(transformMatrix.GetEuler())));
			PxTransform localTransform(Convert(component->ShapeLocalOffset), PxQuatT<F32>(PxIdentity));

			PxRigidDynamic* body = Physics->createRigidDynamic(worldTransform.transform(localTransform));
			body->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eKINEMATIC, true);

			body->attachShape(*CreateShapeFromComponent(component));
			PxRigidBodyExt::updateMassAndInertia(*body, component->Density);

			return body;
		}

		physx::PxActor* CPhysicsWorld3D::MakeRigidDynamic(const STransformComponent* transform, const SPhysics3DComponent* component) const
		{
			const SMatrix& transformMatrix = transform->Transform.GetMatrix();
			PxTransform worldTransform(Convert(transformMatrix.GetTranslation()), Convert(SQuaternion(transformMatrix.GetEuler())));
			PxTransform localTransform(Convert(component->ShapeLocalOffset), PxQuatT<F32>(PxIdentity));

			PxRigidDynamic* body = Physics->createRigidDynamic(worldTransform.transform(localTransform));

			body->attachShape(*CreateShapeFromComponent(component));

			PxRigidBodyExt::updateMassAndInertia(*body, component->Density);

			return body;
		}

		physx::PxShape* CPhysicsWorld3D::CreateShapeFromComponent(const SPhysics3DComponent* component) const
		{
			PxMaterial* material = GetMaterialFromComponent(component);

			switch (component->ShapeType)
			{
			case EPhysics3DShapeType::Sphere:
				return Physics->createShape(PxSphereGeometry(component->ShapeLocalRadius), *material);
			case EPhysics3DShapeType::InfinitePlane:
				return Physics->createShape(PxPlaneGeometry(), *material);
			case EPhysics3DShapeType::Capsule:
				return Physics->createShape(PxCapsuleGeometry(component->ShapeLocalRadiusAndHeight.X, component->ShapeLocalRadiusAndHeight.Y * 0.5f), *material);
			case EPhysics3DShapeType::Box:
				return Physics->createShape(PxBoxGeometry(Convert(component->ShapeLocalExtents * 0.5f)), *material);
			case EPhysics3DShapeType::Convex:
				HV_ASSERT(false, "Convex mesh collision is not supported yet!")
				return Physics->createShape(PxConvexMeshGeometry(), *material);
			}

			HV_ASSERT(false, "Unhandled Physics 3D Shape Type!")
			return nullptr;
		}

		physx::PxMaterial* CPhysicsWorld3D::GetMaterialFromComponent(const SPhysics3DComponent* component) const
		{
			// TODO.NR: How do we store different materials?
			MainMaterial->setDynamicFriction(component->Material.DynamicFriction);
			MainMaterial->setStaticFriction(component->Material.StaticFriction);
			MainMaterial->setRestitution(component->Material.Restitution);

			return MainMaterial;
		}

		physx::PxVec3T<F32> CPhysicsWorld3D::Convert(const SVector& from)
		{
			return { from.X, from.Y, from.Z };
		}

		SVector CPhysicsWorld3D::Convert(const physx::PxVec3T<F32>& from)
		{
			return { from.x, from.y, from.z };
		}

		physx::PxQuatT<F32> CPhysicsWorld3D::Convert(const SQuaternion& from)
		{
			return { from.X, from.Y, from.Z, from.W };
		}

		SQuaternion CPhysicsWorld3D::Convert(const physx::PxQuatT<F32>& from)
		{
			return { from.x, from.y, from.z, from.w };
		}

		CPhysics3DSystem::CPhysics3DSystem(CPhysicsWorld3D* physicsWorld)
			: PhysicsWorld(physicsWorld)
		{}

		void CPhysics3DSystem::Update(CScene* /*scene*/)
		{
			if (GTime::FixedTimeStep())
				PhysicsWorld->Update();
		}
	}
}
