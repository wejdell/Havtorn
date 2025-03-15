// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include <ECS/System.h>

#include <CoreTypes.h>
#include <MathTypes/Vector.h>

namespace Havtorn
{
	class CEditorManager;
	struct STransformComponent;
	struct SCameraComponent;
	struct SInputActionPayload;
	struct SInputAxisPayload;

	class CPickingSystem : public ISystem
	{
	public:
		CPickingSystem(CEditorManager* editorManager);
		void Update(CScene* scene) override;

	private:
		void OnMouseClick(const SInputActionPayload payload) const;
		void OnMouseMove(const SInputAxisPayload payload);
		void WorldSpacePick() const;

		CEditorManager* Manager = nullptr;
		STransformComponent* EditorCameraTransform = nullptr;
		SCameraComponent* EditorCameraComponent = nullptr;
		SVector2<F32> MousePosition = SVector2<F32>::Zero;
	};
}
