// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"
#include "ECS/Entity.h"

namespace Havtorn
{
	struct SVecBoneAnimationKey;
	struct SQuatBoneAnimationKey;
	struct SSkeletalAnimationComponent;

	class CRenderManager;

	// TODO.NR: Store this in component instead? Preprocess as part of component loading?
	struct SBoneAnimDataTransform
	{
		SVector4 Row1TX;
		SVector4 Row2TY;
		SVector4 Row3TZ;
		SVector4 Padding;
	};

	class CAnimatorGraphSystem : public ISystem
	{
	public:
		CAnimatorGraphSystem(CRenderManager* renderManager);
		~CAnimatorGraphSystem() override = default;
		HAVTORN_API void Update(CScene* scene) override;

		HAVTORN_API void BindEvaluateFunction(std::function<I16(CScene*, const SEntity&)>& function, const std::string& classAndFunctionName);

	private:
		// Pre-processing
		std::vector<SBoneAnimDataTransform> PreprocessAnimation(SSkeletalAnimationComponent* component);
		SBoneAnimDataTransform EncodeTransform(const SVecBoneAnimationKey& translationKey, const SQuatBoneAnimationKey& rotationKey, const SVecBoneAnimationKey& scaleKey);

	private:
		CRenderManager* RenderManager;
		std::map<U64, std::function<I16(CScene*, const SEntity&)>> EvaluateFunctionMap;
	};
}