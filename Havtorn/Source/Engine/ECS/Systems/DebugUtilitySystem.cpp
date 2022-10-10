// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DebugUtilitySystem.h"

#include "Scene/Scene.h"
#include "Debug/DebugUtilityShape.h"
#include "ECS/ECSInclude.h"

namespace Havtorn
{
	CDebugUtilitySystem::CDebugUtilitySystem(CScene* scene)
		: ISystem()
	{
		// FrameRate drops by 200 if we simply add these:
		// Copies being made somewhere?
#ifdef USE_DEBUG_SHAPE
		U64 currentNrOfEntities = scene->GetEntities().size();
		for (U16 i = 0; i < Debug::GDebugUtilityShape::MaxDebugShapes; i++)
		{
			Ref<SEntity> entity = scene->CreateEntity("DebugShape");
			//Ref<SEntity> entity = scene->CreateEntity("hie_DebugShape");
			scene->AddTransformComponentToEntity(entity);
			scene->AddDebugShapeComponentToEntity(entity);
		}

		Debug::GDebugUtilityShape::Init(scene, currentNrOfEntities);

		size_t allocated = 
			  (sizeof(SEntity) * Debug::GDebugUtilityShape::MaxDebugShapes) 
			+ (sizeof(SDebugShapeComponent) * Debug::GDebugUtilityShape::MaxDebugShapes) 
			+ (sizeof(STransformComponent) * Debug::GDebugUtilityShape::MaxDebugShapes);
		HV_LOG_INFO(" DebugUtilitySystem: DebugUtilityShape [MaxShapes: %d] [Allocated: %d bytes]", Debug::GDebugUtilityShape::MaxDebugShapes, allocated);
#else
		scene;
#endif
	}

	CDebugUtilitySystem::~CDebugUtilitySystem()
	{}

	void CDebugUtilitySystem::Update(CScene* /*scene*/)
	{
		Debug::GDebugUtilityShape::Update();
	}


}
