// Copyright 2025 Team Havtorn. All Rights Reserved.
#include "hvpch.h"
#include "GameScript.h"
#include "NodeInclude.h"
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

	void SGameScript::InitializeGame(U32 typeId)
	{
		//typeId = 10000;
		NodeFactory->RegisterNodeType<HexRune::SGhostyNode, HexRune::SGhostyNodeEditorContext>(this, typeId++);
		NodeFactory->RegisterNodeType<HexRune::SGhostyPositionNode, HexRune::SGhostyNodePositionEditorContex>(this, typeId++);
		NodeFactory->RegisterNodeType<HexRune::SInterpolatePosition, HexRune::SInterpolatePositionNodeEditorContext>(this, typeId++);
		NodeFactory->RegisterNodeType<HexRune::STimerNode, HexRune::STimerNodeEditorContext>(this, typeId++);
	}
}
