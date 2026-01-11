// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"
#include "Input/InputTypes.h"

namespace Havtorn
{
	class CInputSystem : public ISystem
	{
	public:
		CInputSystem();
		~CInputSystem() override;

		void Update(std::vector<Ptr<CScene>>& scenes) override;


	private:
		void HandleMoveInput(const SInputAxisPayload payload);
		
		DelegateHandle DelegateAxisRightHandle;
		DelegateHandle DelegateAxisForwardHandle;

		SVector GlobalMoveInput;
	};
}