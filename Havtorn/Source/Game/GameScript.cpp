// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GameScript.h"
#include <Scene/World.h>

namespace Havtorn
{
	SGameScript::SGameScript()
		: SScript()
	{
	}

	SGameScript::~SGameScript()
	{
	}

	void SGameScript::Init()
	{
		HexRune::SScript::Init();
	}
}
