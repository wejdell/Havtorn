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

	static bool once = false;
	static int nodeNameIndex = 0;

	ENGINE_API void CAnimatorGraphSystem::Update(CScene* scene)
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
				component->CurrentAnimationTime = fmodf(component->CurrentAnimationTime += deltaTime, component->CurrentAnimation[component->CurrentAnimationIndex].DurationInTicks / STATIC_F32(component->CurrentAnimation[component->CurrentAnimationIndex].TickRate));

			// Ticks == Frames
			F32 ticksPerSecond = STATIC_F32(component->CurrentAnimation[component->CurrentAnimationIndex].TickRate);
			ticksPerSecond = (ticksPerSecond != 0) ? ticksPerSecond : 24.0f;
			F32 timeInTicks = component->CurrentAnimationTime * ticksPerSecond;
			F32 duration = STATIC_F32(component->CurrentAnimation[component->CurrentAnimationIndex].DurationInTicks);
			F32 animationTime = fmodf(timeInTicks, duration);

			// VERSION 2
			component->Bones.clear();

			component->CurrentAnimationIndex = 0;
			std::vector<SSkeletalPosedNode> walkPosedNodes = {};
			ReadAnimationLocalPose(component, mesh, animationTime, mesh->Nodes[0], walkPosedNodes);

			component->CurrentAnimationIndex = 1;
			std::vector<SSkeletalPosedNode> runPosedNodes = {};
			ReadAnimationLocalPose(component, mesh, animationTime, mesh->Nodes[0], runPosedNodes);

			component->CurrentAnimationIndex = 0;

			std::vector<SSkeletalPosedNode> blendPosedNodes = {};
			blendPosedNodes.resize(mesh->Nodes.size());
			for (U32 i = 0; i < blendPosedNodes.size(); i++)
			{
				auto& walk = walkPosedNodes[i];
				auto& run = runPosedNodes[i];
				blendPosedNodes[i].Name = mesh->Nodes[i].Name;
				blendPosedNodes[i].LocalTransform = SMatrix::Interpolate(walk.LocalTransform, run.LocalTransform, component->BlendValue);
			}

			SMatrix root;
			root.SetScale(component->ImportScale);
			ApplyLocalPoseToHierarchy(mesh, blendPosedNodes, mesh->Nodes[0], root);

			component->Bones.resize(mesh->BindPose.size());

			//Apply Inverse Bind Transform
			for (U32 i = 0; i < blendPosedNodes.size(); i++)
			{
				SSkeletalPosedNode& posedNode = blendPosedNodes[i];
				I32 boneIndex = -1;
				
				if (auto it = std::ranges::find(mesh->BindPose, posedNode.Name, &SSkeletalMeshBone::Name); it != mesh->BindPose.end())
					boneIndex = STATIC_I32(std::distance(mesh->BindPose.begin(), it));

				if (boneIndex < 0)
					continue;

				SSkeletalMeshBone& bone = mesh->BindPose[boneIndex];
				component->Bones[boneIndex] = bone.InverseBindPoseTransform * posedNode.GlobalTransform;
			}

			once = true;
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

	void CAnimatorGraphSystem::ApplyLocalPoseToHierarchy(const SSkeletalMeshComponent* mesh, std::vector<SSkeletalPosedNode>& in, const SSkeletalMeshNode& node, const SMatrix& parentTransform)
	{
		SMatrix nodeTransform = node.NodeTransform;

		auto it = std::ranges::find(in, node.Name, &SSkeletalPosedNode::Name);
		if (it != in.end())
			nodeTransform = it->LocalTransform;

		SMatrix globalTransform = nodeTransform * parentTransform;

		if (it != in.end())
			it->GlobalTransform = globalTransform;

		for (auto childNodeIndex : node.ChildIndices)
			ApplyLocalPoseToHierarchy(mesh, in, mesh->Nodes[childNodeIndex], globalTransform);
	}

	std::vector<SMatrix> CAnimatorGraphSystem::ReadAssetAnimationPose(const std::string& animationFile, const F32 animationTime)
	{
		SSkeletalAnimationComponent* component = new SSkeletalAnimationComponent();
		RenderManager->LoadSkeletalAnimationComponent(animationFile, component);

		SSkeletalMeshComponent* mesh = new SSkeletalMeshComponent();
		RenderManager->LoadSkeletalMeshComponent(component->SkeletonName, mesh);

		// Ticks == Frames
		F32 ticksPerSecond = STATIC_F32(component->CurrentAnimation[component->CurrentAnimationIndex].TickRate);
		ticksPerSecond = (ticksPerSecond != 0) ? ticksPerSecond : 24.0f;
		F32 timeInTicks = animationTime * ticksPerSecond;
		F32 duration = STATIC_F32(component->CurrentAnimation[component->CurrentAnimationIndex].DurationInTicks);
		F32 time = fmodf(timeInTicks, duration);

		std::vector<SSkeletalPosedNode> posedNodes = {};

		ReadAnimationLocalPose(component, mesh, time, mesh->Nodes[0], posedNodes);
		SMatrix root;
		root.SetScale(component->ImportScale);
		ApplyLocalPoseToHierarchy(mesh, posedNodes, mesh->Nodes[0], root);

		std::vector<SMatrix> transforms;
		transforms.resize(mesh->BindPose.size());

		//Apply Inverse Bind Transform
		for (U32 i = 0; i < posedNodes.size(); i++)
		{
			SSkeletalPosedNode& posedNode = posedNodes[i];
			I32 boneIndex = -1;

			if (auto it = std::ranges::find(mesh->BindPose, posedNode.Name, &SSkeletalMeshBone::Name); it != mesh->BindPose.end())
				boneIndex = STATIC_I32(std::distance(mesh->BindPose.begin(), it));

			if (boneIndex < 0)
				continue;

			SSkeletalMeshBone& bone = mesh->BindPose[boneIndex];
			transforms[boneIndex] = bone.InverseBindPoseTransform * posedNode.GlobalTransform;
		}

		delete component;
		delete mesh;

		return transforms;
	}

	void CAnimatorGraphSystem::ReadAnimationLocalPose(const SSkeletalAnimationComponent* animationComponent,
													  const SSkeletalMeshComponent* mesh,
													  const F32 animationTime,
													  const SSkeletalMeshNode& node,
													  std::vector<SSkeletalPosedNode>& posedNodes)
	{
		SMatrix nodeTransform = node.NodeTransform;
		// TODO.NW: Streamline this so we can read the local pose of different animations at the same time, then combine them at the end
		std::string nodeName = node.Name.AsString();

		SSkeletalPosedNode posedNode = SSkeletalPosedNode{};
		posedNodes.emplace_back(posedNode);
		posedNodes.back().Name = node.Name;

		auto& tracks = animationComponent->CurrentAnimation[animationComponent->CurrentAnimationIndex].Tracks;
		if (auto it = std::ranges::find(tracks, nodeName, &SBoneAnimationTrack::TrackName); it != tracks.end())
		{
			const SBoneAnimationTrack& track = *it;

			SVector scaling = CalcInterpolatedScaling(animationTime, track);
			SQuaternion rotation = CalcInterpolatedRotation(animationTime, track);
			SVector translation = CalcInterpolatedPosition(animationTime, track);

			SMatrix::Recompose(translation, rotation, scaling, nodeTransform);
			posedNodes.back().LocalTransform = nodeTransform;
		}
		else
		{
			posedNodes.back().LocalTransform = nodeTransform;
		}

		for (auto childIndex : node.ChildIndices)
		{
			ReadAnimationLocalPose(animationComponent, mesh, animationTime, mesh->Nodes[childIndex], posedNodes);
		}
	}

	std::vector<SSkeletalPosedNode> CAnimatorGraphSystem::EvaluateLocalPose(const SSkeletalAnimationComponent* animationComponent, const F32 animationTime)
	{
		std::vector<SSkeletalPosedNode> posedNodes;
		std::vector<SMatrix> localPose;
		for (const SBoneAnimationTrack& track : animationComponent->CurrentAnimation[animationComponent->CurrentAnimationIndex].Tracks)
		{
			//HV_LOG_WARN("bone :%s", track.TrackName.c_str());
			SVector scaling = CalcInterpolatedScaling(animationTime, track);
			SQuaternion rotation = CalcInterpolatedRotation(animationTime, track);
			SVector translation = CalcInterpolatedPosition(animationTime, track);

			SMatrix currentLocalPose = SMatrix::Identity;
			SMatrix::Recompose(translation, rotation, scaling, currentLocalPose);
			localPose.emplace_back(currentLocalPose);


			SSkeletalPosedNode pBone;
			pBone.Name = track.TrackName;
			pBone.LocalTransform = currentLocalPose;
			posedNodes.emplace_back(pBone);
		}

		return posedNodes;
	}
	void CAnimatorGraphSystem::TestWalkThrough(const SSkeletalMeshComponent* skeleton, const SSkeletalMeshNode& node, const SMatrix& parentTransform)
	{
		SMatrix matrix = node.NodeTransform * parentTransform;
		for (U32 i = 0; i < node.ChildIndices.size(); i++)
		{
			TestWalkThrough(skeleton, skeleton->Nodes[i], matrix);
		}
	}
}