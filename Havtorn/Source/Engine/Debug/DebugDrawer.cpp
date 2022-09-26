// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugDrawer.h"

#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	namespace Debug
	{
		GDebugDrawer* GDebugDrawer::Instance = nullptr;
		U16 GDebugDrawer::DebugShapesCount = 0;

		void GDebugDrawer::AddLine(const SVector& start, const SVector& end, const SColor& color, const bool singleFrame, const F32 lifeTimeSeconds)
		{
#ifdef _DEBUG
			if (!InstanceExists())
				return;

			SDebugShape* shape = nullptr;
			if (!Instance->TryGetAvailableShape(shape))
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
			
			PrintDebugAddedShape(*shape, singleFrame, __FUNCTION__);
#else
			start; end; color; singleFrame; lifeTimeSeconds;
#endif
		}

		const SDebugShape GDebugDrawer::GetShapeOn(const U16 index)
		{
			if (!InstanceExists() || index >= DebugShapesCount)
			{
				return SDebugShape();
			}


			return Instance->Shapes[index];
		}

		std::vector<SDebugShape*> GDebugDrawer::GetShapesToRender()
		{
#ifdef _DEBUG
			std::vector<SDebugShape*> shapesToRender;
			if (!InstanceExists())
				return shapesToRender;

			Instance->SortShapes();
			auto&& shapes = Instance->Shapes;
			shapesToRender.resize(DebugShapesCount, nullptr);

			const F32 time = GTimer::Time();
			U16 stillLifeTimeCount = 0;
			for (U16 i = 0; i < DebugShapesCount; i++)
			{
				if (shapes[i].LifeTime > time)
					stillLifeTimeCount = i;

				shapesToRender[i] = &shapes[i];
			}
			DebugShapesCount = stillLifeTimeCount + 1;

#if DEBUG_DRAWER_LOG_SHAPESRENDER
			HV_LOG_INFO("GDebugDrawer::GetShapesToRender -----------------");
			HV_LOG_INFO("Time [%f]\tShapes current frame [%d]\tShapes kept [%d]", time, shapesToRender.size(), DebugShapesCount);
			for (U16 i = 0; i < shapesToRender.size(); i++)
			{
				HV_LOG_INFO("\tShapes To Render: %d\tLifeTime[%f]", i, shapesToRender[i]->LifeTime - time);
			}
			HV_LOG_INFO("-----------------");
#endif
			return shapesToRender;
#endif
	}

		// TODO.AG: Move helpers elsewhere

		std::string ConvertSColorToString(const SColor& color)
		{
			char buffer[128];
			sprintf_s(buffer, "R: %.2f, G: %.2f, B: %.2f, A:%.2f", color.R, color.G, color.B, color.A);
			return buffer;
		}

		std::string ConvertSPositionVertexToString(const SPositionVertex& vertex)
		{
			char buffer[128];
			sprintf_s(buffer, "{X: %.1f, Y: %.1f, Z: %.1f, W:%.1f}", vertex.x, vertex.y, vertex.z, vertex.w);
			return buffer;
		}

		// !TODO.AG

		void GDebugDrawer::PrintDebugAddedShape(const SDebugShape& shape, const bool singleFrame, const char* function)
		{
#if DEBUG_DRAWER_LOG_ADDSHAPE
			HV_LOG_INFO("%s: Added:", function);
			HV_LOG_INFO("\tColor[%s] SingleFrame[%d] LifeTime[%fs]"
				, ConvertSColorToString(shape.Color).c_str()
				, singleFrame
				, shape.LifeTime
			);
			for (U8 i = 0; i < shape.UsedVertices; i++)
			{
				HV_LOG_INFO("\tVertex [%d]: %s]"
					, i
					, ConvertSPositionVertexToString(shape.Vertices[i]).c_str()
				);

			}
#else
			shape; singleFrame; function;
#endif
		}

		void GDebugDrawer::SortShapes()
		{
			std::sort(Shapes.begin(), Shapes.end(), std::greater());
		}

		bool GDebugDrawer::TryGetAvailableShape(SDebugShape*& outShape)
		{
			if (DebugShapesCount < MaxDebugShapes)
			{
				outShape = &Shapes[DebugShapesCount];
				DebugShapesCount++;
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

