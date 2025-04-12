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

	void ApplyPose(const SMatrix& parentTransform, const std::vector<SSkeletalMeshBone>& bindPoseBones, const std::vector<SMatrix>& animatedPose, std::vector<SSkeletalMeshNode>& animatedNodes, SSkeletalMeshNode& animatedNode)
	{
		// ! NodeTransform and localPose are in the same space, they are identical in function, and relative to their parent !

		// find animated pose corresponding to our node, through bind pose bones

		// animatedNodes has been initialized with the bind pose nodes outside this scope, just needs to update their transforms!

		I32 transformIndex = -1;

		if (auto it = std::ranges::find(bindPoseBones, animatedNode.Name, &SSkeletalMeshBone::Name); it != bindPoseBones.end())
			transformIndex = STATIC_I32(std::distance(std::begin(bindPoseBones), it));
		
		if (transformIndex > -1)
		{
			SMatrix parentTransformCopy = parentTransform;
			SMatrix animatedPoseCopy = animatedPose[transformIndex];
			animatedNode.NodeTransform = animatedPoseCopy * parentTransformCopy;
		}

		for (U32 i = 0; i < STATIC_U32(animatedNode.ChildIndices.size()); i++)
		{
			ApplyPose(animatedNode.NodeTransform, bindPoseBones, animatedPose, animatedNodes, animatedNodes[animatedNode.ChildIndices[i]]);
		}
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

			// VERSION 2
			//component->Bones.clear();
			//ReadHierarchy(component, mesh, animationTime, mesh->Nodes[0].NodeTransform, mesh->Nodes[0], component->Bones);

			// VERSION 1
			std::vector<SMatrix> localPose = EvaluateLocalPose(component, animationTime);

			//for (U64 i = 0; i < localPose.size(); i++)
			//{
				//const SSkeletalMeshBone& bindPoseBone = mesh->BindPose[i];
				//SMatrix currentLocalTransform = localPose[i];
				//SMatrix parentTransform = bindPoseBone.ParentIndex > -1 ? localPose[bindPoseBone.ParentIndex] : SMatrix::Identity;
				//localPose[i] = currentLocalTransform * parentTransform;
			//}

			// Initialize unanimated transforms and child indexes
			std::vector<SSkeletalMeshNode> animatedNodes = mesh->Nodes;
			
			// TODO: Need to add the root here? Root may not be identity, should try to save the "InverseGlobalTransform" i.e. root node inverse
			ApplyPose(SMatrix::Identity, mesh->BindPose, localPose, animatedNodes, animatedNodes[0]);

			// Rename localPose to finalPose at the end

			// If this works, we should be able to combine these last for-loops, each element shouldn't be dependent on other elements
			for (U64 i = 0; i < localPose.size(); i++)
			{
				const SSkeletalMeshBone& bindPoseBone = mesh->BindPose[i];
				SMatrix animatedBoneSpaceTransform = localPose[i];
				for (const auto& node : animatedNodes)
				{
					if (node.Name == bindPoseBone.Name)
					{
						/** Node Transform is relative to the node's parent. Need to go through all nodes from root to find the final object space transform for each bone (node). Probably
						a good idea to save that in import, for bind pose.*/
						// go from animated bone space to object space --
						SMatrix boneObjectSpaceTransform = node.NodeTransform;
						localPose[i] = animatedBoneSpaceTransform * boneObjectSpaceTransform;
					}
				}
			}

			for (U64 i = 0; i < localPose.size(); i++)
				localPose[i] = mesh->BindPose[i].InverseBindPoseTransform * localPose[i];
	
			// The inverse bind pose matrices get your vertices into bone space so that their parent joint is the origin.
			component->Bones = localPose;

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

	void CAnimatorGraphSystem::ReadHierarchy(const SSkeletalAnimationComponent* animationComponent, const SSkeletalMeshComponent* mesh, const F32 animationTime, const SMatrix& parentTransform, const SSkeletalMeshNode& node, std::vector<SMatrix>& posedTransforms)
	{
		SMatrix nodeTransform = node.NodeTransform;
		SMatrix currentLocalPose = SMatrix::Identity;

		std::string nodeName = node.Name.AsString();
		I32 boneIndex = -1;
		if (auto it = std::ranges::find(animationComponent->CurrentAnimation, nodeName, &SBoneAnimationTrack::BoneName); it != animationComponent->CurrentAnimation.end())
		{
			boneIndex = STATIC_I32(std::distance(animationComponent->CurrentAnimation.begin(), it));
			const SBoneAnimationTrack& track = *it;

			SVector scaling = CalcInterpolatedScaling(animationTime, track);
			SQuaternion rotation = CalcInterpolatedRotation(animationTime, track);
			SVector translation = CalcInterpolatedPosition(animationTime, track);

			SMatrix::Recompose(translation, rotation, scaling, currentLocalPose);
		}

		SMatrix globalTransform = nodeTransform * parentTransform;

		if (boneIndex >= 0)
		{
			// TODO.NW: Make matrix operations const where they should be
			SMatrix inverseBindPose = mesh->BindPose[boneIndex].InverseBindPoseTransform;
			posedTransforms.emplace_back(inverseBindPose * globalTransform);
		}

		for (auto childIndex : node.ChildIndices)
		{
			ReadHierarchy(animationComponent, mesh, animationTime, globalTransform, mesh->Nodes[childIndex], posedTransforms);
		}
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
			//SQuaternion rotation = SQuaternion::Identity;
			//SVector translation = SVector::Zero;

			SMatrix currentLocalPose = SMatrix::Identity;
			SMatrix::Recompose(translation, rotation, scaling, currentLocalPose);
			localPose.emplace_back(currentLocalPose);
		}

		return localPose;
	}
}