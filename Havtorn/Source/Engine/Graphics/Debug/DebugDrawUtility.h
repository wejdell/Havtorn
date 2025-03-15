// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include "Graphics/GraphicsEnums.h"
#include "Graphics/GraphicsStructs.h"
#include <Color.h>

#include <queue>
#include <map>

#ifdef _DEBUG

#ifndef USE_DEBUG_SHAPE
#define USE_DEBUG_SHAPE
#endif

#define DEBUG_DRAWER_LOG 1
#if DEBUG_DRAWER_LOG
#define DEBUG_DRAWER_LOG_SHAPESRENDER 0
#define DEBUG_DRAWER_LOG_ADDSHAPE 0
#define DEBUG_DRAWER_LOG_ERROR 1
#endif

#endif

namespace Havtorn
{
	class GEngine;
	class CRenderManager;

	class GDebugDraw final
	{
		friend GEngine;

	public: // Static Add Shape functions.
		/*
		*	Adding a shape, steps:
		*	+	Create SPrimitive in GeometryPrimitives.h for the shape
		*	+	Add entry to EVertexBufferPrimitives and EDefaultIndexBuffers in GraphicsEnum.h
		*	+	In RenderManager.cpp: InitVertexBuffers() and InitIndexBuffers() call AddVertexBuffer(..)/AddIndexBuffer(..) for shape
		*	+	In DebugShapeSystem.cpp: add EVertexBufferPrimitives entry and the corresponding SPrimitive to GDebugDraw::PrimitivesMap
		*	+	Create a static function for the shape, use TryAddShapes(..) to set default parameters
		*	+	Optional: add a call to the shape to TestAllShapes() and call it somewhere to see that everything is working as expected.
		*/
		static constexpr U16 MaxShapes = 0;
		static constexpr F32 ThicknessMinimum = 0.005f;
		static constexpr F32 ThicknessMaximum = 0.05f;

		static ENGINE_API void AddLine(const SVector& start, const SVector& end, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static ENGINE_API void AddArrow(const SVector& start, const SVector& end, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// Object aligned bounding box with Width/Height/Depth = 1. Pivot is in center.
		static ENGINE_API void AddCube(const SVector& center, const SVector& eulerRotation, const SVector& scale, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static ENGINE_API void AddCamera(const SVector& origin, const SVector& eulerRotation, const F32 fov = 70.0f, const F32 farZ = 1.0f, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// Adds a circle across the XZ-plane with 8/16/32 segments.
		static ENGINE_API void AddCircle(const SVector& origin, const SVector& eulerRotation, const F32 radius = 0.5f, const U8 segments = 16, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// 10x10 grid across the XZ-plane (default).
		static ENGINE_API void AddGrid(const SVector& origin, const SVector& eulerRotation = SVector(), const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static ENGINE_API void AddAxis(const SVector& origin, const SVector& eulerRotation, const SVector& scale, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// Adds a point at 1/10th of world scale.
		static ENGINE_API void AddPoint(const SVector& origin, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMaximum, const bool ignoreDepth = true);
		// Adds a rectangle across the XZ-plane. Default (Scale = 1,1,1) is a square.
		static ENGINE_API void AddRectangle(const SVector& center, const SVector& eulerRotation, const SVector& scale, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static ENGINE_API void AddSphere(const SVector& center, const SVector& eulerRotation, const SVector& scale, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// Adds a cone with its base facing towards direction.
		static ENGINE_API void AddConeRadius(const SVector& apexPosition, const SVector& direction, const F32 height, const F32 radius, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// Adds a cone with its base facing towards direction. angleRadians: angle between slanted height and height at the apex of the cone.
		static ENGINE_API void AddConeAngle(const SVector& apexPosition, const SVector& direction, const F32 height, const F32 angleDegrees, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		//static void AddCapsule(const SVector& center, const SVector& eulerRotation, const F32 height, const F32 radius, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);

	private:
		bool Init(CRenderManager* renderManager);
		void Update();

		static bool InstanceExists();

		/*
			TODO.AG: Adding several shapes at once: composite shapes.
			Differing parameters: vertexBuffer, indexBuffer, transform
		*/
		struct SDebugDrawData
		{
			EVertexBufferPrimitives VertexBuffer;
			EDefaultIndexBuffers IndexBuffer;

			SMatrix TransformMatrix;
			SColor Color;
			F32 LifeTime = 0.0f;
			F32 Thickness = 1.0f;
			U16 IndexCount = 0;
			U8 VertexBufferIndex = 0;
			U8 IndexBufferIndex = 0;
			bool IgnoreDepth = true;

			SDebugDrawData() = delete;
			SDebugDrawData(EVertexBufferPrimitives vertexBuffer, EDefaultIndexBuffers indexBuffer)
				: VertexBuffer(vertexBuffer)
				, IndexBuffer(indexBuffer)
			{}

			std::partial_ordering operator<=>(const SDebugDrawData& rhs) const { return LifeTime <=> rhs.LifeTime; }
		};

		static bool TryAddShapes(const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth, std::vector<SDebugDrawData>& outData);
		static void TransformToFaceAndReach(const SVector& start, const SVector& end, SMatrix& transform);

#if _DEBUG
	public:
		// TODO.AG: Should only be usable in debug.
		static void ENGINE_API TestAllShapes();
		// To stress test. Should use shape with most vertices & indices.
		static void AddMaxShapes();
#endif

	private:
		static GDebugDraw* Instance;
		CRenderManager* RenderManager = nullptr;

		const static std::map<EVertexBufferPrimitives, const SPrimitive&> PrimitivesMap;
		std::vector<SDebugDrawData> LiveData;
	};
}