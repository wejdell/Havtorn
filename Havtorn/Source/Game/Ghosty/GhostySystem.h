// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn
{
	class CGhostySystem : public ISystem
	{
	public:
		CGhostySystem();
		~CGhostySystem() override = default;
		void Update(std::vector<Ptr<CScene>>& scenes) override;

	public:
		std::function<I16(CScene*, const SEntity&)> EvaluateIdleFunc;
		std::function<I16(CScene*, const SEntity&)> EvaluateLocomotionFunc;

	private:
		I16 EvaluateIdle(CScene* scene, const SEntity& entity);
		I16 EvaluateLocomotion(CScene* scene, const SEntity& entity);

		void HandleAxisInput(const SInputAxisPayload payload);
		void ResetInput();
		SVector input;
	};
}