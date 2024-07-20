// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include "Entity.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	struct SComponentView
	{
		virtual void View(const SEntity& entityOwner, CScene* scene) {};
	};
}

/*
add as components, this is what inspector uses, puti n scene, similar pattern but with std::map<guid, SComponentView*>
*/