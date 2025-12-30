// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Entity.h"
#include "ECS/System.h"
#include "Input/InputTypes.h"

namespace Havtorn
{
	class CPlatformManager;

	class CUISystem final : public ISystem
	{
	public:
		CUISystem(CPlatformManager* platformManager);
		~CUISystem() override;

		void Update(std::vector<Ptr<CScene>>& scenes) override;
		void HandleAxisInput(const SInputAxisPayload payload);
		void HandleMouseInput(const SInputActionPayload payload);

		ENGINE_API void BindEvaluateFunction(std::function<void()>& function, const std::string& classAndFunctionName);
		ENGINE_API std::string GetFunctionName(const U64 boundFunctionHash);

	private:
		std::map<U64, std::function<void()>> FunctionMap;
		std::map<U64, std::string> IdentifierMap;
		CPlatformManager* PlatformManager = nullptr;
		SVector2<F32> MousePosition = SVector2<F32>::Zero;
		SEntity FocusedCanvas = SEntity::Null;
		I8 FocusedElementIndex = -1;
		bool IsMouseClicked = false;
	};
}
