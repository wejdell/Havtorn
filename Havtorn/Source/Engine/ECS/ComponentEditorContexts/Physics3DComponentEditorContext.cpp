// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Physics3DComponentEditorContext.h"

#include "ECS/Components/Physics3DComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/ComponentAlgo.h"
#include "Scene/Scene.h"

#include "Graphics/Debug/DebugDrawUtility.h"

#include <GUI.h>

namespace Havtorn
{
	SPhysics3DComponentEditorContext SPhysics3DComponentEditorContext::Context = {};

    SComponentViewResult SPhysics3DComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		SPhysics3DComponent* physicsComponent = scene->GetComponent<SPhysics3DComponent>(entityOwner);

		GUI::SliderEnum("Body Type", physicsComponent->BodyType, { "Static", "Kinematic", "Dynamic" });
		GUI::SliderEnum("Shape Type", physicsComponent->ShapeType, { "Sphere", "Plane", "Capsule", "Box", "Convex" });

		GUI::DragFloat3("Shape Local Offset", physicsComponent->ShapeLocalOffset, GUI::SliderSpeed);

		switch (physicsComponent->ShapeType)
		{
		case EPhysics3DShapeType::Sphere:
		{
			GUI::DragFloat("Shape Local Radius", physicsComponent->ShapeLocalRadius, GUI::SliderSpeed);
		}
			break;
		case EPhysics3DShapeType::InfinitePlane:
			break;
		case EPhysics3DShapeType::Capsule:
		{
			GUI::DragFloat2("Shape Local Radius And Height", physicsComponent->ShapeLocalRadiusAndHeight, GUI::SliderSpeed);
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

		GUI::DragFloat("Dynamic Friction", physicsComponent->Material.DynamicFriction);
		GUI::DragFloat("Static Friction", physicsComponent->Material.StaticFriction);
		GUI::DragFloat("Restitution", physicsComponent->Material.Restitution);

		GUI::Separator();

		GUI::Text("Velocity: %s", physicsComponent->Velocity.ToString().c_str());

		GUI::DragFloat("Density", physicsComponent->Density);

		GUI::Checkbox("Is Trigger", physicsComponent->IsTrigger);

		if (!physicsComponent->IsTrigger)
			return {};

		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transform))
			return {};

		// TODO.NW: Add debug drawing for other shapes
		if (physicsComponent->ShapeType != EPhysics3DShapeType::Box)
			return {};

		const SVector boundsMin = -physicsComponent->ShapeLocalExtents * 0.5f;
		const SVector boundsMax = physicsComponent->ShapeLocalExtents * 0.5f;
		SVector a = SVector(boundsMin.X, boundsMin.Y, boundsMin.Z);
		SVector b = SVector(boundsMin.X, boundsMin.Y, boundsMax.Z);
		SVector c = SVector(boundsMax.X, boundsMin.Y, boundsMax.Z);
		SVector d = SVector(boundsMax.X, boundsMin.Y, boundsMin.Z);
		SVector e = SVector(boundsMin.X, boundsMax.Y, boundsMin.Z);
		SVector f = SVector(boundsMax.X, boundsMax.Y, boundsMin.Z);
		SVector g = SVector(boundsMax.X, boundsMax.Y, boundsMax.Z);
		SVector h = SVector(boundsMin.X, boundsMax.Y, boundsMax.Z);

		SMatrix transformMatrix = transform->Transform.GetMatrix();

		a = (SVector4(a + physicsComponent->ShapeLocalOffset, 1.0f) * transformMatrix).ToVector3();
		b = (SVector4(b + physicsComponent->ShapeLocalOffset, 1.0f) * transformMatrix).ToVector3();
		c = (SVector4(c + physicsComponent->ShapeLocalOffset, 1.0f) * transformMatrix).ToVector3();
		d = (SVector4(d + physicsComponent->ShapeLocalOffset, 1.0f) * transformMatrix).ToVector3();
		e = (SVector4(e + physicsComponent->ShapeLocalOffset, 1.0f) * transformMatrix).ToVector3();
		f = (SVector4(f + physicsComponent->ShapeLocalOffset, 1.0f) * transformMatrix).ToVector3();
		g = (SVector4(g + physicsComponent->ShapeLocalOffset, 1.0f) * transformMatrix).ToVector3();
		h = (SVector4(h + physicsComponent->ShapeLocalOffset, 1.0f) * transformMatrix).ToVector3();

		U64 renderViewID = 0;
		if (CScene* worldScene = UComponentAlgo::GetContainingScene(entityOwner, GEngine::GetWorld()->GetActiveScenes()); worldScene != scene)
		{
			// TODO.NW: Figure out a solution to this hard coded Prefab scene override
			renderViewID = 90100;
		}

		GDebugDraw::AddLine(a, b, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(b, c, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(c, d, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(d, a, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(a, e, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(b, h, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(d, f, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(c, g, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(e, f, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(f, g, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(g, h, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(h, e, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);

		// TODO.NR: Most of these should only be changed during setup, but if we want a truly responsive editor we can pause
		// during play and unpause, we should probably handle setting the data on physics wrapper entity if we make modifications here.

		return {};
    }

	bool SPhysics3DComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SPhysics3DComponent>(entity);
		scene->AddComponentEditorContext(entity, &SPhysics3DComponentEditorContext::Context);
		return true;
	}

	bool SPhysics3DComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SPhysics3DComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SPhysics3DComponentEditorContext::Context);
		return true;
	}
}
