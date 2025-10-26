// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Entity.h"
#include "ECS/System.h"
#include "Input/InputTypes.h"

namespace Havtorn 
{
	class CCameraSystem final : public ISystem 
	{
	public:
		CCameraSystem();
		~CCameraSystem() override;

		void Update(std::vector<Ptr<CScene>>& scenes) override;
		void HandleAxisInput(const SInputAxisPayload payload);
		void ToggleFreeCam(const SInputActionPayload payload);
		
		void OnBeginPlay(std::vector<Ptr<CScene>>& scenes);
		void OnPausePlay(std::vector<Ptr<CScene>>& scenes);
		void OnEndPlay(std::vector<Ptr<CScene>>& scenes);
	private:
		void ResetInput();

	private:
		SVector CameraMoveInput = SVector::Zero;
		SVector CameraRotateInput = SVector::Zero;

		SEntity PreviousMainCamera = SEntity::Null;
		bool IsFreeCamActive = false;
	};
}
