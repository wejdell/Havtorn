// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugShapeSystem.h"

#include "Scene/Scene.h"
#include "ECS/Components/DebugShapeComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Graphics/GeometryPrimitives.h"
#include "Graphics/GeometryPrimitivesUtility.h"

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
			debugShapes[shapeIndex]->VertexBufferIndex = static_cast<U8>(EVertexBufferPrimitives::LineShape);
			debugShapes[shapeIndex]->IndexCount = static_cast<U8>(UGraphicsUtils::GetIndexCount(EVertexBufferPrimitives::LineShape));
			debugShapes[shapeIndex]->IndexBufferIndex = static_cast<U8>(EDefaultIndexBuffers::LineShape);

			std::vector<Ref<STransformComponent>>& transforms = Instance->Scene->GetTransformComponents();
			const U64 transformIndex = entities[entityIndex]->GetComponentIndex(EComponentType::TransformComponent);
			TransformToFaceAndReach(transforms[transformIndex], start, end);
		}

		void UDebugShapeSystem::AddArrow(const SVector& start, const SVector& end, const SVector4& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
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
			debugShapes[shapeIndex]->VertexBufferIndex = static_cast<U8>(EVertexBufferPrimitives::FlatArrow);
			debugShapes[shapeIndex]->IndexCount = static_cast<U8>(UGraphicsUtils::GetIndexCount(EVertexBufferPrimitives::FlatArrow));
			debugShapes[shapeIndex]->IndexBufferIndex = static_cast<U8>(EDefaultIndexBuffers::FlatArrow);

			std::vector<Ref<STransformComponent>>& transforms = Instance->Scene->GetTransformComponents();
			const U64 transformIndex = entities[entityIndex]->GetComponentIndex(EComponentType::TransformComponent);
			TransformToFaceAndReach(transforms[transformIndex], start, end);
		}

		void UDebugShapeSystem::AddCube(const SVector& center, const SVector& scale, const SVector& eulerRotation, const SVector4& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
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
			debugShapes[shapeIndex]->VertexBufferIndex = static_cast<U8>(EVertexBufferPrimitives::DebugCube);
			debugShapes[shapeIndex]->IndexCount = static_cast<U8>(UGraphicsUtils::GetIndexCount(EVertexBufferPrimitives::DebugCube));
			debugShapes[shapeIndex]->IndexBufferIndex = static_cast<U8>(EDefaultIndexBuffers::DebugCube);

			std::vector<Ref<STransformComponent>>& transforms = Instance->Scene->GetTransformComponents();
			const U64 transformIndex = entities[entityIndex]->GetComponentIndex(EComponentType::TransformComponent);
			
			SMatrix matrix;
			SMatrix::Recompose(center, eulerRotation, scale, matrix);
			transforms[transformIndex]->Transform.SetMatrix(matrix);
		}

		void UDebugShapeSystem::AddCamera(const SVector& origin, const SVector& eulerRotation, const F32 fov, const F32 farZ, const SVector4& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
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
			debugShapes[shapeIndex]->VertexBufferIndex = static_cast<U8>(EVertexBufferPrimitives::Camera);
			debugShapes[shapeIndex]->IndexCount = static_cast<U8>(UGraphicsUtils::GetIndexCount(EVertexBufferPrimitives::Camera));
			debugShapes[shapeIndex]->IndexBufferIndex = static_cast<U8>(EDefaultIndexBuffers::Camera);

			std::vector<Ref<STransformComponent>>& transforms = Instance->Scene->GetTransformComponents();
			const U64 transformIndex = entities[entityIndex]->GetComponentIndex(EComponentType::TransformComponent);

			F32 y = 2.0f * farZ * std::tanf(UMath::DegToRad(fov) * 0.5f);
			F32 x = 2.0f * farZ * std::tanf(UMath::DegToRad(fov) * 0.5f);
			SVector vScale(x, y, farZ);
			SMatrix& matrix = transforms[transformIndex]->Transform.GetMatrix();
			SMatrix::Recompose(origin, eulerRotation, vScale, matrix);
		}

		void UDebugShapeSystem::AddCircleXY(const SVector& origin, const SVector& eulerRotation, const F32 radius, const UINT8 segments, const SVector4& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
		{
			AddDefaultCircle(origin, eulerRotation, radius, segments, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
		}

		void UDebugShapeSystem::AddCircleXZ(const SVector& origin, const SVector& eulerRotation, const F32 radius, const UINT8 segments, const SVector4& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
		{
			SVector rotation = eulerRotation + SVector(90.0f, 0.0f, 0.0f);
			AddDefaultCircle(origin, rotation, radius, segments, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
		}

		void UDebugShapeSystem::AddCircleYZ(const SVector& origin, const SVector& eulerRotation, const F32 radius, const UINT8 segments, const SVector4 & color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
		{
			SVector rotation = eulerRotation + SVector(0.0f, 90.0f, 0.0f);
			AddDefaultCircle(origin, rotation, radius, segments, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
		}

		void UDebugShapeSystem::AddDefaultCircle(const SVector& origin, const SVector& eulerRotation, const F32 radius, const UINT8 segments, const SVector4& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
		{
			if (!InstanceExists())
				return;

			U64 entityIndex = 0;
			if (!Instance->TryGetAvailableIndex(entityIndex))
				return;

			// AG. The Default Circle is across the XY plane
			auto EnumFromSegment = [&](const UINT8& s, EVertexBufferPrimitives& v, EDefaultIndexBuffers& i)
			{
				if (s >= 16 - (16 / 4) && s <= 16 + (16 / 4))
				{
					v = EVertexBufferPrimitives::CircleXY16;
					i = EDefaultIndexBuffers::CircleXY16;
					return;
				}

				if (s < 16)
				{
					v = EVertexBufferPrimitives::CircleXY8;
					i = EDefaultIndexBuffers::CircleXY8;
				}
				else
				{
					v = EVertexBufferPrimitives::CircleXY32;
					i = EDefaultIndexBuffers::CircleXY32;
				}
			};

			const std::vector<Ref<SEntity>>& entities = Instance->Scene->GetEntities();

			const U64 shapeIndex = entities[entityIndex]->GetComponentIndex(EComponentType::DebugShapeComponent);
			std::vector<Ref<SDebugShapeComponent>>& debugShapes = Instance->Scene->GetDebugShapeComponents();
			SetSharedDataForShape(debugShapes[shapeIndex], color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
		
			EVertexBufferPrimitives vertexBufferPrimitive = EVertexBufferPrimitives::CircleXY16;
			EDefaultIndexBuffers indexBuffer = EDefaultIndexBuffers::CircleXY16;
			EnumFromSegment(segments, vertexBufferPrimitive, indexBuffer);
			debugShapes[shapeIndex]->VertexBufferIndex = static_cast<U8>(vertexBufferPrimitive);
			debugShapes[shapeIndex]->IndexCount = static_cast<U8>(UGraphicsUtils::GetIndexCount(vertexBufferPrimitive));
			debugShapes[shapeIndex]->IndexBufferIndex = static_cast<U8>(indexBuffer);

			std::vector<Ref<STransformComponent>>& transforms = Instance->Scene->GetTransformComponents();
			const U64 transformIndex = entities[entityIndex]->GetComponentIndex(EComponentType::TransformComponent);
			SMatrix& matrix = transforms[transformIndex]->Transform.GetMatrix();
			SVector scale(radius / GeometryPrimitives::CircleXYRadius);
			SMatrix::Recompose(origin, eulerRotation, scale, matrix);
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
				return GTime::Time() + requestedLifeTime;
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

		void UDebugShapeSystem::TransformToFaceAndReach(Ref<STransformComponent>& inoutTransform, const SVector& start, const SVector& end)
		{
			const SVector transformUp = inoutTransform->Transform.GetMatrix().GetUp();
			const SVector eulerRotation = SMatrix::LookAtLH(start, end, transformUp).GetEuler();
			const F32 lineLength = start.Distance(end);
			const SVector scale = SVector(1.0f, 1.0f, lineLength);
			SMatrix matrix;
			SMatrix::Recompose(start, eulerRotation, scale, matrix);
			inoutTransform->Transform.SetMatrix(matrix);
		}


		void UDebugShapeSystem::SendRenderCommands(
			const std::vector<Ref<SEntity>>& entities,
			const std::vector<Ref<SDebugShapeComponent>>& debugShapes,
			const std::vector<Ref<STransformComponent>>& transformComponents
		)
		{
			typedef std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> Components;

			// Send and set prepass rendercommand for debug shapes
			std::vector<SRenderCommand> ignoreDepth;
			ignoreDepth.reserve(ActiveIndices.size());
			std::vector<SRenderCommand> useDepth;
			useDepth.reserve(ActiveIndices.size());
			{
				Components components;
				SRenderCommand command(components, ERenderCommandType::PreDebugShape);
				RenderManager->PushRenderCommand(command);

				ignoreDepth.emplace_back(SRenderCommand(components, ERenderCommandType::PostToneMappingIgnoreDepth));
				useDepth.emplace_back(SRenderCommand(components, ERenderCommandType::PostToneMappingUseDepth));
			}

			// Sort render commands based on use of depth
			const U8 debugShapeComponent = static_cast<U8>(EComponentType::DebugShapeComponent);
			const U8 transformComponent = static_cast<U8>(EComponentType::TransformComponent);
			for (U64 i = 0; i < ActiveIndices.size(); i++)
			{
				//const ERenderCommandType renderCommandType = ERenderCommandType::DebugShape;
				const U64 shapeIndex = entities[ActiveIndices[i]]->GetComponentIndex(EComponentType::DebugShapeComponent);
				const U64 transformIndex = entities[ActiveIndices[i]]->GetComponentIndex(EComponentType::TransformComponent);
				Components components;
				components[debugShapeComponent] = debugShapes[shapeIndex];
				components[transformComponent] = transformComponents[transformIndex];

				if (debugShapes[shapeIndex]->IgnoreDepth)
					ignoreDepth.emplace_back(SRenderCommand(components, ERenderCommandType::DebugShapeIgnoreDepth));
				else
					useDepth.emplace_back(SRenderCommand(components, ERenderCommandType::DebugShapeUseDepth));
			}

			auto SendRenderCommands = [&](std::vector<SRenderCommand>& commands)
			{
				for (U64 i = 0; i < commands.size(); i++)
				{
					RenderManager->PushRenderCommand(commands[i]);
				}
			};
			SendRenderCommands(useDepth);
			SendRenderCommands(ignoreDepth);
		}

		void UDebugShapeSystem::CheckActiveIndices(const std::vector<Ref<SDebugShapeComponent>>& debugShapes)
		{
			const F32 time = GTime::Time();
			std::vector<U64> activeIndicesToRemove;
			for (U64 i = 0; i < ActiveIndices.size(); i++)
			{
				const U64& activeIndex = ActiveIndices[i];
				const Ref<SDebugShapeComponent>& shape = debugShapes[activeIndex];
				if (shape->LifeTime <= time)
				{
					AvailableIndices.push(activeIndex);
					activeIndicesToRemove.push_back(i);
				}
			}

			std::sort(activeIndicesToRemove.begin(), activeIndicesToRemove.end());

			while (!activeIndicesToRemove.empty())
			{
				if (ActiveIndices.size() == 1)
				{
					ActiveIndices.pop_back();
					activeIndicesToRemove.pop_back();
					break;
				}

				std::swap(ActiveIndices[activeIndicesToRemove.back()], ActiveIndices.back());
				ActiveIndices.pop_back();
				activeIndicesToRemove.pop_back();
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
