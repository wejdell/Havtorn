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

	class CAnimatorGraphSystem : public ISystem
	{
	public:
		CAnimatorGraphSystem(CRenderManager* renderManager);
		~CAnimatorGraphSystem() override = default;
		HAVTORN_API void Update(CScene* scene) override;

		HAVTORN_API void BindEvaluateFunction(std::function<I16(CScene*, const SEntity&)>& function, const std::string& classAndFunctionName);

	private:
		CRenderManager* RenderManager;
		std::map<U64, std::function<I16(CScene*, const SEntity&)>> EvaluateFunctionMap;
	};
}