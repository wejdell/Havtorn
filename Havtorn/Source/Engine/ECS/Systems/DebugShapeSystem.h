// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

#include "Graphics/GraphicsEnums.h"
#include "Core/Color.h"

#include <queue>

#ifdef _DEBUG

#ifndef USE_DEBUG_SHAPE
#define USE_DEBUG_SHAPE
#endif
#define DEBUG_DRAWER_LOG
#ifdef DEBUG_DRAWER_LOG
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
		static constexpr U16 MaxShapes = 10000;
		static constexpr F32 ThicknessMinimum = 0.005f;
		static constexpr F32 ThicknessMaximum = 0.05f;

		// TODO.AG: write some documentation on steps for adding a shape.
		static HAVTORN_API void AddLine(const SVector& start, const SVector& end, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static HAVTORN_API void AddArrow(const SVector& start, const SVector& end, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		// Cube with Width/Height/Depth = 1. Pivot is in center.
		static HAVTORN_API void AddCube(const SVector& center, const SVector& scale, const SVector& eulerRotation, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static HAVTORN_API void AddCamera(const SVector& origin, const SVector& eulerRotation, const F32 fov = 70.0f, const F32 farZ = 1.0f, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
			
		static HAVTORN_API void AddCircleXY(const SVector& origin, const SVector& eulerRotation, const F32 radius = 0.5f, const UINT8 segments = 16, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static HAVTORN_API void AddCircleXZ(const SVector& origin, const SVector& eulerRotation, const F32 radius = 0.5f, const UINT8 segments = 16, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
		static HAVTORN_API void AddCircleYZ(const SVector& origin, const SVector& eulerRotation, const F32 radius = 0.5f, const UINT8 segments = 16, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);

		// Adds a 10x10 grid across the XZ-plane.
		static HAVTORN_API void AddGrid(const SVector& origin, const SColor& color = SColor::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
	
	private:
		static HAVTORN_API void AddDefaultCircle(const SVector& origin, const SVector& eulerRotation, const F32 radius, const UINT8 segments, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth);
			
		static bool InstanceExists();
		static F32 LifeTimeForShape(const bool useLifeTime, const F32 requestedLifeTime);
		static F32 ClampThickness(const F32 thickness);
			
		static bool TryAddShape(const EVertexBufferPrimitives vertexBuffer, const EDefaultIndexBuffers indexBuffer, const SColor& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth, Ref<STransformComponent>& outTransform);
		
		void TransformToFaceAndReach(SMatrix& transform, const SVector& start, const SVector& end);

		void SendRenderCommands(
			const std::vector<Ref<SEntity>>& entities,
			const std::vector<Ref<SDebugShapeComponent>>& debugShapes,
			const std::vector<Ref<STransformComponent>>& transformComponents
		);
		void CheckActiveIndices(const std::vector<Ref<SDebugShapeComponent>>& debugShapes);

		bool TryGetAvailableIndex(U64& outIndex);
		void ResetAvailableIndices();

	private:
		static HAVTORN_API UDebugShapeSystem* Instance;

		U64 EntityStartIndex = 0;
		CScene* Scene = nullptr;
		CRenderManager* RenderManager = nullptr;
		std::vector<U64> ActiveIndices;
		std::queue<U64> AvailableIndices;
	};
}
