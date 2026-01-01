// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AnimatorGraphSystem.h"
#include "Graphics/RenderManager.h"
#include "ECS/Components/SkeletalAnimationComponent.h"
#include "ECS/Components/SkeletalMeshComponent.h"
#include "Scene/Scene.h"
#include "Assets/AssetRegistry.h"
#include "Assets/RuntimeAssetDeclarations.h"

namespace Havtorn
{
	CAnimatorGraphSystem::CAnimatorGraphSystem(CRenderManager* renderManager)
		: ISystem()
		, RenderManager(renderManager)
	{
	}

	void CAnimatorGraphSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		const F32 deltaTime = GTime::Dt();

		for (Ptr<CScene>& scene : scenes)
		{
			for (SSkeletalAnimationComponent* component : scene->GetComponents<SSkeletalAnimationComponent>())
			{
				if (!SComponent::IsValid(component))
					continue;

				SSkeletalMeshComponent* mesh = scene->GetComponent<SSkeletalMeshComponent>(component->Owner);
				if (!SComponent::IsValid(mesh))
					continue;

				CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
				const SSkeletalMeshAsset* meshAsset = assetRegistry->RequestAssetData<SSkeletalMeshAsset>(mesh->AssetReference, component->Owner.GUID);

				component->Bones.clear();
				F32 importScale = 1.0f;

				// Read local poses of playing animations
				for (SSkeletalAnimationPlayData& playData : component->PlayData)
				{
					if (!UMath::IsWithin(playData.AssetReferenceIndex, 0u, STATIC_U32(component->AssetReferences.size())))
						continue;

					const SSkeletalAnimationAsset* animationAsset = assetRegistry->RequestAssetData<SSkeletalAnimationAsset>(component->AssetReferences[playData.AssetReferenceIndex], component->Owner.GUID);
					if (animationAsset == nullptr)
						continue;

					importScale = animationAsset->ImportScale;

					playData.CurrentAnimationTime = fmodf(playData.CurrentAnimationTime += deltaTime, animationAsset->DurationInTicks / STATIC_F32(animationAsset->TickRate));

					const F32 tickRate = animationAsset->TickRate != 0 ? STATIC_F32(animationAsset->TickRate): 24.0f;
					const F32 animationTime = fmodf(playData.CurrentAnimationTime * tickRate, STATIC_F32(animationAsset->DurationInTicks));

					playData.LocalPosedNodes = {};
					ReadAnimationLocalPose(animationAsset, meshAsset, animationTime, meshAsset->Nodes[0], playData.LocalPosedNodes);
				}
				
				std::vector<SSkeletalPosedNode> posedNodes = {};
				posedNodes.resize(meshAsset->Nodes.size());
				
				// Blend animations
				if (component->PlayData.size() > 1)
				{
					// TODO.NW: Barycentric interpolation for more than 2 animations

					for (U32 i = 0; i < posedNodes.size(); i++)
					{
						const SSkeletalPosedNode& posedNodeA = component->PlayData[0].LocalPosedNodes[i];
						const SSkeletalPosedNode& posedNodeB = component->PlayData[1].LocalPosedNodes[i];

						posedNodes[i].Name = meshAsset->Nodes[i].Name;
						posedNodes[i].LocalTransform = SMatrix::Interpolate(posedNodeA.LocalTransform, posedNodeB.LocalTransform, component->BlendValue);
					}
				}
				else if (component->PlayData.size() > 0)
				{
					posedNodes = component->PlayData[0].LocalPosedNodes;
				}

				// Apply local pose and inverse bind transform
				SMatrix root = SMatrix::Identity;
				root.SetScale(importScale);
				ApplyLocalPoseToHierarchy(meshAsset, posedNodes, meshAsset->Nodes[0], root);

				component->Bones.resize(meshAsset->BindPoseBones.size(), SMatrix::Identity);
				for (U32 i = 0; i < posedNodes.size(); i++)
				{
					SSkeletalPosedNode& posedNode = posedNodes[i];
					I32 boneIndex = -1;
				
					if (auto it = std::ranges::find(meshAsset->BindPoseBones, posedNode.Name, &SSkeletalMeshBone::Name); it != meshAsset->BindPoseBones.end())
						boneIndex = STATIC_I32(std::distance(meshAsset->BindPoseBones.begin(), it));

					if (boneIndex < 0)
						continue;

					const SSkeletalMeshBone& bone = meshAsset->BindPoseBones[boneIndex];
					component->Bones[boneIndex] = bone.InverseBindPoseTransform * posedNode.GlobalTransform;
				}
			}
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

	void CAnimatorGraphSystem::ApplyLocalPoseToHierarchy(const SSkeletalMeshAsset* mesh, std::vector<SSkeletalPosedNode>& in, const SSkeletalMeshNode& node, const SMatrix& parentTransform)
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
		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		SSkeletalAnimationAsset* animationAsset = assetRegistry->RequestAssetData<SSkeletalAnimationAsset>(SAssetReference(animationFile), CAssetRegistry::EditorManagerRequestID);
		SSkeletalMeshAsset* meshAsset = assetRegistry->RequestAssetData<SSkeletalMeshAsset>(SAssetReference(animationAsset->RigPath), CAssetRegistry::EditorManagerRequestID);

		// Ticks == Frames
		F32 ticksPerSecond = STATIC_F32(animationAsset->TickRate);
		ticksPerSecond = (ticksPerSecond != 0) ? ticksPerSecond : 24.0f;
		F32 timeInTicks = animationTime * ticksPerSecond;
		F32 duration = STATIC_F32(animationAsset->DurationInTicks);
		F32 time = fmodf(timeInTicks, duration);

		std::vector<SSkeletalPosedNode> posedNodes = {};

		ReadAnimationLocalPose(animationAsset, meshAsset, time, meshAsset->Nodes[0], posedNodes);
		SMatrix root;
		root.SetScale(animationAsset->ImportScale);
		ApplyLocalPoseToHierarchy(meshAsset, posedNodes, meshAsset->Nodes[0], root);

		std::vector<SMatrix> transforms;
		transforms.resize(meshAsset->BindPoseBones.size());

		//Apply Inverse Bind Transform
		for (U32 i = 0; i < posedNodes.size(); i++)
		{
			SSkeletalPosedNode& posedNode = posedNodes[i];
			I32 boneIndex = -1;

			if (auto it = std::ranges::find(meshAsset->BindPoseBones, posedNode.Name, &SSkeletalMeshBone::Name); it != meshAsset->BindPoseBones.end())
				boneIndex = STATIC_I32(std::distance(meshAsset->BindPoseBones.begin(), it));

			if (boneIndex < 0)
				continue;

			SSkeletalMeshBone& bone = meshAsset->BindPoseBones[boneIndex];
			transforms[boneIndex] = bone.InverseBindPoseTransform * posedNode.GlobalTransform;
		}

		assetRegistry->UnrequestAsset(SAssetReference(animationAsset->RigPath), CAssetRegistry::EditorManagerRequestID);
		assetRegistry->UnrequestAsset(SAssetReference(animationFile), CAssetRegistry::EditorManagerRequestID);

		return transforms;
	}

	void CAnimatorGraphSystem::ReadAnimationLocalPose(const SSkeletalAnimationAsset* animation, const SSkeletalMeshAsset* mesh, const F32 animationTime, const SSkeletalMeshNode& fromNode, std::vector<SSkeletalPosedNode>& posedNodes)
	{
		SMatrix nodeTransform = fromNode.NodeTransform;
		// TODO.NW: Streamline this so we can read the local pose of different animations at the same time, then combine them at the end
		const std::string nodeName = fromNode.Name.AsString();

		posedNodes.emplace_back(SSkeletalPosedNode{});
		posedNodes.back().Name = fromNode.Name;

		const std::vector<SBoneAnimationTrack>& tracks = animation->BoneAnimationTracks;
		if (auto it = std::ranges::find(tracks, nodeName, &SBoneAnimationTrack::TrackName); it != tracks.end())
		{
			const SBoneAnimationTrack& track = *it;

			const SVector scaling = CalcInterpolatedScaling(animationTime, track);
			const SQuaternion rotation = CalcInterpolatedRotation(animationTime, track);
			const SVector translation = CalcInterpolatedPosition(animationTime, track);

			SMatrix::Recompose(translation, rotation, scaling, nodeTransform);
			posedNodes.back().LocalTransform = nodeTransform;
		}
		else
		{
			posedNodes.back().LocalTransform = nodeTransform;
		}

		for (const U32 childIndex : fromNode.ChildIndices)
		{
			ReadAnimationLocalPose(animation, mesh, animationTime, mesh->Nodes[childIndex], posedNodes);
		}
	}
}
