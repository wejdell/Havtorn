// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Physics3DComponentView.h"

#include "ECS/Components/Physics3DComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult SPhysics3DComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		if (!ImGui::UUtils::TryOpenComponentView("Physics3D"))
			return {};

		SPhysics3DComponent* physicsComponent = scene->GetComponent<SPhysics3DComponent>(entityOwner);

		// TODO.NR: Support multiple shapes on same body
		
		SPhysicsMaterial Material = {};

		// TODO.NR: Make a util to deal with enums
		I32 bodyTypeIndex = static_cast<int>(physicsComponent->BodyType);
		const char* bodyTypeNames[3] = { "Static", "Kinematic", "Dynamic" };
		ImGui::SliderInt("Body Type", &bodyTypeIndex, 0, 2, bodyTypeNames[bodyTypeIndex]);
		physicsComponent->BodyType = static_cast<Havtorn::EPhysics3DBodyType>(bodyTypeIndex);

		I32 shapeTypeIndex = static_cast<int>(physicsComponent->ShapeType);
		const char* shapeTypeNames[5] = { "Sphere", "Plane", "Capsule", "Box", "Convex" };
		ImGui::SliderInt("Shape Type", &shapeTypeIndex, 0, 4, shapeTypeNames[shapeTypeIndex]);
		physicsComponent->ShapeType = static_cast<Havtorn::EPhysics3DShapeType>(shapeTypeIndex);

		float localOffset[3] = { physicsComponent->ShapeLocalOffset.X, physicsComponent->ShapeLocalOffset.Y, physicsComponent->ShapeLocalOffset.Z };
		ImGui::DragFloat3("Shape Local Offset", localOffset, ImGui::UUtils::SliderSpeed);
		physicsComponent->ShapeLocalOffset = { localOffset[0], localOffset[1], localOffset[2] };

		float localExtents[3] = { physicsComponent->ShapeLocalExtents.X, physicsComponent->ShapeLocalExtents.Y, physicsComponent->ShapeLocalOffset.Z };
		ImGui::DragFloat3("Shape Local Extents", localExtents, ImGui::UUtils::SliderSpeed);
		physicsComponent->ShapeLocalExtents = { localExtents[0], localExtents[1], localExtents[2] };

		ImGui::TextDisabled("Material");

		ImGui::DragFloat("Dynamic Friction", &physicsComponent->Material.DynamicFriction);
		ImGui::DragFloat("Static Friction", &physicsComponent->Material.StaticFriction);
		ImGui::DragFloat("Restitution", &physicsComponent->Material.Restitution);

		ImGui::Text("Velocity: %s", physicsComponent->Velocity.ToString().c_str());

		ImGui::DragFloat("Density", &physicsComponent->Density);

		ImGui::Checkbox("Is Trigger", &physicsComponent->IsTrigger);

		// TODO.NR: Most of these should only be changed during setup, but if we want a truly responsive editor we can pause
		// during play and unpause, we should probably handle setting the data on physics wrapper entity if we make modifications here.

		return {};
    }
}
