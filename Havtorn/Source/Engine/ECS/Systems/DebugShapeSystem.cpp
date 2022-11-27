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

#include "Core/GeneralUtilities.h"

namespace Havtorn
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

		U64 currentNrOfEntities = scene->GetEntities().size();
		for (U16 i = 0; i < MaxShapes; i++)
		{
			Ref<SEntity> entity = scene->CreateEntity("DebugShape" + std::to_string(i));
			scene->AddTransformComponentToEntity(entity);
			scene->AddDebugShapeComponentToEntity(entity);
		}

		size_t allocated = 
			(sizeof(SEntity) * MaxShapes) 
			+ (sizeof(SDebugShapeComponent) * MaxShapes) 
			+ (sizeof(STransformComponent) * MaxShapes);

		HV_LOG_INFO("UDebugShapeSystem: [MaxShapes: %d] [Allocated: %s ]", MaxShapes, UGeneralUtils::BytesAsString(allocated).c_str());

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

#pragma region AddShape
	void UDebugShapeSystem::AddLine(const SVector& start, const SVector& end, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		Ref<STransformComponent> transform;
		if (TryAddShape(EVertexBufferPrimitives::LineShape, EDefaultIndexBuffers::LineShape, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, transform))
		{
			Instance->TransformToFaceAndReach(transform->Transform.GetMatrix(), start, end);
		}	
	}

	void UDebugShapeSystem::AddArrow(const SVector& start, const SVector& end, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		Ref<STransformComponent> transform;
		if (TryAddShape(EVertexBufferPrimitives::Arrow, EDefaultIndexBuffers::Arrow, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, transform))
		{
			Instance->TransformToFaceAndReach(transform->Transform.GetMatrix(), start, end);
		}	
	}

	void UDebugShapeSystem::AddCube(const SVector& center, const SVector& scale, const SVector& eulerRotation, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		Ref<STransformComponent> transform;
		if (TryAddShape(EVertexBufferPrimitives::DebugCube, EDefaultIndexBuffers::DebugCube, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, transform))
		{
			SMatrix::Recompose(center, eulerRotation, scale, transform->Transform.GetMatrix());
		}		
	}

	void UDebugShapeSystem::AddCamera(const SVector& origin, const SVector& eulerRotation, const F32 fov, const F32 farZ, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		Ref<STransformComponent> transform;
		if (TryAddShape(EVertexBufferPrimitives::Camera, EDefaultIndexBuffers::Camera, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, transform))
		{
			F32 y = 2.0f * farZ * std::tanf(UMath::DegToRad(fov) * 0.5f);
			F32 x = 2.0f * farZ * std::tanf(UMath::DegToRad(fov) * 0.5f);
			SVector vScale(x, y, farZ);
			SMatrix::Recompose(origin, eulerRotation, vScale, transform->Transform.GetMatrix());
		}
	}

	void UDebugShapeSystem::AddCircleXY(const SVector& origin, const SVector& eulerRotation, const F32 radius, const UINT8 segments, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		AddDefaultCircle(origin, eulerRotation, radius, segments, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
	}

	void UDebugShapeSystem::AddCircleXZ(const SVector& origin, const SVector& eulerRotation, const F32 radius, const UINT8 segments, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		SVector rotation = eulerRotation + SVector(90.0f, 0.0f, 0.0f);
		AddDefaultCircle(origin, rotation, radius, segments, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
	}

	void UDebugShapeSystem::AddCircleYZ(const SVector& origin, const SVector& eulerRotation, const F32 radius, const UINT8 segments, const SColor & color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		SVector rotation = eulerRotation + SVector(0.0f, 90.0f, 0.0f);
		AddDefaultCircle(origin, rotation, radius, segments, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
	}

	void UDebugShapeSystem::AddGrid(const SVector& origin, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		Ref<STransformComponent> transform;
		if (TryAddShape(EVertexBufferPrimitives::Grid, EDefaultIndexBuffers::Grid, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, transform))
		{
			SMatrix::Recompose(origin, SVector(), SVector(1.0f), transform->Transform.GetMatrix());
		}
	}

	void UDebugShapeSystem::AddDefaultCircle(const SVector& origin, const SVector& eulerRotation, const F32 radius, const UINT8 segments, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
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

		EVertexBufferPrimitives vertexBufferPrimitive = EVertexBufferPrimitives::CircleXY16;
		EDefaultIndexBuffers indexBuffer = EDefaultIndexBuffers::CircleXY16;
		EnumFromSegment(segments, vertexBufferPrimitive, indexBuffer);
		
		Ref<STransformComponent> transform;
		if (TryAddShape(vertexBufferPrimitive, indexBuffer, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, transform))
		{
			SVector scale(radius / GeometryPrimitives::CircleXYRadius);
			SMatrix::Recompose(origin, eulerRotation, scale, transform->Transform.GetMatrix());
		}
	}
#pragma endregion !AddShape

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

	bool UDebugShapeSystem::TryAddShape(const EVertexBufferPrimitives vertexBuffer, const EDefaultIndexBuffers indexBuffer, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth, Ref<STransformComponent>& outTransform)
	{
		if (!InstanceExists())
			return false;

		U64 entityIndex = 0;
		if (!Instance->TryGetAvailableIndex(entityIndex))
			return false;

		const std::vector<Ref<SEntity>>& entities = Instance->Scene->GetEntities();

		const U64 shapeIndex = entities[entityIndex]->GetComponentIndex(EComponentType::DebugShapeComponent);
		std::vector<Ref<SDebugShapeComponent>>& debugShapes = Instance->Scene->GetDebugShapeComponents();
		debugShapes[shapeIndex]->Color = color;
		debugShapes[shapeIndex]->LifeTime = LifeTimeForShape(useLifeTime, lifeTimeSeconds);
		debugShapes[shapeIndex]->Thickness = ClampThickness(thickness);
		debugShapes[shapeIndex]->IgnoreDepth = ignoreDepth;
		debugShapes[shapeIndex]->VertexBufferIndex = static_cast<U8>(vertexBuffer);
		debugShapes[shapeIndex]->IndexCount = UGraphicsUtils::GetIndexCount<U8>(vertexBuffer);
		debugShapes[shapeIndex]->IndexBufferIndex = static_cast<U8>(indexBuffer);

		std::vector<Ref<STransformComponent>>& transforms = Instance->Scene->GetTransformComponents();
		const U64 transformIndex = entities[entityIndex]->GetComponentIndex(EComponentType::TransformComponent);
		outTransform = transforms[transformIndex];
		
		return true;
	}

	void UDebugShapeSystem::TransformToFaceAndReach(SMatrix& transform, const SVector& start, const SVector& end)
	{
		const SVector transformUp = transform.GetUp();
		const SVector eulerRotation = SMatrix::LookAtLH(start, end, transformUp).GetEuler();
		const F32 lineLength = start.Distance(end);
		const SVector scale = SVector(1.0f, 1.0f, lineLength);
		SMatrix::Recompose(start, eulerRotation, scale, transform);
	}


	void UDebugShapeSystem::SendRenderCommands(
		const std::vector<Ref<SEntity>>& entities,
		const std::vector<Ref<SDebugShapeComponent>>& debugShapes,
		const std::vector<Ref<STransformComponent>>& transformComponents
	)
	{
		typedef std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> Components;

		// Send and set prepass rendercommand for debug shapes
		{
			Components components;
			SRenderCommand command(components, ERenderCommandType::PreDebugShape);
			RenderManager->PushRenderCommand(command);

			command.Type = ERenderCommandType::PostToneMappingIgnoreDepth;
			RenderManager->PushRenderCommand(command);

			command.Type = ERenderCommandType::PostToneMappingUseDepth;
			RenderManager->PushRenderCommand(command);
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

			ERenderCommandType commandType = (debugShapes[shapeIndex]->IgnoreDepth) ? ERenderCommandType::DebugShapeIgnoreDepth : ERenderCommandType::DebugShapeUseDepth;

			SRenderCommand command(components, commandType);
			RenderManager->PushRenderCommand(command);
		}
	}

	void UDebugShapeSystem::CheckActiveIndices(const std::vector<Ref<SDebugShapeComponent>>& debugShapes)
	{
		const F32 time = GTime::Time();
		std::vector<U64> activeIndicesToRemove;
		activeIndicesToRemove.reserve(ActiveIndices.size());
		for (U64 i = 0; i < ActiveIndices.size(); i++)
		{
			const U64& activeIndex = ActiveIndices[i];
			if (debugShapes[activeIndex]->LifeTime <= time)
			{
				AvailableIndices.push(activeIndex);
				activeIndicesToRemove.push_back(i);
			}
		}

		std::sort(activeIndicesToRemove.begin(), activeIndicesToRemove.end());

		while (!activeIndicesToRemove.empty())
		{
			if (ActiveIndices.size() > 1)
			{
				std::swap(ActiveIndices[activeIndicesToRemove.back()], ActiveIndices.back());
			}

			ActiveIndices.pop_back();
			activeIndicesToRemove.pop_back();
		}
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
}
