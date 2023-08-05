// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphSystem.h"
#include "ECS/ComponentS/SpriteAnimatorGraphComponent.h"

namespace Havtorn
{
	CSpriteAnimatorGraphSystem::CSpriteAnimatorGraphSystem()
	{
	}

	CSpriteAnimatorGraphSystem::~CSpriteAnimatorGraphSystem()
	{
	}

	void CSpriteAnimatorGraphSystem::Update(CScene* scene)
	{
		std::vector<SSpriteAnimatorGraphComponent>& cmp = scene->GetSpriteAnimatorGraphComponents();
		for (U64 i = 0; i < cmp.size(); i++)
		{

		}
	}
}