// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AnimatorGraphSystem.h"
#include "Graphics/RenderManager.h"
#include "ECS/Components/SkeletalAnimationComponent.h"
#include "ECS/Components/SkeletalMeshComponent.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	CAnimatorGraphSystem::CAnimatorGraphSystem(CRenderManager* renderManager)
		: ISystem()
		, RenderManager(renderManager)
	{
	}

	void CAnimatorGraphSystem::Update(CScene* scene)
	{
		const F32 deltaTime = GTime::Dt();

		std::vector<SBoneAnimDataTransform> data;
		for (SSkeletalAnimationComponent* component : scene->GetComponents<SSkeletalAnimationComponent>())
		{
			if (!component->IsValid())
				continue;

			if (data.empty())
				data = PreprocessAnimation(component);

			F32 frameDuration = 1.0f / STATIC_F32(component->TickRate);
			if ((component->CurrentFrameTime += deltaTime) >= frameDuration)
			{
				component->CurrentFrameTime -= frameDuration;
				component->AnimationData.Y = (component->AnimationData.Y + 1) % component->DurationInTicks;
			}
		}
		
		//RenderManager->WriteToAnimationDataTexture(data.data(), sizeof(SBoneAnimDataTransform) * data.size());
	}

	void CAnimatorGraphSystem::BindEvaluateFunction(std::function<I16(CScene*, const SEntity&)>& function, const std::string& classAndFunctionName)
	{
		U64 id = std::hash<std::string>{}(classAndFunctionName);

		if (EvaluateFunctionMap.contains(id))
			EvaluateFunctionMap.erase(id);

		EvaluateFunctionMap.emplace(id, function);
	}

	std::vector<SBoneAnimDataTransform> CAnimatorGraphSystem::PreprocessAnimation(SSkeletalAnimationComponent* component)
	{
		std::vector<SBoneAnimDataTransform> data;
		F32 accumulatedTime = 0.0f;
		for (U32 tick = 0; tick < component->DurationInTicks; tick++)
		{
			accumulatedTime += 1.0f / STATIC_F32(component->TickRate);
			for (const SBoneAnimationTrack& track : component->BoneAnimationTracks)
			{
				SVecBoneAnimationKey translationKey;
				for (const SVecBoneAnimationKey& key : track.TranslationKeys)
					translationKey = (key.Time <= accumulatedTime) ? key : translationKey;

				SQuatBoneAnimationKey rotationKey;
				for (const SQuatBoneAnimationKey& key : track.RotationKeys)
					rotationKey = (key.Time <= accumulatedTime) ? key : rotationKey;

				SVecBoneAnimationKey scaleKey;
				for (const SVecBoneAnimationKey& key : track.ScaleKeys)
					scaleKey = (key.Time <= accumulatedTime) ? key : scaleKey;
				
				data.push_back(EncodeTransform(translationKey, rotationKey, scaleKey));
			}
		}
		return data;
	}

	SBoneAnimDataTransform CAnimatorGraphSystem::EncodeTransform(const SVecBoneAnimationKey& translationKey, const SQuatBoneAnimationKey& rotationKey, const SVecBoneAnimationKey& scaleKey)
	{
		SBoneAnimDataTransform dataFrame;
		SMatrix boneMatrix;
		SMatrix::Recompose(translationKey.Value, rotationKey.Value.ToEuler(), scaleKey.Value, boneMatrix);
		SVector translation = boneMatrix.GetTranslation();
		SMatrix::Transpose(boneMatrix); // NR: Maybe should not do this
		
		dataFrame.Row1TX = boneMatrix.GetRow(0);
		dataFrame.Row1TX.W = translation.X;
		dataFrame.Row2TY = boneMatrix.GetRow(1);
		dataFrame.Row2TY.W = translation.Y;
		dataFrame.Row3TZ = boneMatrix.GetRow(2);
		dataFrame.Row3TZ.W = translation.Z;

		return dataFrame;
	}
}