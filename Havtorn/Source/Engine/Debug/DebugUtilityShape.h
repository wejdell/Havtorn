// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/ColorList.h"

#include <queue>

#ifdef _DEBUG

#ifndef USE_DEBUG_SHAPE
#define USE_DEBUG_SHAPE
#endif
#define DEBUG_DRAWER_LOG
#ifdef DEBUG_DRAWER_LOG
#define DEBUG_DRAWER_LOG_SHAPESRENDER 1
#define DEBUG_DRAWER_LOG_ADDSHAPE 1
#define DEBUG_DRAWER_LOG_ERROR 1
#endif

#endif


// TODO.AG Use a Define to use instead of just _DEBUG.
// AG: Game Engine Architecture 2nd Ed. Jason Gregory suggests a debug drawing utility tool should tie its drawn shapes to a scene. Unreal uses a similar structure with UWorld being passed as a parameter.
// Stop using auto :) most of the time.
namespace Havtorn
{
	class CScene;
	struct SDebugShapeComponent;

	namespace Debug
	{
		class GDebugUtilityShape
		{
			friend class GEngine;
			friend class CDebugUtilitySystem;

		public:

			static HAVTORN_API void AddLine(const SVector& start, const SVector& end, const SVector4& color, const bool singleFrame = true, const F32 lifeTimeSeconds = 0.0f);// DepthPrio

		public:
			static constexpr U16 MaxDebugShapes = 500;
			static const std::vector<U64>& GetActiveShapeIndices();
		
		private:
			static bool InstanceExists();

			static F32 LifeTimeForShape(const bool singleFrame, const F32 requestedLifeTime);

			static void Init(CScene* activeScene, U64 entityStartIndex);
			static void Update();

			bool HasConnectionToScene();
			bool TryGetAvailableIndex(U64& outIndex);// Might want to rework how this works.
			void ResetAvailableIndices();
			void PrintDebugAddedShape(const SDebugShapeComponent& shape, const bool singleFrame, const char* function);

		private:
			static HAVTORN_API GDebugUtilityShape* Instance;
			static std::vector<U64> NoShapeIndices;

			GDebugUtilityShape();
			~GDebugUtilityShape();

			U64 EntityStartIndex = 0;
			CScene* ActiveScene = nullptr;
			std::vector<U64> ActiveIndices;
			std::queue<U64> AvailableIndices;
			// IF a shape can be attached to another Entity (transform childed), then the parent could run out of scope. Which would cause a crash.
			// Might have to be part of every scene. And the GDebugUtilityShape simply manages the one existing in the scene. Like sending indices to render.

		};
	}
}