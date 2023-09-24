// Copyright 2023 Team Havtorn. All Rights Reserved.

// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class CScene;

	struct HAVTORN_API SSequencerKeyframe
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
	};
}