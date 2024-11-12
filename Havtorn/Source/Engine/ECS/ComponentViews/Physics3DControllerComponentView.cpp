// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Physics3DControllerComponentView.h"

#include "ECS/Components/Physics3DControllerComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult SPhysics3DControllerComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		if (!ImGui::UUtils::TryOpenComponentView("Physics3D"))
			return {};

		SPhysics3DControllerComponent* physicsComponent = scene->GetComponent<SPhysics3DControllerComponent>(entityOwner);

		SVector Velocity = SVector::Zero;

		// TODO.NR: Make a util to deal with enums
		I32 controllerTypeIndex = static_cast<int>(physicsComponent->ControllerType);
		const char* controllerTypeNames[2] = { "Box", "Capsule" };
		ImGui::SliderInt("Controller Type", &controllerTypeIndex, 0, 1, controllerTypeNames[controllerTypeIndex]);
		physicsComponent->ControllerType = static_cast<Havtorn::EPhysics3DControllerType>(controllerTypeIndex);

		float localExtents[2] = { physicsComponent->ShapeLocalExtents.X, physicsComponent->ShapeLocalExtents.Y };
		ImGui::DragFloat2("Shape Local Extents", localExtents, ImGui::UUtils::SliderSpeed);
		physicsComponent->ShapeLocalExtents = { localExtents[0], localExtents[1] };

		ImGui::Text("Velocity: %s", physicsComponent->Velocity.ToString().c_str());

		// TODO.NR: Most of these should only be changed during setup, but if we want a truly responsive editor we can pause
		// during play and unpause, we should probably handle setting the data on physics wrapper entity if we make modifications here.

		return {};
    }
}
