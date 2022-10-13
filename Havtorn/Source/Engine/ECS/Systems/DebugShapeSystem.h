// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

#include "Core/ColorList.h"

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
	struct SEntity;
	struct STransformComponent;
	struct SDebugShapeComponent;
	class CRenderManager;

	namespace Debug
	{
		// Requirement: Add to Scene systems after RenderSystem.
		class UDebugShapeSystem final : public ISystem
		{
		public: // ISystem inherited.
			UDebugShapeSystem(CScene* scene, CRenderManager* renderManager);
			~UDebugShapeSystem() override;

			void Update(CScene* scene) override;

		public: // Static Add Shape functions.
			static constexpr U16 MaxShapes = 50;
			static constexpr F32 ThicknessMinimum = 0.005f;
			static constexpr F32 ThicknessMaximum = 0.05f;

			static HAVTORN_API void AddLine(const SVector& start, const SVector& end, const SVector4& color = Color::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
			static HAVTORN_API void AddArrow(const SVector& start, const SVector& end, const SVector4& color = Color::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);
			static HAVTORN_API void AddCube(const SVector& center, const F32 scale, const SVector& eulerRotation, const SVector4& color = Color::White, const F32 lifeTimeSeconds = -1.0f, const bool useLifeTime = true, const F32 thickness = ThicknessMinimum, const bool ignoreDepth = true);

		private:
			static bool InstanceExists();
			static F32 LifeTimeForShape(const bool useLifeTime, const F32 requestedLifeTime);
			static F32 ClampThickness(const F32 thickness);
			
			static void SetSharedDataForShape(Ref<SDebugShapeComponent>& inoutShape, const SVector4& color, const F32 lifeTimeSeconds, const bool useLifeTime, const F32 thickness, const bool ignoreDepth);
			static void TransformToFaceAndReach(Ref<STransformComponent>& inoutTransform, const SVector& start, const SVector& end);

			void SendRenderCommands(
				const std::vector<Ref<SEntity>>& entities,
				const std::vector<Ref<SDebugShapeComponent>>& debugShapes,
				const std::vector<Ref<STransformComponent>>& transformComponents
			);
			void CheckActiveIndices(const std::vector<Ref<SDebugShapeComponent>>& debugShapes);

			/*
			* AG: Kept in case it is needed in the future. Currently however, 
			* there should be no circumstance where Scene == nullptr after construction.
			*/ 
			bool HasConnectionToScene();

			bool TryGetAvailableIndex(U64& outIndex);
			void ResetAvailableIndices();

			// AG: To be removed/ looked over. Use case is debatable.
			void PrintDebugAddedShape(const SDebugShapeComponent& shape, const bool useLifeTime, const char* callerFunction);

		private:
			static HAVTORN_API UDebugShapeSystem* Instance;

			U64 EntityStartIndex = 0;
			CScene* Scene = nullptr;
			CRenderManager* RenderManager = nullptr;
			std::vector<U64> ActiveIndices;
			std::queue<U64> AvailableIndices;
		};
	}
}
