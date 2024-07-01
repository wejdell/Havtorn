// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "SpriteAnimatorGraphComponent.h"

namespace Havtorn
{
	SSpriteAnimatorGraphNode& SSpriteAnimatorGraphComponent::SetRoot(const CHavtornStaticString<32>& name, std::function<I16(CScene*, U64)> evaluator)
	{
		Graph = SSpriteAnimatorGraphNode();
		Graph.Evaluate = evaluator;
		Graph.Name = name;
		Graph.AnimationClipKey = -1;
		return Graph;
	}
}