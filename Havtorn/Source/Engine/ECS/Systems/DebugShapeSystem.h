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
#define DEBUG_DRAWER_LOG_ERROR 0
#endif

#endif

namespace Havtorn
{
	struct SDebugShapeComponent;

	namespace Debug
	{
		class UDebugShapeSystem final : public ISystem
		{
		public: // ISystem inherited.
			UDebugShapeSystem(CScene* scene);
			~UDebugShapeSystem() override;

			void Update(CScene* scene) override;

		public:// Static Other
			static constexpr U16 MaxShapes = 5;
			static const std::vector<U64>& GetActiveShapeIndices();

		public:// Static Add Shape functions.
			static HAVTORN_API void AddLine(const SVector& start, const SVector& end, const SVector4& color, const bool singleFrame = true, const F32 lifeTimeSeconds = 0.0f);// DepthPrio

		private:
			static bool InstanceExists();
			static F32 LifeTimeForShape(const bool singleFrame, const F32 requestedLifeTime);

			void Init(CScene* activeScene, U64 entityStartIndex);

			// HasConnectionToScene:
			/*
			* Kept in - case it is needed in the future.Currently however, 
			* there should be no circumstance where Scene == nullptr after construction.
			*/ 
			bool HasConnectionToScene();

			bool TryGetAvailableIndex(U64& outIndex);// Might want to rework how this works.
			void ResetAvailableIndices();

			void PrintDebugAddedShape(const SDebugShapeComponent& shape, const bool singleFrame, const char* callerFunction);

		private:
			static HAVTORN_API UDebugShapeSystem* Instance;
			static const std::vector<U64> NoShapeIndices;

			U64 EntityStartIndex = 0;
			CScene* Scene = nullptr;
			std::vector<U64> ActiveIndices;
			std::queue<U64> AvailableIndices;
		};
	}
}