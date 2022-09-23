// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugDrawer.h"

#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	namespace Debug
	{
		GDebugDrawer* GDebugDrawer::Instance = nullptr;
		U16 GDebugDrawer::DEBUG_SHAPES_COUNT = 0;

		void GDebugDrawer::AddLine(const SVector& start, const SVector& end, const SColor& color, const bool singleFrame, const F32 lifeTimeSeconds)
		{
#ifdef _DEBUG
			if (!InstanceExists())
				return;

			SDebugShape* shape = nullptr;
			if (!Instance->GetAvailableShape(shape))
				return;

			shape->Color = color;
			shape->LifeTime = LifeTimeForShape(singleFrame, lifeTimeSeconds);

			shape->Vertices[0].x = start.X;
			shape->Vertices[0].y = start.Y;
			shape->Vertices[0].z = start.Z;
			shape->Vertices[0].w = 1.0f;

			shape->Vertices[1].x = end.X;
			shape->Vertices[1].y = end.Y;
			shape->Vertices[1].z = end.Z;
			shape->Vertices[1].w = 1.0f;

			shape->UsedVertices = 2;
#else
			start; end; color; singleFrame; lifeTimeSeconds;
#endif
		}

		std::vector<const SDebugShape*> GDebugDrawer::GetShapesToRender()
		{
#ifdef _DEBUG
			std::vector<const SDebugShape*> shapesToRender;
			if (!InstanceExists())
				return shapesToRender;

			Instance->SortShapes();
			auto&& shapes = Instance->Shapes;
			shapesToRender.resize(DEBUG_SHAPES_COUNT, nullptr);

			F32 time = GTimer::Time();
			U16 stillLifeTimeCount = 0;
			for (U16 i = 0; i < DEBUG_SHAPES_COUNT ; i++)
			{
				if (shapes[i].LifeTime > time)
					stillLifeTimeCount = i;

				shapesToRender[i] = &shapes[i];
			}
			DEBUG_SHAPES_COUNT = stillLifeTimeCount;
			return shapesToRender;
#endif
	}

		void GDebugDrawer::SortShapes()
		{
			std::sort(Shapes.begin(), Shapes.end(), std::greater());
		}

		bool GDebugDrawer::GetAvailableShape(SDebugShape* outShape)
		{
			if (DEBUG_SHAPES_COUNT < MAX_DEBUG_SHAPES)
			{
				outShape = &Shapes[DEBUG_SHAPES_COUNT];
				DEBUG_SHAPES_COUNT++;
				return true;
			}
			HV_LOG_ERROR("GDebugDrawer: Reached MAX_DEBUG_SHAPES, cannot draw more shapes!");
			return false;
		}

		GDebugDrawer::GDebugDrawer()
		{
			if (Instance == nullptr)
			{
				Instance = this;
				HV_LOG_INFO("GDebugDrawer created!");
			}
		}

		GDebugDrawer::~GDebugDrawer()
		{
			if (Instance == this)
			{
				Instance = nullptr;
				HV_LOG_INFO("GDebugDrawer destroyed!");
			}
		}

		bool GDebugDrawer::InstanceExists()
		{
			if (Instance == nullptr)
			{
				HV_LOG_WARN("Trying to debug draw however, GDebugDrawer has not been created!");
				return false;
			}
			return true;
		}

		F32 GDebugDrawer::LifeTimeForShape(const bool singleFrame, const F32 requestedLifeTime)
		{
			if (singleFrame)
				return -1.0f;
			else
				return GTimer::Time() + requestedLifeTime;
		}
	}
}

