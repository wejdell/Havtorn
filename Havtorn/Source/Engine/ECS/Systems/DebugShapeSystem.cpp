// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugShapeSystem.h"

#include "Scene/Scene.h"
#include "ECS/Components/DebugShapeComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Graphics/GeometryPrimitives.h"

#include "Graphics/RenderManager.h"
#include "Graphics/RenderCommand.h"

#include "Core/GeneralUtilities.h"

namespace Havtorn
{
	UDebugShapeSystem* UDebugShapeSystem::Instance = nullptr;

	const std::map<EVertexBufferPrimitives, const SPrimitive&> UDebugShapeSystem::Shapes = 
	{
		{ EVertexBufferPrimitives::Line, GeometryPrimitives::Line},
		{ EVertexBufferPrimitives::Pyramid, GeometryPrimitives::Pyramid},
		{ EVertexBufferPrimitives::BoundingBox, GeometryPrimitives::BoundingBox},
		{ EVertexBufferPrimitives::Camera, GeometryPrimitives::Camera},
		{ EVertexBufferPrimitives::Circle8, GeometryPrimitives::Circle8},
		{ EVertexBufferPrimitives::Circle16, GeometryPrimitives::Circle16},
		{ EVertexBufferPrimitives::Circle32, GeometryPrimitives::Circle32},
		{ EVertexBufferPrimitives::HalfCircle16, GeometryPrimitives::HalfCircle16},
		{ EVertexBufferPrimitives::Grid, GeometryPrimitives::Grid},
		{ EVertexBufferPrimitives::Axis, GeometryPrimitives::Axis},
		{ EVertexBufferPrimitives::Octahedron, GeometryPrimitives::Octahedron},
		{ EVertexBufferPrimitives::Square, GeometryPrimitives::Square},
		{ EVertexBufferPrimitives::UVSphere, GeometryPrimitives::UVSphere},
	};

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
			//Ref<SEntity> entity = scene->CreateEntity("DebugShape" + std::to_string(i));
			//scene->AddTransformComponentToEntity(entity);
			//scene->AddDebugShapeComponentToEntity(entity);
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

	void UDebugShapeSystem::Update(CScene* /*scene*/)
	{
		//const std::vector<Ref<SEntity>>& entities = scene->GetEntities();
		//const std::vector<Ref<SDebugShapeComponent>>& debugShapes = scene->GetDebugShapeComponents();
		//const std::vector<Ref<STransformComponent>>& transformComponents = scene->GetTransformComponents();

		//SendRenderCommands(entities, debugShapes, transformComponents);
		//CheckActiveIndices(debugShapes);
	}

#pragma region AddShape
	void UDebugShapeSystem::AddLine(const SVector& /*start*/, const SVector& /*end*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
		//std::vector<SShapeData> shapes = { SShapeData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line)};
		//if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		//{
		//	Instance->TransformToFaceAndReach(start, end, shapes[0].Transform->Transform.GetMatrix());
		//}	
	}

	void UDebugShapeSystem::AddArrow(const SVector& /*start*/, const SVector& /*end*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
		//std::vector<SShapeData> shapes = { 
		//	SShapeData(EVertexBufferPrimitives::Pyramid, EDefaultIndexBuffers::Pyramid),
		//	SShapeData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line)
		//};
		//if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		//{
		//	SMatrix& lineTransform = shapes[1].Transform->Transform.GetMatrix();
		//	Instance->TransformToFaceAndReach(start, end, lineTransform);
		//	
		//	const F32 scale = 0.1f;
		//	const SVector pyramidPos = end - lineTransform.GetForward().GetNormalized() * 0.1f;
		//	// Default pyramid's height is along the Y axis, rotation offset of 90 degrees around X places it along the Z axis.
		//	SMatrix::Recompose(pyramidPos, lineTransform.GetEuler() + SVector(90.0f, 0.0f, 0.0f), scale, shapes[0].Transform->Transform.GetMatrix());
		//}	
	}

	void UDebugShapeSystem::AddCube(const SVector& /*center*/, const SVector& /*eulerRotation*/, const SVector& /*scale*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
/*		std::vector<SShapeData> shapes = { SShapeData(EVertexBufferPrimitives::BoundingBox, EDefaultIndexBuffers::BoundingBox)};
		if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		{
			SMatrix::Recompose(center, eulerRotation, scale, shapes[0].Transform->Transform.GetMatrix());
		}*/		
	}

	void UDebugShapeSystem::AddCamera(const SVector& /*origin*/, const SVector& /*eulerRotation*/, const F32 /*fov*/, const F32 /*farZ*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
		//std::vector<SShapeData> shapes = { SShapeData(EVertexBufferPrimitives::Camera, EDefaultIndexBuffers::Camera)};
		//if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		//{
		//	// TODO.AG: Rework this. Does not seem to properly represent fov & farZ. Might have to use aspectratio?
		//	F32 y = 2.0f * farZ * std::tanf(UMath::DegToRad(fov) * 0.5f);
		//	F32 x = 2.0f * farZ * std::tanf(UMath::DegToRad(fov) * 0.5f);
		//	SVector vScale(x, y, farZ);
		//	SMatrix::Recompose(origin, eulerRotation, vScale, shapes[0].Transform->Transform.GetMatrix());
		//}
	}

	void UDebugShapeSystem::AddCircle(const SVector& /*origin*/, const SVector& /*eulerRotation*/, const F32 /*radius*/, const UINT8 /*segments*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
		//// AG. The Default Circle is across the XZ plane
		//EVertexBufferPrimitives vertexBufferPrimitive = EVertexBufferPrimitives::Circle16;
		//EDefaultIndexBuffers indexBuffer = EDefaultIndexBuffers::Circle16;

		//// if requested segments are closer to 32
		//if (segments > 24)
		//{
		//	vertexBufferPrimitive = EVertexBufferPrimitives::Circle32;
		//	indexBuffer = EDefaultIndexBuffers::Circle32;
		//}
		//// if requested segments are closer to 8
		//else if (segments < 12)
		//{
		//	vertexBufferPrimitive = EVertexBufferPrimitives::Circle8;
		//	indexBuffer = EDefaultIndexBuffers::Circle8;
		//}

		//std::vector<SShapeData> shapes = { SShapeData(vertexBufferPrimitive, indexBuffer)};
		//if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		//{
		//	SVector scale(radius / GeometryPrimitives::CircleRadius);
		//	SMatrix::Recompose(origin, eulerRotation, scale, shapes[0].Transform->Transform.GetMatrix());
		//}
	}

	void UDebugShapeSystem::AddGrid(const SVector& /*origin*/, const SVector& /*eulerRotation*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
		//std::vector<SShapeData> shapes = { SShapeData(EVertexBufferPrimitives::Grid, EDefaultIndexBuffers::Grid)};
		//if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		//{
		//	SMatrix::Recompose(origin, eulerRotation, SVector(1.0f), shapes[0].Transform->Transform.GetMatrix());
		//}
	}

	void UDebugShapeSystem::AddAxis(const SVector& /*origin*/, const SVector& /*eulerRotation*/, const SVector& /*scale*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
		//std::vector<SShapeData> shapes = { SShapeData(EVertexBufferPrimitives::Axis, EDefaultIndexBuffers::Axis)};
		//if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		//{
		//	SMatrix::Recompose(origin, eulerRotation, scale, shapes[0].Transform->Transform.GetMatrix());
		//}
	}

	void UDebugShapeSystem::AddPoint(const SVector& /*origin*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
		//std::vector<SShapeData> shapes = { SShapeData(EVertexBufferPrimitives::Octahedron, EDefaultIndexBuffers::Octahedron)};
		//if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		//{
		//	SMatrix::Recompose(origin, SVector(), SVector(0.1f), shapes[0].Transform->Transform.GetMatrix());
		//}
	}

	void UDebugShapeSystem::AddRectangle(const SVector& /*center*/, const SVector& /*eulerRotation*/, const SVector& /*scale*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
		//std::vector<SShapeData> shapes = { SShapeData(EVertexBufferPrimitives::Square, EDefaultIndexBuffers::Square)};
		//if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		//{
		//	SMatrix::Recompose(center, eulerRotation, scale, shapes[0].Transform->Transform.GetMatrix());
		//}
	}

	void UDebugShapeSystem::AddSphere(const SVector& /*center*/, const SVector& /*eulerRotation*/, const SVector& /*scale*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
		//std::vector<SShapeData> shapes = { SShapeData(EVertexBufferPrimitives::UVSphere, EDefaultIndexBuffers::UVSphere)};
		//if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		//{
		//	SMatrix::Recompose(center, eulerRotation, scale, shapes[0].Transform->Transform.GetMatrix());
		//}
	}

	void UDebugShapeSystem::AddConeRadius(const SVector& /*apexPosition*/, const SVector& /*direction*/, const F32 /*height*/, const F32 /*radius*/, const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/)
	{
		//std::vector<SShapeData> shapes = { 
		//	SShapeData(EVertexBufferPrimitives::Circle16, EDefaultIndexBuffers::Circle16),
		//	SShapeData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line),
		//	SShapeData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line),
		//	SShapeData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line),
		//	SShapeData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line),
		//};
		//if (TryAddShape(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, shapes))
		//{
		//	const SVector base = apexPosition + direction.GetNormalized() * height;

		//	const SVector scale(radius / GeometryPrimitives::CircleRadius);
		//	const SVector up = direction.IsEqual(SVector::Up) ? SVector::Forward : SVector::Up;
		//	const SMatrix lookAt = SMatrix::Face(apexPosition, direction, up);
		//	
		//	// Default circle lies on the XZ plane, adding a rotation offset of 90degrees around X rotates it to the XY plane.
		//	SMatrix::Recompose(base, lookAt.GetEuler() + SVector(90.0f, 0.0f, 0.0f), scale, shapes[0].Transform->Transform.GetMatrix());

		//	const SVector lookAtRight = lookAt.GetRight();
		//	const SVector lookAtUp = lookAt.GetUp();
		//	Instance->TransformToFaceAndReach(apexPosition, base + lookAtRight * radius, shapes[1].Transform->Transform.GetMatrix());
		//	Instance->TransformToFaceAndReach(apexPosition, base + lookAtRight * -radius, shapes[2].Transform->Transform.GetMatrix());
		//	Instance->TransformToFaceAndReach(apexPosition, base + lookAtUp * radius, shapes[3].Transform->Transform.GetMatrix());
		//	Instance->TransformToFaceAndReach(apexPosition, base + lookAtUp * -radius, shapes[4].Transform->Transform.GetMatrix());

		//}
	}

	void UDebugShapeSystem::AddConeAngle(const SVector& apexPosition, const SVector& direction, const F32 height, const F32 angleDegrees, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		const F32 radius = height * UMath::Sin(UMath::DegToRad(angleDegrees));
		AddConeRadius(apexPosition, direction, height, radius, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
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

	bool UDebugShapeSystem::TryAddShape(const SColor& /*color*/, const F32 /*lifeTimeSeconds*/, const bool /*useLifeTime*/, const F32 /*thickness*/, const bool /*ignoreDepth*/, std::vector<SShapeData>& /*outShapes*/)
	{
		//if (!InstanceExists())
		//	return false;

		//std::vector<U64> indices;
		//if (!Instance->TryGetAvailableIndices(outShapes.size(), indices))
		//	return false;

		//const std::vector<Ref<SEntity>>& entities = Instance->Scene->GetEntities();

		//for (U64 i = 0u; i < indices.size(); i++)
		//{
		//	const U64 entityIndex = indices[i];
		//	const U64 shapeIndex = entities[entityIndex]->GetComponentIndex(EComponentType::DebugShapeComponent);
		//	std::vector<Ref<SDebugShapeComponent>>& debugShapes = Instance->Scene->GetDebugShapeComponents();
		//	debugShapes[shapeIndex]->Color = color;
		//	debugShapes[shapeIndex]->LifeTime = useLifeTime ? (GTime::Time() + lifeTimeSeconds) : -1.0f;
		//	debugShapes[shapeIndex]->Thickness = UMath::Clamp(thickness, ThicknessMinimum, ThicknessMaximum);
		//	debugShapes[shapeIndex]->IgnoreDepth = ignoreDepth;
		//	debugShapes[shapeIndex]->VertexBufferIndex = static_cast<U8>(outShapes[i].VertexBuffer);
		//	debugShapes[shapeIndex]->IndexCount = static_cast<U16>(Shapes.at(outShapes[i].VertexBuffer).Indices.size());
		//	debugShapes[shapeIndex]->IndexBufferIndex = static_cast<U8>(outShapes[i].IndexBuffer);

		//	std::vector<Ref<STransformComponent>>& transforms = Instance->Scene->GetTransformComponents();
		//	const U64 transformIndex = entities[entityIndex]->GetComponentIndex(EComponentType::TransformComponent);
		//	outShapes[i].Transform = transforms[transformIndex];
		//}

		return true;
	}

	void UDebugShapeSystem::TransformToFaceAndReach(const SVector& start, const SVector& end, SMatrix& transform)
	{
		// AG: Using SVector::Forward as up works for some reason?
		const SVector up = SVector::Forward;// SVector::Up: breaks up == direction.
		const SVector direction = (end - start).GetNormalized();
		const SVector scale = SVector(1.0f, 1.0f, start.Distance(end));
		transform = SMatrix::Face(start, direction, up);
		SMatrix::Recompose(start, transform.GetEuler(), scale, transform);
	}


	void UDebugShapeSystem::SendRenderCommands(
		const std::vector<Ref<SEntity>>& /*entities*/,
		const std::vector<Ref<SDebugShapeComponent>>& /*debugShapes*/,
		const std::vector<Ref<STransformComponent>>& /*transformComponents*/
	)
	{
		typedef std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> Components;

		// Send and set prepass rendercommand for debug shapes
		{
			//Components components;
			//SRenderCommand command(components, ERenderCommandType::PreDebugShape);
			//RenderManager->PushRenderCommand(command);

			//command.Type = ERenderCommandType::PostToneMappingIgnoreDepth;
			//RenderManager->PushRenderCommand(command);

			//command.Type = ERenderCommandType::PostToneMappingUseDepth;
			//RenderManager->PushRenderCommand(command);
		}

		// Sort render commands based on use of depth
		//const U8 debugShapeComponent = static_cast<U8>(EComponentType::DebugShapeComponent);
		//const U8 transformComponent = static_cast<U8>(EComponentType::TransformComponent);
		//Components components;
		//for (U64 i = 0; i < ActiveIndices.size(); i++)
		//{
			//const U64& activeIndex = ActiveIndices[i];
			//const U64 shapeIndex = entities[activeIndex]->GetComponentIndex(EComponentType::DebugShapeComponent);
			//const U64 transformIndex = entities[activeIndex]->GetComponentIndex(EComponentType::TransformComponent);
			//
			//components[debugShapeComponent] = debugShapes[shapeIndex];
			//components[transformComponent] = transformComponents[transformIndex];

			//ERenderCommandType commandType = (debugShapes[shapeIndex]->IgnoreDepth) ? ERenderCommandType::DebugShapeIgnoreDepth : ERenderCommandType::DebugShapeUseDepth;

			//SRenderCommand command(components, commandType);
			//RenderManager->PushRenderCommand(command);
		//}
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

	bool UDebugShapeSystem::TryGetAvailableIndices(const U64 nrOfIndices, std::vector<U64>& outIndices)
	{
		if (AvailableIndices.empty() || AvailableIndices.size() < nrOfIndices)
		{
#if DEBUG_DRAWER_LOG_ERROR
			HV_LOG_ERROR("UDebugShapeSystem: Reached MAX_DEBUG_SHAPES, no more shapes available!");
#endif
			return false;
		}

		outIndices.resize(nrOfIndices);
		for (U64 i = 0u; i < nrOfIndices; i++)
		{
			outIndices[i] = AvailableIndices.front();
			AvailableIndices.pop();
			ActiveIndices.push_back(outIndices[i]);
		}
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

#if _DEBUG
	void UDebugShapeSystem::TestAllShapes()
	{
		// TODO.AG: add Camera

		AddGrid(SVector(), SVector(), SColor::Grey, 1.0f, false, ThicknessMinimum * 4.f, false);
		AddArrow(SVector(), SVector::Right * 6.0f, SColor::Red, 1.0f, false, ThicknessMinimum  * 2.f, true);
		AddArrow(SVector(), SVector::Up * 6.0f, SColor::Green, 1.0f, false, ThicknessMinimum  * 2.f, true);
		AddArrow(SVector(), SVector::Forward * 6.0f, SColor::Blue, 1.0f, false, ThicknessMinimum  * 2.f, true);

		const F32 time = GTime::Time();
		const F32 cosTime = UMath::Cos(time);
		const F32 sinTime = UMath::Sin(time);

		static F32 previousTime = 0.0f;
		const F32 lifeTime = 2.0f;
		if (time >= (previousTime + lifeTime + 0.5f))
		{
			previousTime = time;

			const SVector posLowerBound(-3.0f);
			const SVector posUpperBound(3.0f);
			const SVector rotLowerBound(0.0f);
			const SVector rotUpperBound(180.0f);
			const SVector sclLowerBound(0.5f);
			const SVector sclUpperBound(2.0f);

			AddCube(
				SVector::Random(posLowerBound, posUpperBound), 
				SVector::Random(rotLowerBound, rotUpperBound), 
				SVector::Random(sclLowerBound, sclUpperBound), 
				SColor::Random(0, 255, 255), lifeTime, true, ThicknessMaximum, false);

			for (U8 i = 0; i < 7; i++)
			{
				AddPoint(SVector::Random(posLowerBound, posUpperBound), SColor::Magenta, lifeTime);
			}

			auto Line = [&](const SVector& pos, const SVector& rot, const SColor& axisColor, const SColor& lineColor)
			{
				SVector targetScl = SVector(cosTime, cosTime, cosTime) + SVector(0.5f);
				AddAxis(pos, rot, targetScl, axisColor, lifeTime, true, ThicknessMaximum, true);
				AddLine(SVector(), pos, lineColor, lifeTime, true, ThicknessMaximum * 0.7f, true);
			};
			Line(SVector::Random(posLowerBound, posUpperBound), SVector::Random(rotLowerBound, rotUpperBound), SColor::White, SColor::Grey);
			Line(SVector::Random(posLowerBound, posUpperBound), SVector::Random(rotLowerBound, rotUpperBound), SColor::Grey, SColor::Black);
			Line(SVector::Random(posLowerBound, posUpperBound), SVector::Random(rotLowerBound, rotUpperBound), SColor::Yellow, SColor::Orange);
			Line(SVector::Random(posLowerBound, posUpperBound), SVector::Random(rotLowerBound, rotUpperBound), SColor::Orange, SColor::Red);

			const SVector coneDirection = SVector::Random(posLowerBound, posUpperBound) - SVector::Random(rotLowerBound, rotUpperBound);
			AddConeAngle({ -5.0f, 0.0f, 5.0f }, coneDirection, UMath::Random(0.5f, 2.0f), UMath::Random(UMath::DegToRad(10.0f), UMath::DegToRad(90.0f)), SColor::White, lifeTime, true, ThicknessMinimum * 2.0f, false);
		}
		

		const F32 radiusIncrement = (0.5f * sinTime);
		auto Circle = [&](UINT8 segments, F32 radius, const SVector& rotation, const SColor& color)
		{
			AddCircle(SVector(), rotation, radius + radiusIncrement, segments, color, lifeTime, false, ThicknessMaximum, false);
		};
		Circle(static_cast<UINT8>(UMath::Random(0, 11)), 2.0f, { 0.0f, 180.0f * sinTime, 90.0f }, SColor::Red);
		Circle(static_cast<UINT8>(UMath::Random(12, 24)), 2.25f, { 0.0f, 180.0f * sinTime, 0.0f }, SColor::Green);
		Circle(static_cast<UINT8>(UMath::Random(25, 33)), 2.5f, { 90.0f, 0.0f, 180.0f * sinTime },SColor::Blue);
		AddSphere(SVector(), SVector(), SVector(3.0f), SColor::Teal, 0.0f, false, ThicknessMinimum, false);

		const SVector scale = { UMath::FAbs(cosTime), 0.0f,  UMath::FAbs(sinTime) };
		const SColor rectColor = SColor::Black;
		const SVector rectPos = SVector(4.5f, 0.0f, 4.5f);
		AddRectangle(rectPos, SVector(), scale, rectColor);
		AddRectangle(rectPos, SVector(0.0f, 0.0f, 90.0f), scale, rectColor);
		AddRectangle(rectPos, SVector(90.0f, 0.0f, 0.0f), scale, rectColor);

		//auto LineFollowingAxis = [&](F32 rotation, F32 scale, const SVector& pos, const SColor& axisColor, const SColor& lineColor)
		//{
		//	SVector targetRot = SVector(rotation * cosTime, rotation * sinTime, rotation * cosTime);
		//	SVector targetScl = SVector(scale * cosTime, scale * cosTime, scale * cosTime) + SVector(0.5f);
		//	AddAxis(pos, targetRot, targetScl, axisColor, 1.0f, false, ThicknessMaximum, true);
		//	AddLine(SVector(), pos, lineColor, 1.0f, false, ThicknessMaximum * 0.7f, true);
		//};
		//
		//LineFollowingAxis(90.0f, 2.0f, SVector(3.0f * cosTime, 3.0f * sinTime, 3.0f * cosTime), SColor::Yellow, SColor::Orange);
		//LineFollowingAxis(90.0f, 0.5f, SVector(0.0f, 1.5f * cosTime, 1.5f * sinTime), SColor::Red, SColor::Red);
		//LineFollowingAxis(90.0f, 0.5f, SVector(1.5f * cosTime, 0.0f, 1.5f * sinTime), SColor::Green, SColor::Green);
		//LineFollowingAxis(90.0f, 0.5f, SVector(1.5f * sinTime, 1.5f * cosTime, 0.0f), SColor::Blue, SColor::Blue);
	}

	void UDebugShapeSystem::AddMaxShapes()
	{
		const SVector posLowerBound(-100.0f);
		const SVector posUpperBound(100.0f);
		const SVector rotLowerBound(0.0f);
		const SVector rotUpperBound(180.0f);
		//const SVector sclLowerBound(0.5f);
		//const SVector sclUpperBound(2.0f);
		for (U16 i = 0; i < MaxShapes; i++)
		{
			//UDebugShapeSystem::AddCircle(
			//	SVector::Random(posLowerBound, posUpperBound), 
			//	SVector::Random(rotLowerBound, rotUpperBound), 
			//	0.5f,
			//	32,
			//	SColor::Black, 1.0f, false, ThicknessMaximum, i % 2 == 0);

			UDebugShapeSystem::AddGrid(
				SVector::Random(posLowerBound, posUpperBound), 
				SVector::Random(rotLowerBound, rotUpperBound), 
				SColor::Black, 1.0f, false, 1.0f, i % 2 == 0);
		}

	}
#endif
}
