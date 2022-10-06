// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugUtilityShape.h"

#include "Scene/Scene.h"
#include "ECS/Components/DebugShapeComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Graphics/VertexBufferPrimitivesUtility.h"

namespace Havtorn
{
	namespace Debug
	{
		GDebugUtilityShape* GDebugUtilityShape::Instance = nullptr;
		std::vector<U64> GDebugUtilityShape::NoShapeIndices;

		void GDebugUtilityShape::AddLine(const SVector& start, const SVector& end, const SVector4& color, const bool singleFrame, const F32 lifeTimeSeconds)
		{
			end; start;
			if (!InstanceExists())
				return;

			if (!Instance->HasConnectionToScene())
				return;
			U64 entityIndex = 0;
			if (!Instance->TryGetAvailableIndex(entityIndex))
				return;

			const std::vector<Ref<SEntity>>& entities = Instance->ActiveScene->GetEntities();
			const U64 shapeIndex = entities[entityIndex]->GetComponentIndex(EComponentType::DebugShapeComponent);
			std::vector<Ref<SDebugShapeComponent>>& debugShapes = Instance->ActiveScene->GetDebugShapeComponents();
			debugShapes[shapeIndex]->Color = color;
			debugShapes[shapeIndex]->LifeTime = LifeTimeForShape(singleFrame, lifeTimeSeconds);
			debugShapes[shapeIndex]->VertexBufferIndex = Utility::VertexBufferPrimitives::GetVertexBufferIndex<U8>(EVertexBufferPrimitives::LineShape);
			debugShapes[shapeIndex]->VertexCount = Utility::VertexBufferPrimitives::GetVertexCount<U8>(EVertexBufferPrimitives::LineShape);
			
			std::vector<Ref<STransformComponent>>& transforms = Instance->ActiveScene->GetTransformComponents();
			const U64 transformIndex = entities[entityIndex]->GetComponentIndex(EComponentType::TransformComponent);
			
			//Rotate to face
			 
			// Scale to reach
			//F32 length = start.Distance(end);
			//transforms[transformIndex]->Transform.Scale(length);
			
			transforms[transformIndex]->Transform.Move(start);

			Instance->PrintDebugAddedShape(*debugShapes[shapeIndex].get(), singleFrame, __FUNCTION__);	
		}

		void GDebugUtilityShape::PrintDebugAddedShape(const SDebugShapeComponent& shape, const bool singleFrame, const char* function)
		{
#if DEBUG_DRAWER_LOG_ADDSHAPE
			HV_LOG_INFO("%s: Added:", function);
			HV_LOG_INFO("\tColor[%s] SingleFrame[%d] LifeTime[%5.fs] Type[%d]"
				, shape.Color.ToString().c_str()
				, singleFrame
				, shape.LifeTime
				, shape.VertexBufferIndex
			);
#else
			shape; singleFrame; function;
#endif
		}

		bool GDebugUtilityShape::TryGetAvailableIndex(U64& outIndex)
		{
			if (AvailableIndices.empty())
			{
#if DEBUG_DRAWER_LOG_ERROR
				HV_LOG_ERROR("GDebugUtilityShape: Reached MAX_DEBUG_SHAPES, no more shapes available!");
#endif
				outIndex = 0;
				return false;
			}

			outIndex = AvailableIndices.front();
			AvailableIndices.pop();
			ActiveIndices.push_back(outIndex);
			return true;
		}

		void GDebugUtilityShape::Init(CScene* activeScene, U64 entityStartIndex)
		{
			if (!InstanceExists())
				return;

			Instance->ActiveScene = activeScene;
			Instance->EntityStartIndex = entityStartIndex;
			Instance->ActiveIndices.clear();
			Instance->ResetAvailableIndices();
		}

		void GDebugUtilityShape::Update()
		{
			if (!InstanceExists())
				return;
			if (!Instance->HasConnectionToScene())
				return;

			const F32 time = GTimer::Time();
			std::vector<Ref<SDebugShapeComponent>>& debugShapes = Instance->ActiveScene->GetDebugShapeComponents();
			std::vector<U64> activeIndicesToRemove;
			for (U64 i = 0; i < Instance->ActiveIndices.size(); i++)
			{
				U64& activeIndex = Instance->ActiveIndices[i];
				Ref<SDebugShapeComponent> shape = debugShapes[activeIndex];
				if (shape->LifeTime <= time)
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

		void GDebugUtilityShape::ResetAvailableIndices()
		{
			std::queue<U64> emptyQueue;
			AvailableIndices.swap(emptyQueue);
			for (U64 i = EntityStartIndex; i < (EntityStartIndex + MaxDebugShapes); i++)
			{
				AvailableIndices.push(i);
			}
		}

		GDebugUtilityShape::GDebugUtilityShape()
		{
#ifdef USE_DEBUG_SHAPE
			if (Instance == nullptr)
			{
				Instance = this;
				HV_LOG_INFO("GDebugUtilityShape created!");
			}
#endif
		}

		GDebugUtilityShape::~GDebugUtilityShape()
		{
#ifdef USE_DEBUG_SHAPE
			if (Instance == this)
			{
				Instance = nullptr;
				HV_LOG_INFO("GDebugUtilityShape destroyed!");
			}
#endif
		}

		const std::vector<U64>& GDebugUtilityShape::GetActiveShapeIndices()
		{
			if (!InstanceExists())
			{
				return NoShapeIndices;
			}

			if (!Instance->HasConnectionToScene())
			{
				return NoShapeIndices;
			}

			return Instance->ActiveIndices;
		}

		bool GDebugUtilityShape::InstanceExists()
		{
			if (Instance == nullptr)
			{
#if DEBUG_DRAWER_LOG_ERROR
				HV_LOG_ERROR("GDebugUtilityShape has not been created!");
#endif
				return false;
			}
			return true;
		}

		bool GDebugUtilityShape::HasConnectionToScene()
		{
			if (Instance->ActiveScene == nullptr)
			{
#if DEBUG_DRAWER_LOG_ERROR
				HV_LOG_ERROR("GDebugUtilityShape is missing a reference to the active scene!");
#endif
				return false;
			}
			return true;
		}

		F32 GDebugUtilityShape::LifeTimeForShape(const bool singleFrame, const F32 requestedLifeTime)
		{
			if (singleFrame)
				return -1.0f;
			else
				return GTimer::Time() + requestedLifeTime;
		}
	}
}

