// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Entity.h"
#include "ECS/System.h"
#include "Input/InputTypes.h"

namespace Havtorn 
{
	class CCameraSystem final : public ISystem {
	public:
		CCameraSystem();
		~CCameraSystem() override;

		void Update(CScene* scene) override;
		void HandleAxisInput(const SInputAxisPayload payload);
		void ToggleFreeCam(const SInputActionPayload payload);
		
		void OnBeginPlay(CScene* scene);
		void OnPausePlay(CScene* scene);
		void OnEndPlay(CScene* scene);
	private:
		void ResetInput();

	private:
		SVector CameraMoveInput = SVector::Zero;
		SVector CameraRotateInput = SVector::Zero;

		SEntity PreviousMainCamera = SEntity::Null;
		bool IsFreeCamActive = false;
	};
}
