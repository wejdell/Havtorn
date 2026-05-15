// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include <ECS/System.h>

#include <CoreTypes.h>
#include <MathTypes/Vector.h>

namespace Havtorn
{
	class CEditorManager;
	struct SEntity;
	struct STransformComponent;
	struct SCameraComponent;
	struct SInputActionPayload;
	struct SInputAxisPayload;

	class CPickingSystem : public ISystem
	{
	public:
		CPickingSystem(CEditorManager* editorManager);
		virtual ~CPickingSystem() override;
		void Update(std::vector<Ptr<CScene>>& scenes) override;

	private:
		void OnMouseClick(const SInputActionPayload payload);
		void OnMouseMove(const SInputAxisPayload payload);
		void WorldSpacePick(const bool modifierHeld) const;
		void WorldSpaceContextPick();

		CEditorManager* Manager = nullptr;
		STransformComponent* EditorCameraTransform = nullptr;
		SVector2<F32> MousePosition = SVector2<F32>::Zero;
		SMatrix ContextPickStartingCameraMatrix = SMatrix::Identity;
	};
}
