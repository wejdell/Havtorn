// Copyright 2022 Team Havtorn. All Rights Reserved.

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
		void Update(CScene* scene) override;

	public:
		std::function<I16(CScene*, U64)> EvaluateIdleFunc;
		std::function<I16(CScene*, U64)> EvaluateLocomotionFunc;
	private:
		I16 EvaluateIdle(CScene* scene, U64 entitySceneIndex);
		I16 EvaluateLocomotion(CScene* scene, U64 entitySceneIndex);

	private:
		void HandleAxisInput(const SInputAxisPayload payload);
		void ResetInput();
		SVector input;
	};
}