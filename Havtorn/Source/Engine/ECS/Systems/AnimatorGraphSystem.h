// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"
#include "ECS/Entity.h"

namespace Havtorn
{
	struct SVecBoneAnimationKey;
	struct SQuatBoneAnimationKey;
	struct SSkeletalAnimationAsset;
	struct SSkeletalMeshAsset;
	struct SSkeletalMeshNode;
	struct SSkeletalPosedNode;
	class CRenderManager;

	class CAnimatorGraphSystem : public ISystem
	{
	public:
		CAnimatorGraphSystem(CRenderManager* renderManager);
		~CAnimatorGraphSystem() override = default;
		ENGINE_API void Update(std::vector<Ptr<CScene>>& scenes) override;
		ENGINE_API void BindEvaluateFunction(std::function<I16(CScene*, const SEntity&)>& function, const std::string& classAndFunctionName);

		void ReadAnimationLocalPose(const SSkeletalAnimationAsset* animation, const SSkeletalMeshAsset* mesh, const F32 animationTime, const SSkeletalMeshNode& fromNode, std::vector<SSkeletalPosedNode>& posedBoneOrder);
		void ApplyLocalPoseToHierarchy(const SSkeletalMeshAsset* mesh, std::vector<SSkeletalPosedNode>& in, const SSkeletalMeshNode& node, const SMatrix& parentTransform);

		// TODO.NW: Make static function that additionally takes RenderManager arg?
		ENGINE_API std::vector<SMatrix> ReadAssetAnimationPose(const std::string& animationFile, const F32 animationTime);

	private:
		CRenderManager* RenderManager;
		std::map<U64, std::function<I16(CScene*, const SEntity&)>> EvaluateFunctionMap;
	};
}