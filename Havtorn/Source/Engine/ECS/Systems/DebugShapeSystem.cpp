// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugShapeSystem.h"

#include "Scene/Scene.h"
#include "ECS/Components/DebugShapeComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Graphics/VertexBufferPrimitivesUtility.h"

namespace Havtorn
{
	namespace Debug
	{
		UDebugShapeSystem* UDebugShapeSystem::Instance = nullptr;
		const std::vector<U64> UDebugShapeSystem::NoShapeIndices;

		UDebugShapeSystem::UDebugShapeSystem(CScene* scene)
			: ISystem()
		{
#ifdef USE_DEBUG_SHAPE
			if (Instance != nullptr)
			{
				HV_LOG_WARN("UDebugShapeSystem already exists, replacing existing Instance!");
			}

			Instance = this;
			HV_LOG_INFO("GDebugUtilityShape: Instance created.");

			// FrameRate drops by 200@10kshapes if we add these:
			//	Primary Cause: Editor window iterates through all entities, 10000+ iterations is a lot.
			//				   Filtering DebugShapes improves performance by a little.	
			U64 currentNrOfEntities = scene->GetEntities().size();
			for (U16 i = 0; i < MaxShapes; i++)
			{
				Ref<SEntity> entity = scene->CreateEntity("DebugShape");
				//Ref<SEntity> entity = scene->CreateEntity("hie_DebugShape");
				scene->AddTransformComponentToEntity(entity);
				scene->AddDebugShapeComponentToEntity(entity);
			}

			size_t allocated = 
				(sizeof(SEntity) * MaxShapes) 
				+ (sizeof(SDebugShapeComponent) * MaxShapes) 
				+ (sizeof(STransformComponent) * MaxShapes);
			HV_LOG_INFO(" UDebugShapeSystem: [MaxShapes: %d] [Allocated: %d bytes]", MaxShapes, allocated);

			Init(scene, currentNrOfEntities);
#else
			scene;
#endif
		}

		UDebugShapeSystem::~UDebugShapeSystem()
		{
#ifdef USE_DEBUG_SHAPE
			if (Instance == this)
			{
				Instance = nullptr;
				HV_LOG_INFO("UDebugShapeSystem destroyed!");
			}
#endif
		}

		void UDebugShapeSystem::Update(CScene* scene)
		{
			const F32 time = GTimer::Time();
			std::vector<Ref<SDebugShapeComponent>>& debugShapes = scene->GetDebugShapeComponents();
			std::vector<U64> activeIndicesToRemove;
			for (U64 i = 0; i < Instance->ActiveIndices.size(); i++)
			{
				U64& activeIndex = Instance->ActiveIndices[i];
				Ref<SDebugShapeComponent> shape = debugShapes[activeIndex];
				if (shape->LifeTime <= time && shape->Rendered)
				{
					Instance->AvailableIndices.push(activeIndex);
					activeIndicesToRemove.push_back(i);
				}
			}

			for (U64 i = 0; i < activeIndicesToRemove.size(); i++)
			{
				if (Instance->ActiveIndices.size() <= 1)
				{
					Instance->ActiveIndices.pop_back();
					continue;
				}
				std::swap(Instance->ActiveIndices[i], Instance->ActiveIndices.back());
				Instance->ActiveIndices.pop_back();
			}
		}


		const std::vector<U64>& UDebugShapeSystem::GetActiveShapeIndices()
		{
			if (!InstanceExists())
			{
				return NoShapeIndices;
			}

			//if (!Instance->HasConnectionToScene())// Call should be unnecessary
			//{//	return NoShapeIndices; //}

			return Instance->ActiveIndices;
		}

		void UDebugShapeSystem::AddLine(const SVector& start, const SVector& end, const SVector4& color, const bool singleFrame, const F32 lifeTimeSeconds)
		{
			end; start; color; singleFrame; lifeTimeSeconds;

			if (!InstanceExists())
				return;

			U64 entityIndex = 0;
			if (!Instance->TryGetAvailableIndex(entityIndex))
				return;

			const std::vector<Ref<SEntity>>& entities = Instance->Scene->GetEntities();
			const U64 shapeIndex = entities[entityIndex]->GetComponentIndex(EComponentType::DebugShapeComponent);
			std::vector<Ref<SDebugShapeComponent>>& debugShapes = Instance->Scene->GetDebugShapeComponents();
			debugShapes[shapeIndex]->Color = color;
			debugShapes[shapeIndex]->LifeTime = LifeTimeForShape(singleFrame, lifeTimeSeconds);
			debugShapes[shapeIndex]->VertexBufferIndex = Utility::VertexBufferPrimitives::GetVertexBufferIndex<U8>(EVertexBufferPrimitives::LineShape);
			debugShapes[shapeIndex]->VertexCount = Utility::VertexBufferPrimitives::GetVertexCount<U8>(EVertexBufferPrimitives::LineShape);
			debugShapes[shapeIndex]->Rendered = false;

			std::vector<Ref<STransformComponent>>& transforms = Instance->Scene->GetTransformComponents();
			const U64 transformIndex = entities[entityIndex]->GetComponentIndex(EComponentType::TransformComponent);

			const F32 lineLength = start.Distance(end);
			const SVector transformUp = transforms[transformIndex]->Transform.GetMatrix().GetUp();
			SMatrix::Recompose(
				start, 
				SMatrix::LookAtLH(start, end, transformUp).GetEuler(), 
				SVector(1.0f, 1.0f, lineLength), 
				transforms[transformIndex]->Transform.LocalMatrix // Make setter: Parent/Child relationship affects LocalMatrix 
			);

			Instance->PrintDebugAddedShape(*debugShapes[shapeIndex].get(), singleFrame, __FUNCTION__);	
		}


		bool UDebugShapeSystem::InstanceExists()
		{
			if (Instance == nullptr)
			{
#if DEBUG_DRAWER_LOG_ERROR
				HV_LOG_ERROR("UDebugShapeSystem has not been created!");
#endif
				return false;
			}
			return true;
		}

		F32 UDebugShapeSystem::LifeTimeForShape(const bool singleFrame, const F32 requestedLifeTime)
		{
			if (singleFrame)
				return -1.0f;
			else
				return GTimer::Time() + requestedLifeTime;
		}


		void UDebugShapeSystem::Init(CScene* activeScene, U64 entityStartIndex)
		{
			Scene = activeScene;
			EntityStartIndex = entityStartIndex;
			ActiveIndices.clear();
			ResetAvailableIndices();
		}

		bool UDebugShapeSystem::HasConnectionToScene()
		{
			if (Instance->Scene == nullptr)
			{
#if DEBUG_DRAWER_LOG_ERROR
				HV_LOG_ERROR("UDebugShapeSystem is missing a reference to the active scene!");
#endif
				return false;
			}
			return true;
		}

		bool UDebugShapeSystem::TryGetAvailableIndex(U64& outIndex)
		{
			if (AvailableIndices.empty())
			{
#if DEBUG_DRAWER_LOG_ERROR
				HV_LOG_ERROR("UDebugShapeSystem: Reached MAX_DEBUG_SHAPES, no more shapes available!");
#endif
				outIndex = 0;
				return false;
			}

			outIndex = AvailableIndices.front();
			AvailableIndices.pop();
			ActiveIndices.push_back(outIndex);
			return true;
		}

		void UDebugShapeSystem::ResetAvailableIndices()
		{
			std::queue<U64> emptyQueue;
			AvailableIndices.swap(emptyQueue);
			for (U64 i = EntityStartIndex; i < (EntityStartIndex + MaxShapes); i++)
			{
				AvailableIndices.push(i);
			}
		}

		void UDebugShapeSystem::PrintDebugAddedShape(const SDebugShapeComponent& shape, const bool singleFrame, const char* callerFunction)
		{
#if DEBUG_DRAWER_LOG_ADDSHAPE
			HV_LOG_INFO("%s: Added:", callerFunction);
			HV_LOG_INFO("\tColor[%s] SingleFrame[%d] LifeTime[%5.fs] Type[%d]"
				, shape.Color.ToString().c_str()
				, singleFrame
				, shape.LifeTime
				, shape.VertexBufferIndex
			);
#else
			shape; singleFrame; callerFunction;
#endif
		}
	}
}
