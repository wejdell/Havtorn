// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Physics3DComponentEditorContext.h"

#include "ECS/Components/Physics3DComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>

namespace Havtorn
{
	SPhysics3DComponentEditorContext SPhysics3DComponentEditorContext::Context = {};

    SComponentViewResult SPhysics3DComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("Physics3D"))
			return {};

		SPhysics3DComponent* physicsComponent = scene->GetComponent<SPhysics3DComponent>(entityOwner);

		// TODO.NR: Make a util to deal with enums
		I32 bodyTypeIndex = static_cast<int>(physicsComponent->BodyType);
		const char* bodyTypeNames[3] = { "Static", "Kinematic", "Dynamic" };
		GUI::SliderInt("Body Type", &bodyTypeIndex, 0, 2, bodyTypeNames[bodyTypeIndex]);
		physicsComponent->BodyType = static_cast<Havtorn::EPhysics3DBodyType>(bodyTypeIndex);

		I32 shapeTypeIndex = static_cast<int>(physicsComponent->ShapeType);
		const char* shapeTypeNames[5] = { "Sphere", "Plane", "Capsule", "Box", "Convex" };
		GUI::SliderInt("Shape Type", &shapeTypeIndex, 0, 4, shapeTypeNames[shapeTypeIndex]);
		physicsComponent->ShapeType = static_cast<Havtorn::EPhysics3DShapeType>(shapeTypeIndex);

		GUI::DragFloat3("Shape Local Offset", physicsComponent->ShapeLocalOffset, GUI::SliderSpeed);

		switch (physicsComponent->ShapeType)
		{
		case EPhysics3DShapeType::Sphere:
		{
			GUI::DragFloat("Shape Local Radius", &physicsComponent->ShapeLocalRadius, GUI::SliderSpeed);
		}
			break;
		case EPhysics3DShapeType::InfinitePlane:
			break;
		case EPhysics3DShapeType::Capsule:
		{
			F32 localExtents[2] = { physicsComponent->ShapeLocalRadiusAndHeight.X, physicsComponent->ShapeLocalRadiusAndHeight.Y };
			GUI::DragFloat2("Shape Local Radius And Height", localExtents, GUI::SliderSpeed);
			physicsComponent->ShapeLocalRadiusAndHeight = { localExtents[0], localExtents[1] };
		}
			break;
		case EPhysics3DShapeType::Box:
		{
			GUI::DragFloat3("Shape Local Extents", physicsComponent->ShapeLocalExtents, GUI::SliderSpeed);
		}
			break;
		case EPhysics3DShapeType::Convex:
			break;
		}

		GUI::TextDisabled("Material");

		GUI::DragFloat("Dynamic Friction", &physicsComponent->Material.DynamicFriction);
		GUI::DragFloat("Static Friction", &physicsComponent->Material.StaticFriction);
		GUI::DragFloat("Restitution", &physicsComponent->Material.Restitution);

		GUI::Separator();

		GUI::Text("Velocity: %s", physicsComponent->Velocity.ToString().c_str());

		GUI::DragFloat("Density", &physicsComponent->Density);

		GUI::Checkbox("Is Trigger", &physicsComponent->IsTrigger);

		// TODO.NR: Most of these should only be changed during setup, but if we want a truly responsive editor we can pause
		// during play and unpause, we should probably handle setting the data on physics wrapper entity if we make modifications here.

		return {};
    }

	bool SPhysics3DComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Physics 3D Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SPhysics3DComponent>(entity);
		scene->AddComponentEditorContext(entity, &SPhysics3DComponentEditorContext::Context);
		return true;
	}

	bool SPhysics3DComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##8"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SPhysics3DComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SPhysics3DComponentEditorContext::Context);
		return true;
	}
}
