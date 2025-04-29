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
				
				SVector walkPos;
				SQuaternion walkRot;
				SVector walkScale;
				SMatrix::Decompose(walk.localTransform, walkPos, walkRot, walkScale);

				SVector runPos;
				SQuaternion runRot;
				SVector runScale;
				SMatrix::Decompose(run.localTransform, runPos, runRot, runScale);

				SVector blendPos = walkPos;
					//SVector::Lerp(walkPos, runPos, component->BlendValue);
				SQuaternion blendRot = walkRot;
					//SQuaternion::Slerp(walkRot, runRot, component->BlendValue).GetNormalized();
				SVector blendScale = walkScale;
					//SVector::Lerp(walkScale, runScale, component->BlendValue);

				SMatrix blendMatrix = SMatrix::Identity;
				SMatrix::Recompose(blendPos, blendRot, blendScale, blendMatrix);

				blendPosedNodes[i].localTransform = blendMatrix;
					//walk.localTransform;
					//SMatrix::Interpolate(walk.localTransform, run.localTransform, component->BlendValue);
			}

			//blendPosedNodes = runPosedNodes;
			ApplyLocalPoseToHierarchy(mesh, blendPosedNodes, mesh->Nodes[0], SMatrix::Identity);


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
				component->Bones[boneIndex] = bone.InverseBindPoseTransform * posedNode.globalTransform;
			}

			once = true;
#pragma region Test Blend

			//Node Transforms 240

			//KeyFrame Tracks 140

			//Bones *= inverseBindPose 49

			//component->CurrentAnimationIndex = 0;


			//U32 size = STATIC_U32(composedBonesInOrderOne.size());
			//std::vector<SPosedBone> blendedBones;
			//blendedBones.resize(size);
			//for (U32 i = 0; i < size; i++)
			//{
			//	SPosedBone& a = composedBonesInOrderOne[i];
			//	SPosedBone& b = composedBonesInOrderTwo[i];
			//	
			//	SPosedBone blendedBone;
			//	blendedBone.localTransform = SMatrix::Interpolate(a.localTransform, b.localTransform, component->BlendValue);
			//	blendedBone.boneIndex = a.boneIndex;

			//	/*SVector posA;
			//				SQuaternion rotA;
			//				SVector scaleA;
			//				SMatrix::Decompose(a.globalTransform, posA, rotA, scaleA);
			//				SVector posB;
			//				SQuaternion rotB;
			//				SVector scaleB;
			//				SMatrix::Decompose(b.globalTransform, posB, rotB, scaleB);
			//				SVector blendPos = SVector::Lerp(posA, posB, component->BlendValue);
			//				SQuaternion blendRot = SQuaternion::Slerp(rotA, rotB, component->BlendValue);
			//				SVector blendScale = SVector::Lerp(scaleA, scaleB, component->BlendValue);
			//				SMatrix blendedMatrix = SMatrix::Identity;
			//				SMatrix::Recompose(blendPos, blendRot, blendScale, blendedMatrix);
			//				SPosedBone blendedBone;
			//				blendedBone.globalTransform = blendedMatrix;
			//				blendedBone.boneIndex = a.boneIndex;*/

			//	blendedBones[i] = blendedBone;
			//}

			/*std::vector<SPosedBone> outBones{};
			outBones.resize(blendedBones.size());*/


			//SimplePass(mesh, composedBonesInOrderOne, 0, mesh->Nodes[0].NodeTransform);

			//component->Bones.clear();
			//component->Bones.resize(blendedBones.size());
			//for (U32 i = 0; i < blendedBones.size(); i++)
			//{
			//	if (blendedBones[i].hasGlobalTransform)
			//	{
			//		SPosedBone& posedNode = blendedBones[i];
			//		component->Bones[i] = mesh->BindPose[posedBone.boneIndex].InverseBindPoseTransform * posedNode.globalTransform;
			//	}
			//}


						// ^ Local Space Pose (???)

						//We want to move this Blended pose into ModelSpace - by composing it aye?? (????)
						//std::vector<SPosedBone> modelSpaceBones;
						//modelSpaceBones.resize(blendedBones.size());
						//SimplePass(mesh, blendedBones, modelSpaceBones, 0, SMatrix::Identity);

						////Using Original Solution & able to Transform into ModelSpace outside Original Solution
						////for (auto& posedNode : modelSpaceBones)
						////{
						////	posedBone.finalTransform = mesh->BindPose[posedBone.boneIndex].InverseBindPoseTransform * posedNode.globalTransform;
						////}

						//for (U32 i = 0; i < modelSpaceBones.size(); i++)
						//{
						//	SPosedBone& posedNode = modelSpaceBones[i];
						//	posedBone.finalTransform = posedNode.globalTransform*  mesh->BindPose[posedBone.boneIndex].InverseBindPoseTransform;
						//}

						//component->Bones = std::vector<SMatrix>(modelSpaceBones.size(), SMatrix());
						//for (U32 i = 0; i < modelSpaceBones.size(); i++)
						//{
						//	SPosedBone& posedNode = modelSpaceBones[i];
						//	component->Bones[i] = posedNode.finalTransform; //mesh->BindPose[posedBone.boneIndex].InverseBindPoseTransform* posedNode.globalTransform;
						//}
#pragma endregion						 
#pragma region IdeaOne			
			//std::vector<SMatrix> ideaOne = component->Bones;

			//component->Bones.clear();
			//component->Bones = std::vector<SMatrix>(composedBonesInOrder.size(), SMatrix());

			////Entierly new solution - Only walking through Nodes and multiplying in BindPose to the Composed Bones.
			//SMatrix composedTransform = mesh->Nodes[0].NodeTransform;
			//std::vector<SPosedBone> composedBones;
			//SimplePass(mesh, mesh->Nodes[0], composedTransform, composedBones);

			//for (U32 i = 0; i < composedBones.size(); i++)
			//{
			//	SPosedBone& posedNode = composedBones[i];
			//	component->Bones[i] = mesh->BindPose[posedBone.boneIndex].InverseBindPoseTransform * posedNode.globalTransform;
			//}
#pragma endregion
#pragma region Version 1
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
#pragma endregion
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
			nodeTransform = it->localTransform;

		SMatrix globalTransform = nodeTransform * parentTransform;

		if (it != in.end())
			it->globalTransform = globalTransform;

		for (auto childNodeIndex : node.ChildIndices)
		{
			ApplyLocalPoseToHierarchy(mesh, in, mesh->Nodes[childNodeIndex], globalTransform);
		}
	}

	//std::string nodeName = mesh->Nodes[nodeIndex].Name.AsString();
	//SMatrix nodeTransform = mesh->Nodes[nodeIndex].NodeTransform;

	//auto it = std::ranges::find(in, nodeName, &SPosedBone::Name);	
	//if (it != in.end())
	//{
	//	nodeTransform = it->localTransform;
	//}

	//SMatrix globalTransform = nodeTransform * parentTransform;

	//auto itBindPose = std::ranges::find(mesh->BindPose, nodeName, &SSkeletalMeshBone::Name);
	//if (itBindPose != mesh->BindPose.end())
	//{
	//	I32 newBoneIndex = STATIC_I32(std::distance(mesh->BindPose.begin(), itBindPose));
	//	it->boneIndex = newBoneIndex;
	//	it->globalTransform = globalTransform;
	//	it->hasGlobalTransform = true;
	//}

	//for (auto childIndex : mesh->Nodes[nodeIndex].ChildIndices)
	//{
	//	SimplePass(mesh, in, childIndex, globalTransform);
	//}


	//SMatrix global = parentTransform;
	//if (boneIndex < in.size())
	//{
	//	out[boneIndex].globalTransform = in[boneIndex].localTransform * parentTransform;
	//	global = out[boneIndex].globalTransform;
	//}

	//for (U32 i = 0; i < mesh->Nodes[boneIndex].ChildIndices.size(); i++)
	//{
	//	SimplePass(mesh, in, out, mesh->Nodes[boneIndex].ChildIndices[i], global);
	//}
//std::string nodeName = node.Name.AsString();

//SMatrix globalTransform = node.NodeTransform * composedTransform;

//if (auto it = std::ranges::find(mesh->BindPose, nodeName, &SSkeletalMeshBone::Name); it != mesh->BindPose.end())
//{
//	I32 boneIndex = STATIC_I32(std::distance(mesh->BindPose.begin(), it));
//	SPosedBone posedNode = {};
//	posedBone.boneIndex = boneIndex;
//	posedBone.globalTransform = globalTransform;
//	posedBone.finalTransform = SMatrix::Identity;
//	outComposedBones.push_back(posedBone);
//	HV_LOG_WARN("Simple Pass Index: %i", boneIndex);
//}

//for (auto childNodeIndex : node.ChildIndices)
//{
//	SimplePass(mesh, mesh->Nodes[childNodeIndex], globalTransform, outComposedBones);
//}

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

		auto& tracks = animationComponent->CurrentAnimation[animationComponent->CurrentAnimationIndex].tracks;
		if (auto it = std::ranges::find(tracks, nodeName, &SBoneAnimationTrack::TrackName); it != tracks.end())
		{
			const SBoneAnimationTrack& track = *it;

			SVector scaling = CalcInterpolatedScaling(animationTime, track);
			SQuaternion rotation = CalcInterpolatedRotation(animationTime, track);
			SVector translation = CalcInterpolatedPosition(animationTime, track);

			SMatrix::Recompose(translation, rotation, scaling, nodeTransform);
			posedNodes.back().localTransform = nodeTransform;
		}
		else
		{
			posedNodes.back().localTransform = nodeTransform;
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
		for (const SBoneAnimationTrack& track : animationComponent->CurrentAnimation[animationComponent->CurrentAnimationIndex].tracks)
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
			pBone.localTransform = currentLocalPose;
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