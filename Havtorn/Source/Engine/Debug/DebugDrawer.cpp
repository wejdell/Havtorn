// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugDrawer.h"

namespace Havtorn
{
	namespace Debug
	{
		GDebugDrawer* GDebugDrawer::Instance = nullptr;

		void GDebugDrawer::DrawLine(const SVector& start, const SVector& end, const SVector4& color)
		{
			start; end; color;
#ifdef _DEBUG
			if (!InstanceExists())
				return;

			// Do stuff
#endif
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
	}
}

