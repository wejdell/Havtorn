// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugUtilitySystem.h"

#include "Scene/Scene.h"
#include "Debug/DebugDrawer.h"

namespace Havtorn
{
	CDebugUtilitySystem::CDebugUtilitySystem()
		: ISystem()
	{}

	CDebugUtilitySystem::~CDebugUtilitySystem()
	{}

	void CDebugUtilitySystem::Update(CScene* scene)
	{
#ifdef _DEBUG
		CheckDebugShapesForRendering(scene);
#endif
	}

	void CDebugUtilitySystem::CheckDebugShapesForRendering(CScene* scene)
	{
#ifdef _DEBUG
		scene->AddDebugShapes(Debug::GDebugDrawer::AddToRendering());
#endif
	}

}
