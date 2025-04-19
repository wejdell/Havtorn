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

	static bool once = false;
	static int nodeNameIndex = 0;

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
			component->Bones.clear();
			ReadHierarchy(component, mesh, animationTime, mesh->Nodes[0].NodeTransform, mesh->Nodes[0], component->Bones);
			once = true;

			//// VERSION 1
			//std::vector<SMatrix> localPose = EvaluateLocalPose(component, animationTime);

			////for (U64 i = 0; i < localPose.size(); i++)
			////{
			//	//const SSkeletalMeshBone& bindPoseBone = mesh->BindPose[i];
			//	//SMatrix currentLocalTransform = localPose[i];
			//	//SMatrix parentTransform = bindPoseBone.ParentIndex > -1 ? localPose[bindPoseBone.ParentIndex] : SMatrix::Identity;
			//	//localPose[i] = currentLocalTransform * parentTransform;
			////}

			//// Initialize unanimated transforms and child indexes
			//std::vector<SSkeletalMeshNode> animatedNodes = mesh->Nodes;
			//
			//// TODO: Need to add the root here? Root may not be identity, should try to save the "InverseGlobalTransform" i.e. root node inverse
			//ApplyPose(SMatrix::Identity, mesh->BindPose, localPose, animatedNodes, animatedNodes[0]);

			//// Rename localPose to finalPose at the end

			//// If this works, we should be able to combine these last for-loops, each element shouldn't be dependent on other elements
			//for (U64 i = 0; i < localPose.size(); i++)
			//{
			//	const SSkeletalMeshBone& bindPoseBone = mesh->BindPose[i];
			//	SMatrix animatedBoneSpaceTransform = localPose[i];
			//	for (const auto& node : animatedNodes)
			//	{
			//		if (node.Name == bindPoseBone.Name)
			//		{
			//			/** Node Transform is relative to the node's parent. Need to go through all nodes from root to find the final object space transform for each bone (node). Probably
			//			a good idea to save that in import, for bind pose.*/
			//			// go from animated bone space to object space --
			//			SMatrix boneObjectSpaceTransform = node.NodeTransform;
			//			localPose[i] = animatedBoneSpaceTransform * boneObjectSpaceTransform;
			//		}
			//	}
			//}

			//for (U64 i = 0; i < localPose.size(); i++)
			//	localPose[i] = mesh->BindPose[i].InverseBindPoseTransform * localPose[i];
	
			//// The inverse bind pose matrices get your vertices into bone space so that their parent joint is the origin.
			//component->Bones = localPose;
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

		U64 rotationIndex = FindRotation(animationTime, track);
		U64 nextRotationIndex = (rotationIndex + 1);
		assert(nextRotationIndex < track.RotationKeys.size());
		
		F32 deltaTime = static_cast<F32>(track.RotationKeys[nextRotationIndex].Time - track.RotationKeys[rotationIndex].Time);

		F32 factor = (animationTime - (F32)track.RotationKeys[rotationIndex].Time) / deltaTime;
		factor = UMath::Clamp(factor);

		const SQuaternion& startRotationQ = track.RotationKeys[rotationIndex].Value;
		const SQuaternion& endRotationQ = track.RotationKeys[nextRotationIndex].Value;
		return SQuaternion::Slerp(startRotationQ, endRotationQ, factor).GetNormalized();
	}

	U64 FindScaling(F32 animationTime, const SBoneAnimationTrack& track)
	{
		assert(track.ScaleKeys.size() > 0);

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
	}

	SVector CalcInterpolatedScaling(F32 animationTime, const SBoneAnimationTrack& track)
	{
		if (track.ScaleKeys.empty())
			return SVector(1.0f);

		if (track.ScaleKeys.size() == 1)
		{
			return track.ScaleKeys[0].Value;
		}

		U64 scalingIndex = FindScaling(animationTime, track);
		U64 nextScalingIndex = (scalingIndex + 1);
		assert(nextScalingIndex < track.ScaleKeys.size());
		
		F32 deltaTime = static_cast<F32>(track.ScaleKeys[nextScalingIndex].Time - track.ScaleKeys[scalingIndex].Time);
		
		F32 factor = (animationTime - (F32)track.ScaleKeys[scalingIndex].Time) / deltaTime;
		factor = UMath::Clamp(factor);

		const SVector& startScaling = track.ScaleKeys[scalingIndex].Value;
		const SVector& endScaling = track.ScaleKeys[nextScalingIndex].Value;
		return startScaling * (1 - factor) + endScaling * factor;
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

		U64 positionIndex = FindPosition(animationTime, track);
		U64 nextPositionIndex = (positionIndex + 1);
		assert(nextPositionIndex < track.TranslationKeys.size());
	
		F32 deltaTime = static_cast<F32>(track.TranslationKeys[nextPositionIndex].Time - track.TranslationKeys[positionIndex].Time);
		
		F32 factor = (animationTime - (F32)track.TranslationKeys[positionIndex].Time) / deltaTime;	
		factor = UMath::Clamp(factor);
		
		const SVector& startPosition = track.TranslationKeys[positionIndex].Value;
		const SVector& endPosition = track.TranslationKeys[nextPositionIndex].Value;
		return startPosition * (1 - factor) + endPosition * factor;
	}

	void CAnimatorGraphSystem::ReadHierarchy(const SSkeletalAnimationComponent* animationComponent, const SSkeletalMeshComponent* mesh, const F32 animationTime, const SMatrix& parentTransform, const SSkeletalMeshNode& node, std::vector<SMatrix>& posedTransforms)
	{
		SMatrix nodeTransform = node.NodeTransform;

		// TODO.NW: Streamline this so we can read the local pose of different animations at the same time, then combine them at the end
		std::string nodeName = node.Name.AsString();
		//if (!once)
		//	HV_LOG_TRACE("%i: %s", ++nodeNameIndex, nodeName.c_str());

		if (auto it = std::ranges::find(animationComponent->CurrentAnimation, nodeName, &SBoneAnimationTrack::TrackName); it != animationComponent->CurrentAnimation.end())
		{
			const SBoneAnimationTrack& track = *it;

			SVector scaling = CalcInterpolatedScaling(animationTime, track);
			SQuaternion rotation = CalcInterpolatedRotation(animationTime, track);
			SVector translation = CalcInterpolatedPosition(animationTime, track);

			SMatrix::Recompose(translation, rotation, scaling, nodeTransform);
			//if (!once)
			//{
			//	HV_LOG_TRACE("%i: %s", ++nodeNameIndex, nodeName.c_str());
			//	SVector trans;
			//	SVector rot;
			//	SVector scal;
			//	SMatrix::Decompose(nodeTransform, trans, rot, scal);
			//	HV_LOG_TRACE("Node Transform: t: %s, r: %s", trans.ToString().c_str(), rot.ToString().c_str());
			//}
		}

		SMatrix globalTransform = nodeTransform * parentTransform;
		//if (!once)
		//{
		//	HV_LOG_TRACE("%i: %s", ++nodeNameIndex, nodeName.c_str());
		//	SVector trans;
		//	SVector rot;
		//	SVector scal;
		//	SMatrix::Decompose(globalTransform, trans, rot, scal);
		//	HV_LOG_WARN("Global: t: %s, r: %s, s: %s", trans.ToString().c_str(), rot.ToString().c_str(), scal.ToString().c_str());
		//}

		if (auto it = std::ranges::find(mesh->BindPose, nodeName, &SSkeletalMeshBone::Name); it != mesh->BindPose.end())
		{
			I32 boneIndex = STATIC_I32(std::distance(mesh->BindPose.begin(), it));
			posedTransforms.emplace_back(mesh->BindPose[boneIndex].InverseBindPoseTransform * globalTransform);

			//if (!once)
			//{
			//	HV_LOG_WARN("%i: %s", ++nodeNameIndex, nodeName.c_str());
			//	SVector trans;
			//	SVector rot;
			//	SVector scal;
			//	SMatrix::Decompose(inverseBindPose, trans, rot, scal);
			//	HV_LOG_WARN("Global: t: %s, r: %s, s: %s", trans.ToString().c_str(), rot.ToString().c_str(), scal.ToString().c_str());
			//}
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
			//HV_LOG_WARN("bone :%s", track.TrackName.c_str());
			SVector scaling = CalcInterpolatedScaling(animationTime, track);
			SQuaternion rotation = CalcInterpolatedRotation(animationTime, track);
			SVector translation = CalcInterpolatedPosition(animationTime, track);

			SMatrix currentLocalPose = SMatrix::Identity;
			SMatrix::Recompose(translation, rotation, scaling, currentLocalPose);
			localPose.emplace_back(currentLocalPose);
		}

		return localPose;
	}
}