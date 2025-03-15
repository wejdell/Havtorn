// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class CScene;

	struct ENGINE_API SSequencerKeyframe : ISerializable
	{
		U32 FrameNumber = 0;
		bool ShouldBlendLeft = true;
		bool ShouldBlendRight = true;
		bool ShouldRecord = false;

		// NR: Make sure to support nextKeyframe being nullptr, this means that the function should 
		// only set its data to the current value at the current keyframe
		virtual void Blend(SSequencerKeyframe* /*nextKeyframe*/, F32 /*blendParam*/) {};
		virtual void SetEntityDataOnKeyframe(CScene* /*scene*/, U64 /*sceneIndex*/) {};
		virtual void SetKeyframeDataOnEntity(CScene* /*scene*/, U64 /*sceneIndex*/) {};

		// Inherited via ISerializable
		virtual U32 GetSize() const override;
		virtual void Serialize(char* toData, U64& pointerPosition) const override;
		virtual void Deserialize(const char* fromData, U64& pointerPosition) override;
	};
}