// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "SequencerKeyframe.h"

namespace Havtorn
{
	struct ENGINE_API SSequencerTransformKeyframe : public SSequencerKeyframe
	{
		SMatrix KeyframedMatrix;
		SMatrix IntermediateMatrix;

		virtual void Blend(SSequencerKeyframe* nextKeyframe, F32 blendParam) override;
		virtual void SetEntityDataOnKeyframe(CScene* scene, U64 sceneIndex) override;
		virtual void SetKeyframeDataOnEntity(CScene* scene, U64 sceneIndex) override;

		virtual U32 GetSize() const override;
		virtual void Serialize(char* toData, U64& pointerPosition) const override;
		virtual void Deserialize(const char* fromData, U64& pointerPosition) override;
	};
}