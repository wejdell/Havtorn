// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Physics3DControllerComponentView.h"

#include "ECS/Components/Physics3DControllerComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/ComponentAlgo.h"
#include "Scene/Scene.h"

#include "Graphics/Debug/DebugDrawUtility.h"

#include <GUI.h>

namespace Havtorn
{
    SComponentViewResult SPhysics3DControllerComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		SPhysics3DControllerComponent* physicsComponent = scene->GetComponent<SPhysics3DControllerComponent>(entityOwner);

		GUI::ComboEnum("Physics Controller Type", physicsComponent->ControllerType);

		const STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);

		U64 renderViewID = 0;
		if (CScene* worldScene = UComponentAlgo::GetContainingScene(entityOwner, GEngine::GetWorld()->GetActiveScenes()); worldScene != scene)
		{
			// TODO.NW: Figure out a solution to this hard coded Prefab scene override
			renderViewID = 90100;
		}

		switch (physicsComponent->ControllerType)
    	{
		case EPhysics3DControllerType::Box:
		{
			GUI::DragFloat3("Shape Local Extents", physicsComponent->ShapeLocalExtents, GUI::SliderSpeed);
		
			if (SComponent::IsValid(transform))
			{
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

				a = (SVector4(a, 1.0f) * transformMatrix).ToVector3();
				b = (SVector4(b, 1.0f) * transformMatrix).ToVector3();
				c = (SVector4(c, 1.0f) * transformMatrix).ToVector3();
				d = (SVector4(d, 1.0f) * transformMatrix).ToVector3();
				e = (SVector4(e, 1.0f) * transformMatrix).ToVector3();
				f = (SVector4(f, 1.0f) * transformMatrix).ToVector3();
				g = (SVector4(g, 1.0f) * transformMatrix).ToVector3();
				h = (SVector4(h, 1.0f) * transformMatrix).ToVector3();

				GDebugDraw::AddLine(a, b, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(b, c, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(c, d, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(d, a, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(a, e, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(b, h, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(d, f, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(c, g, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(e, f, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(f, g, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(g, h, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(h, e, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
			}
		}
			break;
		case EPhysics3DControllerType::Capsule:
		{
			GUI::DragFloat2("Shape Local Radius And Height", physicsComponent->ShapeLocalRadiusAndHeight, GUI::SliderSpeed);

			if (SComponent::IsValid(transform))
			{
				const SMatrix transformMatrix = transform->Transform.GetMatrix();
				const SVector facing = transformMatrix.GetEuler();
				const SVector center = transformMatrix.GetTranslation();

				const SVector heightOffset = SVector(0.0f, physicsComponent->ShapeLocalRadiusAndHeight.Y * 0.5f, 0.0f);
				GDebugDraw::AddSphere(center + heightOffset, facing, SVector(physicsComponent->ShapeLocalRadiusAndHeight.X), SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddSphere(center - heightOffset, facing, SVector(physicsComponent->ShapeLocalRadiusAndHeight.X), SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddArrow(center, center + transformMatrix.GetForward(), SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
				GDebugDraw::AddLine(center + heightOffset, center - heightOffset, SColor::Green, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
			}
		}
			break;
		}

		GUI::DragFloat3("Displacement:", physicsComponent->Displacement);
		GUI::Text("Velocity: %s", physicsComponent->Velocity.ToString().c_str());


		// TODO.NR: Most of these should only be changed during setup, but if we want a truly responsive editor we can pause
		// during play and unpause, we should probably handle setting the data on physics wrapper entity if we make modifications here.

		return {};
    }
}
