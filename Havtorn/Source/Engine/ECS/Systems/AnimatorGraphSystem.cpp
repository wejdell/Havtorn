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

		for (SSkeletalAnimationComponent* component : scene->GetComponents<SSkeletalAnimationComponent>())
		{
			if (!component->IsValid())
				continue;
			
			SSkeletalMeshComponent* mesh = scene->GetComponent<SSkeletalMeshComponent>(component->Owner);
			if (!mesh->IsValid())
				continue;

			if (component->IsPlaying)
				component->CurrentAnimationTime = fmodf(component->CurrentAnimationTime += deltaTime, component->DurationInTicks / STATIC_F32(component->TickRate));

			// Ticks == Frames
			F32 ticksPerSecond = STATIC_F32(component->TickRate);
			ticksPerSecond = (ticksPerSecond != 0) ? ticksPerSecond : 24.0f;
			F32 timeInTicks = component->CurrentAnimationTime * ticksPerSecond;
			F32 duration = STATIC_F32(component->DurationInTicks);
			F32 animationTime = fmodf(timeInTicks, duration);

			std::vector<SMatrix> localPose = EvaluateLocalPose(component, animationTime);

			// https://www.youtube.com/watch?v=cieheqt7eqc
			//
			// void applyPoseToJoints(localPose, joint, parentTransform)
			// {
			//		SMatrix currentLocalTransform = localPose[joint];
			//		SMatrix currentTransform = parentTransform * currentLocalTransform;
			//		for (childJoint : joint.Children)
			//		{
			//			applyPoseToJoints(localPose, childJoint, currentTransform);
			//		}
			// 
			//		currentTransform = currentTransform * joint.InverseBindTransform;
			//		localPose[joint] = currentTransform;
			// }
			//
			// applyPoseToJoints(localPose, rootJoint, Identity);


			for (U64 i = 0; i < localPose.size(); i++)
			{
				//const SSkeletalMeshBone& bindPoseBone = mesh->BindPose[i];
				//SMatrix newBonePose = bindPoseBone.Transform;
				//newBonePose *= bindPoseBone.ParentIndex > -1 ? mesh->BindPose[bindPoseBone.ParentIndex].Transform : SMatrix::Identity;
				//localPose[i] *= newBonePose;

				const SSkeletalMeshBone& bindPoseBone = mesh->BindPose[i];
				SMatrix currentLocalTransform = localPose[i];
				SMatrix parentTransform = bindPoseBone.ParentIndex > -1 ? mesh->BindPose[bindPoseBone.ParentIndex].Transform : SMatrix::Identity;
				localPose[i] = parentTransform * currentLocalTransform;
			}

			for (U64 i = 0; i < localPose.size(); i++)
				localPose[i] *= mesh->BindPose[i].Transform.Inverse();
			
	
			// The inverse bind pose matrices get your vertices into bone space so that their parent joint is the origin.
			component->Bones = localPose;
			//component->AnimationData.X = RenderManager->WriteToAnimationDataTexture(component->AssetName);
		}
	}

	void CAnimatorGraphSystem::BindEvaluateFunction(std::function<I16(CScene*, const SEntity&)>& function, const std::string& classAndFunctionName)
	{
		U64 id = std::hash<std::string>{}(classAndFunctionName);

		if (EvaluateFunctionMap.contains(id))
			EvaluateFunctionMap.erase(id);

		EvaluateFunctionMap.emplace(id, function);
	}

	U64 FindRotation(F32 animationTime, const SBoneAnimationTrack& track)
	{
		assert(track.RotationKeys.size() > 0);

		for (U64 i = 0; i < track.RotationKeys.size() - 1; i++)
		{
			if (animationTime < (F32)track.RotationKeys[i + 1].Time)
			{
				return i;
			}
		}
		// This is an 'ugly-fix'
		// In short: bypasses the error by returning the last working key
		return track.RotationKeys.size() - 2;

		//assert(0);
		//return 0xFFFFFFFF;
	}

	SQuaternion CalcInterpolatedRotation(F32 animationTime, const SBoneAnimationTrack& track)
	{
		if (track.RotationKeys.empty())
			return SQuaternion::Identity;

		// we need at least two values to interpolate...
		if (track.RotationKeys.size() == 1)
		{
			return track.RotationKeys[0].Value;
		}

		U64 RotationIndex = FindRotation(animationTime, track);
		U64 NextRotationIndex = (RotationIndex + 1);
		assert(NextRotationIndex < track.RotationKeys.size());
		F32 DeltaTime = static_cast<F32>(track.RotationKeys[NextRotationIndex].Time - track.RotationKeys[RotationIndex].Time);
		F32 Factor = (animationTime - (F32)track.RotationKeys[RotationIndex].Time) / DeltaTime;
		// This if just stops the assert below it from triggering. SP6 animations had some anims with issues and this was faster than having SG debug their animations.
		if (!(Factor >= 0.0f && Factor <= 1.0f))
		{
			Factor = 0.0f;
		}
		// ! If, that stops the assert below it 
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const SQuaternion& StartRotationQ = track.RotationKeys[RotationIndex].Value;
		const SQuaternion& EndRotationQ = track.RotationKeys[NextRotationIndex].Value;
		return SQuaternion::Slerp(StartRotationQ, EndRotationQ, Factor).GetNormalized();
	}

	U64 FindScaling(F32 animationTime, const SBoneAnimationTrack& track)
	{
		assert(track.ScaleKeys.size() > 0);

		// 2021 02 02 Testing/ Figuring out animation speed
		//for (U64 i = 0; i < track.ScaleKeys.size() - 1; i++)
		//{
		//	std::cout << (F32)track.ScaleKeys[i + 1].Time << std::endl;
		//}

		for (U64 i = 0; i < track.ScaleKeys.size() - 1; i++)
		{
			if (animationTime < (F32)track.ScaleKeys[i + 1].Time)
			{
				return i;
			}
		}

		// This is an 'ugly-fix'
		// In short: bypasses the error by returning the last working key
		return track.ScaleKeys.size() - 2;

		//assert(0);
		//return 0xFFFFFFFF;
	}

	SVector CalcInterpolatedScaling(F32 animationTime, const SBoneAnimationTrack& track)
	{
		if (track.ScaleKeys.empty())
			return SVector(1.0f);

		// we need at least two values to interpolate...
		if (track.ScaleKeys.size() == 1)
		{
			return track.ScaleKeys[0].Value;
		}

		U64 ScalingIndex = FindScaling(animationTime, track);
		U64 NextScalingIndex = (ScalingIndex + 1);
		assert(NextScalingIndex < track.ScaleKeys.size());
		F32 DeltaTime = static_cast<F32>(track.ScaleKeys[NextScalingIndex].Time - track.ScaleKeys[ScalingIndex].Time);
		F32 Factor = (animationTime - (F32)track.ScaleKeys[ScalingIndex].Time) / DeltaTime;
		// This if just stops the assert below it from triggering. SP6 animations had some anims with issues and this was faster than having SG debug their animations.
		if (!(Factor >= 0.0f && Factor <= 1.0f))
		{
			Factor = 0.0f;
		}
		// ! If, that stops the assert below it 
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const SVector& StartScaling = track.ScaleKeys[ScalingIndex].Value;
		const SVector& EndScaling = track.ScaleKeys[NextScalingIndex].Value;
		return StartScaling * (1 - Factor) + EndScaling * Factor;
	}

	U64 FindPosition(F32 animationTime, const SBoneAnimationTrack& track)
	{
		assert(track.TranslationKeys.size() > 0);

		for (U64 i = 0; i < track.TranslationKeys.size() - 1; i++)
		{
			if (animationTime < (F32)track.TranslationKeys[i + 1].Time)
			{
				return i;
			}
		}

		// This is an 'ugly-fix'
		// In short: bypasses the error by returning the last working key
		return track.TranslationKeys.size() - 2;

		//assert(0);
		//return 0xFFFFFFFF;
	}

	SVector CalcInterpolatedPosition(F32 animationTime, const SBoneAnimationTrack& track)
	{
		if (track.TranslationKeys.empty())
			return SVector::Zero;

		// we need at least two values to interpolate...
		if (track.TranslationKeys.size() == 1)
		{
			return track.TranslationKeys[0].Value;
		}

		U64 PositionIndex = FindPosition(animationTime, track);
		U64 NextPositionIndex = (PositionIndex + 1);
		assert(NextPositionIndex < track.TranslationKeys.size());
		F32 DeltaTime = static_cast<F32>(track.TranslationKeys[NextPositionIndex].Time - track.TranslationKeys[PositionIndex].Time);
		F32 Factor = (animationTime - (F32)track.TranslationKeys[PositionIndex].Time) / DeltaTime;
		// This if just stops the assert below it from triggering. SP6 animations had some anims with issues and this was faster than having SG debug their animations.
		if (!(Factor >= 0.0f && Factor <= 1.0f))
		{
			Factor = 0.0f;
		}
		// ! If, that stops the assert below it 
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const SVector& StartPosition = track.TranslationKeys[PositionIndex].Value;
		const SVector& EndPosition = track.TranslationKeys[NextPositionIndex].Value;
		return StartPosition * (1 - Factor) + EndPosition * Factor;
	}

	std::vector<SMatrix> CAnimatorGraphSystem::EvaluateLocalPose(const SSkeletalAnimationComponent* animationComponent, const F32 animationTime)
	{
		std::vector<SMatrix> localPose;
		for (const SBoneAnimationTrack& track : animationComponent->CurrentAnimation)
		{
			//HV_LOG_WARN("bone :%s", track.BoneName.c_str());
			SVector scaling = CalcInterpolatedScaling(animationTime, track);
			SQuaternion rotation = CalcInterpolatedRotation(animationTime, track);
			SVector translation = CalcInterpolatedPosition(animationTime, track);

			//SVector scaling = SVector(1.0f);
			//SQuaternion rotation = CalcInterpolatedRotation(animationTime, track);
			//SQuaternion rotation = SQuaternion::Identity;
			//SVector translation = SVector::Zero;

			SMatrix currentLocalPose = SMatrix::Identity;
			SMatrix::Recompose(translation, rotation, scaling, currentLocalPose);
			localPose.emplace_back(currentLocalPose);
		}

		return localPose;
	}
}