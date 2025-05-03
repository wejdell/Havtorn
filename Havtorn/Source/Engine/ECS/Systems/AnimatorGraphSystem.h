// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"
#include "ECS/Entity.h"

namespace Havtorn
{
	struct SVecBoneAnimationKey;
	struct SQuatBoneAnimationKey;
	struct SSkeletalAnimationComponent;
	struct SSkeletalMeshComponent;
	struct SSkeletalMeshNode;
	struct SSkeletalPosedNode;
	class CRenderManager;



	class CAnimatorGraphSystem : public ISystem
	{
	public:
		CAnimatorGraphSystem(CRenderManager* renderManager);
		~CAnimatorGraphSystem() override = default;
		ENGINE_API void Update(CScene* scene) override;
		ENGINE_API void BindEvaluateFunction(std::function<I16(CScene*, const SEntity&)>& function, const std::string& classAndFunctionName);

		std::vector<SSkeletalPosedNode> EvaluateLocalPose(const SSkeletalAnimationComponent* animationComponent, const F32 animationTime);

		void TestWalkThrough(const SSkeletalMeshComponent* skeleton, const SSkeletalMeshNode& node, const SMatrix& parentTransform);

		void ReadAnimationLocalPose(const SSkeletalAnimationComponent* animationComponent, const SSkeletalMeshComponent* mesh, const F32 animationTime, const SSkeletalMeshNode& node, std::vector<SSkeletalPosedNode>& posedBoneOrder);
		void ApplyLocalPoseToHierarchy(const SSkeletalMeshComponent* mesh, std::vector<SSkeletalPosedNode>& in, const SSkeletalMeshNode& node, const SMatrix& parentTransform);

		ENGINE_API std::vector<SMatrix> ReadAssetAnimationPose(const std::string& animationFile, const F32 animationTime);

	private:
		CRenderManager* RenderManager;
		std::map<U64, std::function<I16(CScene*, const SEntity&)>> EvaluateFunctionMap;
	};
}