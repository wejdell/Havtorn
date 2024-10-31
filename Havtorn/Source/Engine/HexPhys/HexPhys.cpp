// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "HexPhys.h"

#include "ECS/Components/Physics2DComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	namespace HexPhys2D
	{
		CPhysicsWorld2D::CPhysicsWorld2D()
		{
			b2::World::Params worldParams = {};
			worldParams.gravity = { 0.f, -9.8f };
			World = b2::World(worldParams);
			DebugRenderer = b2::DebugImguiRenderer();
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
		// TODO.NR: Integrate PhysX or Havok
	}
}