// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ECSNodeViews.h"

#include <GUI.h>

namespace Havtorn
{
	namespace HexRune
	{
		SEntityLoopNodeView::SEntityLoopNodeView()
		{
			Name = "For Each Loop (Entity)";
			Category = "General";
		}

		SComponentLoopNodeView::SComponentLoopNodeView()
		{
			Name = "For Each Loop (Component)";
			Category = "General";
		}

		SPrintEntityNameNodeView::SPrintEntityNameNodeView()
		{
			Name = "Print Entity Name";
			Category = "General";
			Color = SColor::Teal;
		}

		SSetStaticMeshNodeView::SSetStaticMeshNodeView()
		{
			Name = "Set Static Mesh";
			Category = "ECS";
			Color = SColor::Orange;
		}

		STogglePointLightNodeView::STogglePointLightNodeView()
		{
			Name = "Toggle Point Light";
			Category = "ECS";
			Color = SColor::Orange;
		}

		SOnBeginOverlapNodeView::SOnBeginOverlapNodeView()
		{
			Name = "On Begin Overlap";
			Category = "ECS";
			Color = SColor::Red;
		}

		SOnEndOverlapNodeView::SOnEndOverlapNodeView()
		{
			Name = "On End Overlap";
			Category = "ECS";
			Color = SColor::Red;
		}
	}
}
