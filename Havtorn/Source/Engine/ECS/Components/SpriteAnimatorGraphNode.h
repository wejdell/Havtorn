// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/HavtornString.h"

#include <functional>

namespace Havtorn
{
	class CScene;
	struct SSpriteAnimatorGraphComponent;
	struct SSpriteAnimationClip
	{
		SSpriteAnimationClip() = default;
		SSpriteAnimationClip(const std::vector<SVector4>& uvRects, const std::vector<float>& durations, const bool isLooping)
			: UVRects(uvRects)
			, Durations(durations)
			, IsLooping(isLooping)
		{
		}

		std::vector<SVector4> UVRects;
		std::vector<float> Durations;
		bool IsLooping = false;

		// AS: UVRect count vs Durations Count, if UVRect.Size == 5 and Durations.Size == 2, we'll still loop 5 times when processing an AnimationClip,
		// then we could for example, for frame 3 (> Duration.Size()) choose Duration[min(Duration.Size() - 1, i)]
		const U32 KeyFrameCount() const { return static_cast<U32>(UVRects.size()); }
	};

	// AS: This is basically a Switch and an index into animationClips vector
	struct SSpriteAnimatorGraphNode 
	{
		CHavtornStaticString<32> Name;
		std::vector<SSpriteAnimatorGraphNode> Nodes;
		std::function<I16(CScene*, U64)> Evaluate;
		I16 AnimationClipKey = -1;

		HAVTORN_API SSpriteAnimatorGraphNode& AddSwitchNode(const CHavtornStaticString<32>& name, std::function<I16(CScene*, U64)> evaluator);
		HAVTORN_API void AddClipNode(SSpriteAnimatorGraphComponent* component, const CHavtornStaticString<32>& name, const SSpriteAnimationClip& clipData);
	};
}