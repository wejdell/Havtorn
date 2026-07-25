// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Physics2DComponentView.h"

#include "ECS/Components/Physics2DComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	SPhysics2DComponentView SPhysics2DComponentView::Context = {};

    SComponentViewResult SPhysics2DComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		SPhysics2DComponent* physicsComponent = scene->GetComponent<SPhysics2DComponent>(entityOwner);

		GUI::SliderEnum("Body Type", physicsComponent->BodyType, { "Static", "Kinematic", "Dynamic" });
		GUI::SliderEnum("Shape Type", physicsComponent->ShapeType, { "Circle", "Capsule", "Segment", "Polygon" });

		GUI::DragFloat2("Shape Local Offset", physicsComponent->ShapeLocalOffset, GUI::SliderSpeed);
		GUI::DragFloat2("Shape Local Extents", physicsComponent->ShapeLocalExtents, GUI::SliderSpeed);

		GUI::Text("Velocity: %s", physicsComponent->Velocity.ToString().c_str());

		GUI::Checkbox("Constrain Rotation", physicsComponent->ConstrainRotation);

		// TODO.NR: Most of these should only be changed during setup, but if we want a truly responsive editor we can pause
		// during play and unpause, we should probably handle setting the data on physics wrapper entity if we make modifications here.

		return {};
    }
}
