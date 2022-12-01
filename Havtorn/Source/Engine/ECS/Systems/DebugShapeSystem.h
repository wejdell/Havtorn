// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

#include "Graphics/GraphicsEnums.h"
#include "Graphics/GraphicsStructs.h"
#include "Core/Color.h"

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
	struct SMatrix;
	struct SEntity;
	struct STransformComponent;
	struct SDebugShapeComponent;
	class CRenderManager;

	// Requirement: Add to Scene systems after RenderSystem.
	class UDebugShapeSystem final : public ISystem
	{
	public: // ISystem inherited.
		UDebugShapeSystem(CScene* scene, CRenderManager* renderManager);
		~UDebugShapeSystem() override;

		void Update(CScene* scene) override;

	public: // Static Add Shape functions.
		/*
		*	Adding a shape, steps:
		*	+	Create SPrimitive in GeometryPrimitives.h for the shape
		*	+	Add entry to EVertexBufferPrimitives and EDefaultIndexBuffers in GraphicsEnum.h
		*	+	In RenderManager.cpp: InitVertexBuffers() and InitIndexBuffers() call AddVertexBuffer(..)/AddIndexBuffer(..) for shape
		*	+	In DebugShapeSystem.cpp: add EVertexBufferPrimitives entry and the corresponding SPrimitive to UDebugShapeSystem::Shapes
		*	+	Create a static function for the shape, use TryAddShape(..) to set default parameters
		*	+	Optional: add a call to the shape to TestAllShapes() and call it somewhere to see that everything is working as expected.
		*/
		static constexpr U16 MaxShapes = 2048;
		static constexpr F32 ThicknessMinimum = 0.005f;
		static constexpr F32 ThicknessMaximum = 0.05f;

		static HAVTORN_API void AddLine(const SVector& start, const SVector& end, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static HAVTORN_API void AddArrow(const SVector& start, const SVector& end, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// Object aligned bounding box with Width/Height/Depth = 1. Pivot is in center.
		static HAVTORN_API void AddCube(const SVector& center, const SVector& eulerRotation, const SVector& scale, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static HAVTORN_API void AddCamera(const SVector& origin, const SVector& eulerRotation, const F32 fov = 70.0f, const F32 farZ = 1.0f, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// Adds a circle across the XZ-plane with 8/16/32 segments.
		static HAVTORN_API void AddCircle(const SVector& origin, const SVector& eulerRotation, const F32 radius = 0.5f, const UINT8 segments = 16, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// 10x10 grid across the XZ-plane (default).
		static HAVTORN_API void AddGrid(const SVector& origin, const SVector& eulerRotation = SVector(), const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static HAVTORN_API void AddAxis(const SVector& origin, const SVector& eulerRotation, const SVector& scale, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// Adds a point at 1/10th of world scale.
		static HAVTORN_API void AddPoint(const SVector& origin, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMaximum, const bool ignoreDepth = true);
		// Adds a rectangle across the XZ-plane. Default (Scale = 1,1,1) is a square.
		static HAVTORN_API void AddRectangle(const SVector& center, const SVector& eulerRotation, const SVector& scale, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);

	private:	
		static bool InstanceExists();
		static bool TryAddShape(const EVertexBufferPrimitives vertexBuffer, const EDefaultIndexBuffers indexBuffer, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth, Ref<STransformComponent>& outTransform);
		
		void SendRenderCommands(
			const std::vector<Ref<SEntity>>& entities,
			const std::vector<Ref<SDebugShapeComponent>>& debugShapes,
			const std::vector<Ref<STransformComponent>>& transformComponents
		);
		void CheckActiveIndices(const std::vector<Ref<SDebugShapeComponent>>& debugShapes);
		void ResetAvailableIndices();

		void TransformToFaceAndReach(const SVector& start, const SVector& end, SMatrix& transform);
		bool TryGetAvailableIndex(U64& outIndex);

#if _DEBUG
	public:
		// TODO.AG: Should only be usable in debug.
		static void TestAllShapes();
		// To stress test. Should use shape with most vertices & indices.
		static void AddMaxShapes();
#endif
		
	private:
		static HAVTORN_API UDebugShapeSystem* Instance;
		const static std::map<EVertexBufferPrimitives, const SPrimitive&> Shapes;

		U64 EntityStartIndex = 0;
		CScene* Scene = nullptr;
		CRenderManager* RenderManager = nullptr;
		std::vector<U64> ActiveIndices;
		std::queue<U64> AvailableIndices;
	};
}
