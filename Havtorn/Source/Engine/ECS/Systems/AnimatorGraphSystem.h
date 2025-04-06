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

	class CRenderManager;

	class CAnimatorGraphSystem : public ISystem
	{
	public:
		CAnimatorGraphSystem(CRenderManager* renderManager);
		~CAnimatorGraphSystem() override = default;
		ENGINE_API void Update(CScene* scene) override;

		ENGINE_API void BindEvaluateFunction(std::function<I16(CScene*, const SEntity&)>& function, const std::string& classAndFunctionName);

		std::vector<SMatrix> EvaluateLocalPose(const SSkeletalAnimationComponent* animationComponent, const F32 animationTime);
		void ReadHierarchy(const SSkeletalAnimationComponent* animationComponent, const SSkeletalMeshComponent* mesh, const F32 animationTime, const SMatrix& parentTransform, const SSkeletalMeshNode& node, std::vector<SMatrix>& posedTransforms);

	private:
		CRenderManager* RenderManager;
		std::map<U64, std::function<I16(CScene*, const SEntity&)>> EvaluateFunctionMap;
	};
}