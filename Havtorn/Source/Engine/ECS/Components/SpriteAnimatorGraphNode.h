// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include <functional>
#include "Core/HavtornString.h"

namespace Havtorn
{
	class CScene;
	struct SSpriteAnimatorGraphComponent;
	struct SSpriteAnimationClip
	{
		std::vector<SVector4> UVRects;
		std::vector<float> Durations;
		bool IsLooping = false;

		const U32 KeyFrameCount() const { return static_cast<U32>(UVRects.size()); }
	};

	struct SSpriteAnimatorGraphNode //This is basically a Switch and an index into animationClips vector
	{
		CHavtornStaticString<32> Name;
		std::vector<SSpriteAnimatorGraphNode> Nodes;
		std::function<I16(CScene*, U64)> Evaluate;
		I16 AnimationClipKey = -1;

		HAVTORN_API SSpriteAnimatorGraphNode& AddSwitchNode(const CHavtornStaticString<32>& name, std::function<I16(CScene*, U64)> evaluator);
		HAVTORN_API void AddClipNode(SSpriteAnimatorGraphComponent* component, const CHavtornStaticString<32>& name, const SSpriteAnimationClip& clipData);
	};
}