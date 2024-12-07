// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"
#include "ECS/Entity.h"

namespace Havtorn
{
	struct SSpriteAnimatorGraphComponent;
	class CSpriteAnimatorGraphSystem : public ISystem
	{
	public:
		~CSpriteAnimatorGraphSystem() override = default;
		HAVTORN_API void Update(CScene* scene) override;

		HAVTORN_API void BindEvaluateFunction(std::function<I16(CScene*, const SEntity&)>& function, const std::string& classAndFunctionName);

	private:
		SVector4 TickAnimationClip(SSpriteAnimatorGraphComponent& component, const F32 deltaTime);

		std::map<U64, std::function<I16(CScene*, const SEntity&)>> EvaluateFunctionMap;
	};
}