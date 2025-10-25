// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GameScript.h"
#include <Scene/World.h>
#include "GhostyNode.h"
#include "Ghosty/SGhostyNodeEditorContext.h"

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
		NodeFactory->RegisterNodeType<HexRune::SGhostyNode, HexRune::SGhostyNodeEditorContext>(this, typeId++);
		NodeFactory->RegisterNodeType<HexRune::SGhostyPositionNode, HexRune::SGhostyNodePositionEditorContex>(this, typeId++);
		
	}
}
