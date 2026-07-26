// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "../NodeView.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SEntityLoopNodeView : public SNodeView
		{
			SEntityLoopNodeView();
		};

		struct SComponentLoopNodeView : public SNodeView
		{
			SComponentLoopNodeView();
		};

		struct SOnBeginOverlapNodeView : public SNodeView
		{
			SOnBeginOverlapNodeView();
		};

		struct SOnEndOverlapNodeView : public SNodeView
		{
			SOnEndOverlapNodeView();
		};

		struct SPrintEntityNameNodeView : public SNodeView
		{
			SPrintEntityNameNodeView();
		};

		struct SSetStaticMeshNodeView : public SNodeView
		{
			SSetStaticMeshNodeView();
		};

		struct STogglePointLightNodeView : public SNodeView
		{
			STogglePointLightNodeView();
		};
	}
}
