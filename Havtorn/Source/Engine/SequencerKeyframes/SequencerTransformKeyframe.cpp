// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "SequencerTransformKeyframe.h"
#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	void SSequencerTransformKeyframe::Blend(SSequencerKeyframe* nextKeyframe, F32 blendParam)
	{
		if (SSequencerTransformKeyframe* nextTransformKeyframe = dynamic_cast<SSequencerTransformKeyframe*>(nextKeyframe))
		{
			IntermediateMatrix = SMatrix::Interpolate(KeyframedMatrix, nextTransformKeyframe->KeyframedMatrix, blendParam);
			return;
		}

		IntermediateMatrix = KeyframedMatrix;
	}

	void SSequencerTransformKeyframe::SetEntityDataOnKeyframe(CScene* scene, U64 sceneIndex)
	{
		STransformComponent& transformComponent = scene->GetTransformComponents()[sceneIndex];
		KeyframedMatrix = transformComponent.Transform.GetMatrix();
	}

	void SSequencerTransformKeyframe::SetKeyframeDataOnEntity(CScene* scene, U64 sceneIndex)
	{
		STransformComponent& transformComponent = scene->GetTransformComponents()[sceneIndex];
		transformComponent.Transform.SetMatrix(IntermediateMatrix);
	}

	U32 SSequencerTransformKeyframe::GetSize() const
	{
		U32 size = SSequencerKeyframe::GetSize();
		size += sizeof(SMatrix) * 2;
		return size;
	}

	void SSequencerTransformKeyframe::Serialize(char* toData, U64& pointerPosition) const
	{
		SSequencerKeyframe::Serialize(toData, pointerPosition);
		SerializeSimple(KeyframedMatrix, toData, pointerPosition);
		SerializeSimple(IntermediateMatrix, toData, pointerPosition);
	}

	void SSequencerTransformKeyframe::Deserialize(const char* fromData, U64& pointerPosition)
	{
		SSequencerKeyframe::Deserialize(fromData, pointerPosition);
		DeserializeSimple(KeyframedMatrix, fromData, pointerPosition);
		DeserializeSimple(IntermediateMatrix, fromData, pointerPosition);
	}
}