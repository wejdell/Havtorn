// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Physics2DComponentEditorContext.h"

#include "ECS/Components/Physics2DComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	SPhysics2DComponentEditorContext SPhysics2DComponentEditorContext::Context = {};

    SComponentViewResult SPhysics2DComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("Physics2D"))
			return {};

		SPhysics2DComponent* physicsComponent = scene->GetComponent<SPhysics2DComponent>(entityOwner);

		// TODO.NR: Make a util to deal with enums
		I32 bodyTypeIndex = static_cast<int>(physicsComponent->BodyType);
		const char* bodyTypeNames[3] = { "Static", "Kinematic", "Dynamic" };
		GUI::SliderInt("Body Type", &bodyTypeIndex, 0, 2, bodyTypeNames[bodyTypeIndex]);
		physicsComponent->BodyType = static_cast<Havtorn::EPhysics2DBodyType>(bodyTypeIndex);

		I32 shapeTypeIndex = static_cast<int>(physicsComponent->ShapeType);
		const char* shapeTypeNames[4] = { "Circle", "Capsule", "Segment", "Polygon" };
		GUI::SliderInt("Shape Type", &shapeTypeIndex, 0, 3, shapeTypeNames[shapeTypeIndex]);
		physicsComponent->ShapeType = static_cast<Havtorn::EPhysics2DShapeType>(shapeTypeIndex);

		float localOffset[2] = { physicsComponent->ShapeLocalOffset.X, physicsComponent->ShapeLocalOffset.Y };
		GUI::DragFloat2("Shape Local Offset", localOffset, GUI::SliderSpeed);
		physicsComponent->ShapeLocalOffset = { localOffset[0], localOffset[1] };

		float localExtents[2] = { physicsComponent->ShapeLocalExtents.X, physicsComponent->ShapeLocalExtents.Y };
		GUI::DragFloat2("Shape Local Extents", localExtents, GUI::SliderSpeed);
		physicsComponent->ShapeLocalExtents = { localExtents[0], localExtents[1] };

		GUI::Text("Velocity: %s", physicsComponent->Velocity.ToString().c_str());

		GUI::Checkbox("Constrain Rotation", &physicsComponent->ConstrainRotation);

		// TODO.NR: Most of these should only be changed during setup, but if we want a truly responsive editor we can pause
		// during play and unpause, we should probably handle setting the data on physics wrapper entity if we make modifications here.

		return {};
    }

	bool SPhysics2DComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Physics 2D Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SPhysics2DComponent>(entity);
		scene->AddComponentEditorContext(entity, &SPhysics2DComponentEditorContext::Context);
		return true;
	}

	bool SPhysics2DComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##7"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SPhysics2DComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SPhysics2DComponentEditorContext::Context);
		return true;
	}
}
