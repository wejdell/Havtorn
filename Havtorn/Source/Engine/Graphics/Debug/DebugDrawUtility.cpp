// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugDrawUtility.h"

#include "Graphics/GeometryPrimitives.h"

#include "Graphics/RenderManager.h"
#include "Graphics/RenderCommand.h"

namespace Havtorn
{
	GDebugDraw* GDebugDraw::Instance = nullptr;

	const std::map<EVertexBufferPrimitives, const SPrimitive&> GDebugDraw::PrimitivesMap =
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

	void GDebugDraw::Update()
	{
		if (LiveData.empty())
			return;

		{
			// Push prepass render commands
			SRenderCommand command(ERenderCommandType::PreDebugShape);
			RenderManager->PushRenderCommand(command, 0);

			command.Type = ERenderCommandType::PostToneMappingIgnoreDepth;
			RenderManager->PushRenderCommand(command, 0);

			command.Type = ERenderCommandType::PostToneMappingUseDepth;
			RenderManager->PushRenderCommand(command, 0);
		}

		const F32 dt = GTime::Dt();
		for (SDebugDrawData& data : LiveData)
		{
			ERenderCommandType commandType = (data.IgnoreDepth) ? ERenderCommandType::DebugShapeIgnoreDepth : ERenderCommandType::DebugShapeUseDepth;

			SRenderCommand command;
			command.Type = commandType;
			command.Matrices.push_back(data.TransformMatrix);
			command.Vectors.push_back(data.Color.AsVector4());
			command.F32s.push_back(data.Thickness);
			command.U16s.push_back(data.IndexCount);
			command.U8s.push_back(data.VertexBufferIndex);
			command.U8s.push_back(data.IndexBufferIndex);
			RenderManager->PushRenderCommand(command, 0);

			data.LifeTime -= dt;
		}

		std::erase_if(LiveData, [](const SDebugDrawData& data) { return data.LifeTime <= 0.0f; });
	}

	bool GDebugDraw::Init(CRenderManager* renderManager)
	{
#ifdef USE_DEBUG_SHAPE
		RenderManager = renderManager;

		if (RenderManager == nullptr)
			return false;

		if (Instance != nullptr)
			HV_LOG_WARN("GDebugDraw already exists, replacing existing Instance!");

		Instance = this;

		HV_LOG_INFO("GDebugDraw: Instance initialized.");
		return true;
#else
		renderManager;
		return false;
#endif
	}

#pragma region AddShape

	void GDebugDraw::AddLine(const SVector& start, const SVector& end, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		if (start.IsEqual(end))
			return;

		std::vector<SDebugDrawData> newData = { SDebugDrawData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line) };

		TransformToFaceAndReach(start, end, newData[0].TransformMatrix);

		TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, newData);
	}

	void GDebugDraw::AddArrow(const SVector& start, const SVector& end, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		std::vector<SDebugDrawData> newData = { 
			SDebugDrawData(EVertexBufferPrimitives::Pyramid, EDefaultIndexBuffers::Pyramid),
			SDebugDrawData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line)
		};
		SMatrix& lineTransform = newData[1].TransformMatrix;
		TransformToFaceAndReach(start, end, lineTransform);

		constexpr F32 scale = 0.1f;
		const SVector pyramidPos = end - lineTransform.GetForward().GetNormalized() * 0.1f;
		// Default pyramid's height is along the Y axis, rotation offset of 90 degrees around X places it along the Z axis.
		SMatrix::Recompose(pyramidPos, lineTransform.GetEuler() + SVector(90.0f, 0.0f, 0.0f), scale, newData[0].TransformMatrix);
		if (TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, newData))
		{
		}	
	}

	void GDebugDraw::AddCube(const SVector& center, const SVector& eulerRotation, const SVector& scale, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		std::vector<SDebugDrawData> newData = { SDebugDrawData(EVertexBufferPrimitives::BoundingBox, EDefaultIndexBuffers::BoundingBox)};
		SMatrix::Recompose(center, eulerRotation, scale, newData[0].TransformMatrix);
		if (TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, newData))
		{
		}
	}

	void GDebugDraw::AddCamera(const SVector& origin, const SVector& eulerRotation, const F32 fov, const F32 aspectRatio, const F32 farZ, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		std::vector<SDebugDrawData> newData = { SDebugDrawData(EVertexBufferPrimitives::Camera, EDefaultIndexBuffers::Camera)};
		// TODO.AG: Rework this. Does not seem to properly represent fov & farZ. Might have to use aspectratio?
		F32 y = 2.0f * farZ * std::tanf(UMath::DegToRad(fov) * 0.5f);
		F32 x = 2.0f * farZ * std::tanf(UMath::DegToRad(fov) * 0.5f) * aspectRatio;
		SVector vScale(x, y, farZ);
		SMatrix::Recompose(origin, eulerRotation, vScale, newData[0].TransformMatrix);
		TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, newData);
		AddCube(origin + newData[0].TransformMatrix.GetBackward() * 0.02f, eulerRotation, SVector(0.15f, 0.15f, 0.25f), color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
	}

	void GDebugDraw::AddCircle(const SVector& origin, const SVector& eulerRotation, const F32 radius, const U8 segments, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		// AG. The Default Circle is across the XZ plane
		EVertexBufferPrimitives vertexBufferPrimitive = EVertexBufferPrimitives::Circle16;
		EDefaultIndexBuffers indexBuffer = EDefaultIndexBuffers::Circle16;

		// if requested segments are closer to 32
		if (segments > 24)
		{
			vertexBufferPrimitive = EVertexBufferPrimitives::Circle32;
			indexBuffer = EDefaultIndexBuffers::Circle32;
		}
		// if requested segments are closer to 8
		else if (segments < 12)
		{
			vertexBufferPrimitive = EVertexBufferPrimitives::Circle8;
			indexBuffer = EDefaultIndexBuffers::Circle8;
		}

		std::vector<SDebugDrawData> newData = { SDebugDrawData(vertexBufferPrimitive, indexBuffer)};
		SVector scale(radius / GeometryPrimitives::CircleRadius);
		SMatrix::Recompose(origin, eulerRotation, scale, newData[0].TransformMatrix);
		if (TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, newData))
		{
		}
	}

	void GDebugDraw::AddGrid(const SVector& origin, const SVector& eulerRotation, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		std::vector<SDebugDrawData> data = { SDebugDrawData(EVertexBufferPrimitives::Grid, EDefaultIndexBuffers::Grid)};
		SMatrix::Recompose(origin, eulerRotation, SVector(1.0f), data[0].TransformMatrix);
		if (TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, data))
		{
		}
	}

	void GDebugDraw::AddAxis(const SVector& origin, const SVector& eulerRotation, const SVector& scale, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		std::vector<SDebugDrawData> newData = { SDebugDrawData(EVertexBufferPrimitives::Axis, EDefaultIndexBuffers::Axis)};
		SMatrix::Recompose(origin, eulerRotation, scale, newData[0].TransformMatrix);
		if (TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, newData))
		{
		}
	}

	void GDebugDraw::AddPoint(const SVector& origin, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		std::vector<SDebugDrawData> newData = { SDebugDrawData(EVertexBufferPrimitives::Octahedron, EDefaultIndexBuffers::Octahedron)};
		SMatrix::Recompose(origin, SVector(), SVector(0.1f), newData[0].TransformMatrix);
		if (TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, newData))
		{
		}
	}

	void GDebugDraw::AddRectangle(const SVector& center, const SVector& eulerRotation, const SVector& scale, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		std::vector<SDebugDrawData> newData = { SDebugDrawData(EVertexBufferPrimitives::Square, EDefaultIndexBuffers::Square)};
		SMatrix::Recompose(center, eulerRotation, scale, newData[0].TransformMatrix);
		if (TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, newData))
		{
		}
	}

	void GDebugDraw::AddSphere(const SVector& center, const SVector& eulerRotation, const SVector& scale, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		std::vector<SDebugDrawData> newData = { SDebugDrawData(EVertexBufferPrimitives::UVSphere, EDefaultIndexBuffers::UVSphere)};
		SMatrix::Recompose(center, eulerRotation, scale, newData[0].TransformMatrix);
		if (TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, newData))
		{
		}
	}

	void GDebugDraw::AddConeRadius(const SVector& apexPosition, const SVector& direction, const F32 height, const F32 radius, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		std::vector<SDebugDrawData> newData = { 
			SDebugDrawData(EVertexBufferPrimitives::Circle16, EDefaultIndexBuffers::Circle16),
			SDebugDrawData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line),
			SDebugDrawData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line),
			SDebugDrawData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line),
			SDebugDrawData(EVertexBufferPrimitives::Line, EDefaultIndexBuffers::Line),
		};
			const SVector base = apexPosition + direction.GetNormalized() * height;

			const SVector scale(radius / GeometryPrimitives::CircleRadius);
			const SVector up = direction.IsEqual(SVector::Up) ? SVector::Forward : SVector::Up;
			const SMatrix lookAt = SMatrix::Face(apexPosition, direction, up);
			
			// Default circle lies on the XZ plane, adding a rotation offset of 90degrees around X rotates it to the XY plane.
			SMatrix::Recompose(base, lookAt.GetEuler() + SVector(90.0f, 0.0f, 0.0f), scale, newData[0].TransformMatrix);

			const SVector lookAtRight = lookAt.GetRight();
			const SVector lookAtUp = lookAt.GetUp();
			TransformToFaceAndReach(apexPosition, base + lookAtRight * radius, newData[1].TransformMatrix);
			TransformToFaceAndReach(apexPosition, base + lookAtRight * -radius, newData[2].TransformMatrix);
			TransformToFaceAndReach(apexPosition, base + lookAtUp * radius, newData[3].TransformMatrix);
			TransformToFaceAndReach(apexPosition, base + lookAtUp * -radius, newData[4].TransformMatrix);
		if (TryAddShapes(color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth, newData))
		{
		}
	}

	void GDebugDraw::AddConeAngle(const SVector& apexPosition, const SVector& direction, const F32 height, const F32 angleDegrees, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth)
	{
		const F32 radius = height * UMath::Sin(UMath::DegToRad(angleDegrees));
		AddConeRadius(apexPosition, direction, height, radius, color, lifeTimeSeconds, useLifeTime, thickness, ignoreDepth);
	}

#pragma endregion !AddShape

	bool GDebugDraw::InstanceExists()
	{
		if (Instance == nullptr)
		{
#if DEBUG_DRAWER_LOG_ERROR
			HV_LOG_ERROR("GDebugDraw has not been created!");
#endif
			return false;
		}
		return true;
	}

	bool GDebugDraw::TryAddShapes(const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth, std::vector<SDebugDrawData>& outData)
	{
		if (!InstanceExists())
			return false;

		if (outData.empty())
		{
			HV_LOG_WARN("GDebugDraw: Tried to add shapes without any data to initialize from. Please supply index and vertex buffers enums to outData in TryAddShapes.");
			return false;
		}

		for (SDebugDrawData& data : outData)
		{
			data.Color = color;
			data.LifeTime = useLifeTime ? lifeTimeSeconds : -1.0f;
			data.Thickness = UMath::Clamp(thickness, ThicknessMinimum, ThicknessMaximum);
			data.IndexCount = STATIC_U16(PrimitivesMap.at(data.VertexBuffer).Indices.size());
			data.VertexBufferIndex = STATIC_U8(data.VertexBuffer);
			data.IndexBufferIndex = STATIC_U8(data.IndexBuffer);
			data.IgnoreDepth = ignoreDepth;
		}

		auto iterator = std::ranges::find_if(Instance->LiveData, [outData](const SDebugDrawData& data) { return data.LifeTime > outData[0].LifeTime; });
		Instance->LiveData.insert(iterator, outData.begin(), outData.end());

		return true;
	}

	void GDebugDraw::TransformToFaceAndReach(const SVector& start, const SVector& end, SMatrix& transform)
	{
		// AG: Using SVector::Forward as up works for some reason?
		const SVector up = SVector::Forward; // SVector::Up: breaks up == direction.
		const SVector direction = (end - start).GetNormalized();
		const SVector scale = SVector(1.0f, 1.0f, start.Distance(end));
		transform = SMatrix::Face(start, direction, up);
		SMatrix::Recompose(start, transform.GetEuler(), scale, transform);
	}

#if _DEBUG
	void GDebugDraw::TestAllShapes()
	{
		// TODO.AG: add Camera

		AddGrid(SVector(), SVector(), SColor::Grey, 1.0f, false, ThicknessMinimum * 4.f, false);
		AddArrow(SVector(), SVector::Right * 5.0f, SColor::Red, 1.0f, false, ThicknessMinimum * 2.f, true);
		AddArrow(SVector(), SVector::Up * 5.0f, SColor::Green, 1.0f, false, ThicknessMinimum * 2.f, true);
		AddArrow(SVector(), SVector::Forward * 5.0f, SColor::Blue, 1.0f, false, ThicknessMinimum * 2.f, true);

		const F32 time = GTime::Time();
		const F32 cosTime = UMath::Cos(time);
		const F32 sinTime = UMath::Sin(time);

		static F32 previousTime = 0.0f;
		constexpr F32 lifeTime = 2.0f;
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
		Circle(static_cast<UINT8>(UMath::Random(25, 33)), 2.5f, { 90.0f, 0.0f, 180.0f * sinTime }, SColor::Blue);
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

	void GDebugDraw::AddMaxShapes()
	{
		const SVector posLowerBound(-100.0f);
		const SVector posUpperBound(100.0f);
		const SVector rotLowerBound(0.0f);
		const SVector rotUpperBound(180.0f);
		//const SVector sclLowerBound(0.5f);
		//const SVector sclUpperBound(2.0f);
		for (U16 i = 0; i < MaxShapes; i++)
		{
			//GDebugDraw::AddCircle(
			//	SVector::Random(posLowerBound, posUpperBound), 
			//	SVector::Random(rotLowerBound, rotUpperBound), 
			//	0.5f,
			//	32,
			//	SColor::Black, 1.0f, false, ThicknessMaximum, i % 2 == 0);

			GDebugDraw::AddGrid(
				SVector::Random(posLowerBound, posUpperBound),
				SVector::Random(rotLowerBound, rotUpperBound),
				SColor::Black, 1.0f, false, 1.0f, i % 2 == 0);
		}

	}
#endif
}
