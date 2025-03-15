// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include <HavtornString.h>

#include <functional>

namespace Havtorn
{
	class CScene;
	struct SSpriteAnimatorGraphComponent;
	struct SSpriteAnimationClip : public ISerializable
	{
		SSpriteAnimationClip() = default;
		SSpriteAnimationClip(const std::vector<SVector4>& uvRects, const std::vector<float>& durations, const bool isLooping)
			: UVRects(uvRects)
			, Durations(durations)
			, IsLooping(isLooping)
		{
		}

		ENGINE_API [[nodiscard]] U32 GetSize() const override;
		ENGINE_API void Serialize(char* toData, U64& pointerPosition) const override;
		ENGINE_API void Deserialize(const char* fromData, U64& pointerPosition) override;

		std::vector<SVector4> UVRects;
		std::vector<float> Durations;
		bool IsLooping = false;

		// AS: UVRect count vs Durations Count, if UVRect.Size == 5 and Durations.Size == 2, we'll still loop 5 times when processing an AnimationClip,
		// then we could for example, for frame 3 (> Duration.Size()) choose Duration[min(Duration.Size() - 1, i)]
		const U32 KeyFrameCount() const { return STATIC_U32(UVRects.size()); }
	};

	// AS: This is basically a Switch and an index into animationClips vector
	struct SSpriteAnimatorGraphNode : public ISerializable
	{
		CHavtornStaticString<32> Name;
		std::vector<SSpriteAnimatorGraphNode> Nodes;
		U64 EvaluateFunctionMapKey = 0;
		I16 AnimationClipKey = -1;

		ENGINE_API SSpriteAnimatorGraphNode& AddSwitchNode(const CHavtornStaticString<32>& name, const std::string& evaluatorClassAndFunctionName);
		ENGINE_API void AddClipNode(SSpriteAnimatorGraphComponent* component, const CHavtornStaticString<32>& name, const SSpriteAnimationClip& clipData);

		ENGINE_API [[nodiscard]] U32 GetSize() const override;
		ENGINE_API void Serialize(char* toData, U64& pointerPosition) const override;
		ENGINE_API void Deserialize(const char* fromData, U64& pointerPosition) override;
	};
}