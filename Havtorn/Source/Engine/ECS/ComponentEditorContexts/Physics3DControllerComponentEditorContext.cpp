// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Physics3DControllerComponentEditorContext.h"

#include "ECS/Components/Physics3DControllerComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	SPhysics3DControllerComponentEditorContext SPhysics3DControllerComponentEditorContext::Context = {};

    SComponentViewResult SPhysics3DControllerComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("Physics3D"))
			return {};

		SPhysics3DControllerComponent* physicsComponent = scene->GetComponent<SPhysics3DControllerComponent>(entityOwner);

		// TODO.NR: Make a util to deal with enums
		I32 controllerTypeIndex = static_cast<int>(physicsComponent->ControllerType);
		const char* controllerTypeNames[2] = { "Box", "Capsule" };
		GUI::SliderInt("Controller Type", &controllerTypeIndex, 0, 1, controllerTypeNames[controllerTypeIndex]);
		physicsComponent->ControllerType = static_cast<Havtorn::EPhysics3DControllerType>(controllerTypeIndex);

		switch (physicsComponent->ControllerType)
    	{
		case EPhysics3DControllerType::Box:
		{
			GUI::DragFloat3("Shape Local Extents", physicsComponent->ShapeLocalExtents, GUI::SliderSpeed);
		}
			break;
		case EPhysics3DControllerType::Capsule:
		{
			F32 localExtents[2] = { physicsComponent->ShapeLocalRadiusAndHeight.X, physicsComponent->ShapeLocalRadiusAndHeight.Y };
			GUI::DragFloat2("Shape Local Radius And Height", localExtents, GUI::SliderSpeed);
			physicsComponent->ShapeLocalRadiusAndHeight = { localExtents[0], localExtents[1] };
		}
			break;
		}

		GUI::Text("Velocity: %s", physicsComponent->Velocity.ToString().c_str());

		// TODO.NR: Most of these should only be changed during setup, but if we want a truly responsive editor we can pause
		// during play and unpause, we should probably handle setting the data on physics wrapper entity if we make modifications here.

		return {};
    }

	bool SPhysics3DControllerComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Physics 3D Controller Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SPhysics3DControllerComponent>(entity);
		scene->AddComponentEditorContext(entity, &SPhysics3DControllerComponentEditorContext::Context);
		return true;
	}

	bool SPhysics3DControllerComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##9"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SPhysics3DControllerComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SPhysics3DControllerComponentEditorContext::Context);
		return true;
	}
}
