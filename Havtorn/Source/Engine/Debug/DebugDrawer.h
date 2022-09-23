// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Debug/DebugShape.h"

// TODO.AG Use a Define to use instead of just _DEBUG.
// AG: Game Engine Architecture 2nd Ed. Jason Gregory suggests a debug drawing utility tool should tie its drawn shapes to a scene. Unreal uses a similar structure with UWorld being passed as a parameter.
namespace Havtorn
{
	namespace Debug
	{
		class GDebugDrawer
		{
			friend class GEngine;
			friend class CRenderSystem;

		public:
			static constexpr U16 MAX_DEBUG_SHAPES = 10000;
			static U16 DEBUG_SHAPES_COUNT;
			/*
				Color should be something other than an SVector4. Temp implementation.
			*/
			static HAVTORN_API void AddLine(const SVector& start, const SVector& end, const SColor& color/*temp*/, const bool singleFrame = true, const F32 lifeTimeSeconds = 0.0f);


		private:
			static bool InstanceExists();
			static F32 LifeTimeForShape(const bool singleFrame, const F32 requestedLifeTime);
			static std::vector<const SDebugShape*> GetShapesToRender();

			void SortShapes();
			
			bool GetAvailableShape(SDebugShape* outShape);

		private:
			static HAVTORN_API GDebugDrawer* Instance;

			GDebugDrawer();
			~GDebugDrawer();

			// Could be an array with a fixed size
			//std::vector<SDebugShape> Shapes;//Used to be std::list, but can't figure out a reason to why it would be preferable over std::vector. - AG
			std::array<SDebugShape, MAX_DEBUG_SHAPES> Shapes;//Used to be std::list, but can't figure out a reason to why it would be preferable over std::vector. - AG
		};
	}
}