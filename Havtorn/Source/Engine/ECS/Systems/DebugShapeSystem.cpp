// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugShapeSystem.h"

#include "Scene/Scene.h"
#include "ECS/Components/DebugShapeComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Graphics/VertexBufferPrimitivesUtility.h"

#include "Graphics/RenderManager.h"
#include "Graphics/RenderCommand.h"

namespace Havtorn
{
	namespace Debug
	{
		UDebugShapeSystem* UDebugShapeSystem::Instance = nullptr;

		UDebugShapeSystem::UDebugShapeSystem(CScene* scene, CRenderManager* renderManager)
			: ISystem()
		{
#ifdef USE_DEBUG_SHAPE
			if (Instance != nullptr)
			{
				HV_LOG_WARN("UDebugShapeSystem already exists, replacing existing Instance!");
			}

			Instance = this;
			HV_LOG_INFO("GDebugUtilityShape: Instance created.");

			// AG: FrameRate drops by 200@10kshapes if we add these:
			// Primary Cause: Editor window iterates through all entities, 10000+ iterations is a lot.
			// Filtering DebugShapes improves performance by a little.	
			U64 currentNrOfEntities = scene->GetEntities().size();
			for (U16 i = 0; i < MaxShapes; i++)
			{
				// hie_ prefix filters HierarchyWindow display.
				//Ref<SEntity> entity = scene->CreateEntity("DebugShape" + std::to_string(i));
				Ref<SEntity> entity = scene->CreateEntity("hie_DebugShape" + std::to_string(i));
				scene->AddTransformComponentToEntity(entity);
				scene->AddDebugShapeComponentToEntity(entity);
			}

			size_t allocated = 
				(sizeof(SEntity) * MaxShapes) 
				+ (sizeof(SDebugShapeComponent) * MaxShapes) 
				+ (sizeof(STransformComponent) * MaxShapes);
			HV_LOG_INFO(" UDebugShapeSystem: [MaxShapes: %d] [Allocated: %d bytes]", MaxShapes, allocated);

			Scene = scene;
			RenderManager = renderManager;
			EntityStartIndex = currentNrOfEntities;
			ActiveIndices.clear();
			ResetAvailableIndices();
#else
			scene; renderManager;
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
			const std::vector<Ref<SEntity>>& entities = scene->GetEntities();
			const std::vector<Ref<SDebugShapeComponent>>& debugShapes = scene->GetDebugShapeComponents();
			const std::vector<Ref<STransformComponent>>& transformComponents = scene->GetTransformComponents();

			SendRenderCommands(entities, debugShapes, transformComponents);
			CheckActiveIndices(debugShapes);
		}

		void UDebugShapeSystem::AddLine(const SVector& start, const SVector& end, const SVector4& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
		{
			if (!InstanceExists())
				return;

			U64 entityIndex = 0;
			if (!Instance->TryGetAvailableIndex(entityIndex))
				return;

			const std::vector<Ref<SEntity>>& entities = Instance->Scene->GetEntities();
			const U64 shapeIndex = entities[entityIndex]->GetComponentIndex(EComponentType::DebugShapeComponent);
			std::vector<Ref<SDebugShapeComponent>>& debugShapes = Instance->Scene->GetDebugShapeComponents();
			SetSharedDataForShape(debugShapes[shapeIndex], color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
			debugShapes[shapeIndex]->VertexBufferIndex = Utility::VertexBufferPrimitives::GetVertexBufferIndex<U8>(EVertexBufferPrimitives::LineShape);
			debugShapes[shapeIndex]->VertexCount = Utility::VertexBufferPrimitives::GetVertexCount<U8>(EVertexBufferPrimitives::LineShape);

			std::vector<Ref<STransformComponent>>& transforms = Instance->Scene->GetTransformComponents();
			const U64 transformIndex = entities[entityIndex]->GetComponentIndex(EComponentType::TransformComponent);

			const SVector transformUp = transforms[transformIndex]->Transform.GetMatrix().GetUp();
			const SVector eulerRotation = SMatrix::LookAtLH(start, end, transformUp).GetEuler();
			const F32 lineLength = start.Distance(end);
			const SVector scale = SVector(1.0f, 1.0f, lineLength);
			SMatrix matrix;
			SMatrix::Recompose(start, eulerRotation, scale, matrix);

			transforms[transformIndex]->Transform.SetMatrix(matrix);

#if DEBUG_DRAWER_LOG_ADDSHAPE
			Instance->PrintDebugAddedShape(*debugShapes[shapeIndex].get(), useLifeTime, __FUNCTION__);	
#endif
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

		F32 UDebugShapeSystem::LifeTimeForShape(const bool useLifeTime, const F32 requestedLifeTime)
		{
			if (!useLifeTime)
				return -1.0f;
			else
				return GTimer::Time() + requestedLifeTime;
		}

		F32 UDebugShapeSystem::ClampThickness(const F32 thickness)
		{
			return UMath::Clamp(thickness, ThicknessMinimum, ThicknessMaximum);
		}

		void UDebugShapeSystem::SetSharedDataForShape(Ref<SDebugShapeComponent>& inoutShape, const SVector4& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
		{
			inoutShape->Color = color;
			inoutShape->LifeTime = LifeTimeForShape(useLifeTime, lifeTimeSeconds);
			inoutShape->Thickness = ClampThickness(thickness);
			inoutShape->IgnoreDepth = ignoreDepth;
		}


		void UDebugShapeSystem::SendRenderCommands(
			const std::vector<Ref<SEntity>>& entities,
			const std::vector<Ref<SDebugShapeComponent>>& debugShapes,
			const std::vector<Ref<STransformComponent>>& transformComponents
		)
		{
			for (U64 i = 0; i < ActiveIndices.size(); i++)
			{
				std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
				const U64 shapeIndex = entities[ActiveIndices[i]]->GetComponentIndex(EComponentType::DebugShapeComponent);
				const U64 transformIndex = entities[ActiveIndices[i]]->GetComponentIndex(EComponentType::TransformComponent);
				components[static_cast<U8>(EComponentType::DebugShapeComponent)] = debugShapes[shapeIndex];
				components[static_cast<U8>(EComponentType::TransformComponent)] = transformComponents[transformIndex];

				SRenderCommand command(components, ERenderCommandType::DebugShape);
				RenderManager->PushRenderCommand(command);
			}
		}

		void UDebugShapeSystem::CheckActiveIndices(const std::vector<Ref<SDebugShapeComponent>>& debugShapes)
		{
			const F32 time = GTimer::Time();
			std::queue<U64> activeIndicesToRemove;
			for (U64 i = 0; i < ActiveIndices.size(); i++)
			{
				const U64& activeIndex = ActiveIndices[i];
				const Ref<SDebugShapeComponent>& shape = debugShapes[activeIndex];
				if (shape->LifeTime <= time)
				{
					AvailableIndices.push(activeIndex);
					activeIndicesToRemove.push(i);
				}
			}

			while (!activeIndicesToRemove.empty())
			{
				std::swap(ActiveIndices[activeIndicesToRemove.front()], ActiveIndices.back());
				ActiveIndices.pop_back();
				activeIndicesToRemove.pop();
			}
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

		void UDebugShapeSystem::PrintDebugAddedShape(const SDebugShapeComponent& shape, const bool useLifeTime, const char* callerFunction)
		{
			HV_LOG_INFO("%s: Added:", callerFunction);
			HV_LOG_INFO("\tColor[%s] UseLifeTime[%d] LifeTime[%5.fs] Type[%d]"
				, shape.Color.ToString().c_str()
				, useLifeTime
				, shape.LifeTime
				, shape.VertexBufferIndex
			);
		}
	}
}
