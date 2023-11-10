
// Copyright 2023 Team Havtorn. All Rights Reserved.

// Copyright 2022 Team Havtorn. All Rights Reserved.

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
}