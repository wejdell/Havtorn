// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "StaticMeshComponentView.h"

#include "ECS/Components/StaticMeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

#include "Graphics/Debug/DebugDrawUtility.h"

#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult SStaticMeshComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		if (!ImGui::UUtils::TryOpenComponentView("StaticMesh"))
			return SComponentViewResult();

		SStaticMeshComponent* staticMesh = scene->GetComponent<SStaticMeshComponent>(entityOwner);
		ImGui::TextDisabled("Number Of Materials: %i", staticMesh->NumberOfMaterials);

		SVector a = SVector(staticMesh->BoundsMin.X, staticMesh->BoundsMin.Y, staticMesh->BoundsMin.Z);
		SVector b = SVector(staticMesh->BoundsMin.X, staticMesh->BoundsMin.Y, staticMesh->BoundsMax.Z);
		SVector c = SVector(staticMesh->BoundsMax.X, staticMesh->BoundsMin.Y, staticMesh->BoundsMax.Z);
		SVector d = SVector(staticMesh->BoundsMax.X, staticMesh->BoundsMin.Y, staticMesh->BoundsMin.Z);
		SVector e = SVector(staticMesh->BoundsMin.X, staticMesh->BoundsMax.Y, staticMesh->BoundsMin.Z);
		SVector f = SVector(staticMesh->BoundsMax.X, staticMesh->BoundsMax.Y, staticMesh->BoundsMin.Z);
		SVector g = SVector(staticMesh->BoundsMax.X, staticMesh->BoundsMax.Y, staticMesh->BoundsMax.Z);
		SVector h = SVector(staticMesh->BoundsMin.X, staticMesh->BoundsMax.Y, staticMesh->BoundsMax.Z);

		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);
		SMatrix transformMatrix = transform->Transform.GetMatrix();

		a = (SVector4(a, 1.0f) * transformMatrix).ToVector3();
		b = (SVector4(b, 1.0f) * transformMatrix).ToVector3();
		c = (SVector4(c, 1.0f) * transformMatrix).ToVector3();
		d = (SVector4(d, 1.0f) * transformMatrix).ToVector3();
		e = (SVector4(e, 1.0f) * transformMatrix).ToVector3();
		f = (SVector4(f, 1.0f) * transformMatrix).ToVector3();
		g = (SVector4(g, 1.0f) * transformMatrix).ToVector3();
		h = (SVector4(h, 1.0f) * transformMatrix).ToVector3();

		GDebugDraw::AddLine(a, b, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(b, c, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(c, d, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(d, a, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(a, e, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(b, h, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(d, f, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(c, g, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(e, f, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(f, g, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(g, h, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);
		GDebugDraw::AddLine(h, e, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false);

		return { EComponentViewResultLabel::InspectAssetComponent, staticMesh, 0 };
    }
}
