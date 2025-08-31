// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "StaticMeshComponentEditorContext.h"

#include "ECS/Components/StaticMeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

#include "Graphics/Debug/DebugDrawUtility.h"

#include <GUI.h>
#include <Engine.h>
#include <Scene/AssetRegistry.h>

namespace Havtorn
{
	SStaticMeshComponentEditorContext SStaticMeshComponentEditorContext::Context = {};

    SComponentViewResult SStaticMeshComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("StaticMesh"))
			return SComponentViewResult();

		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transform))
			return SComponentViewResult();

		SStaticMeshComponent* staticMesh = scene->GetComponent<SStaticMeshComponent>(entityOwner);
		const SStaticMeshAsset* staticMeshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SStaticMeshAsset>(staticMesh->AssetReference, entityOwner.GUID);
		GUI::TextDisabled("Number Of Materials: %i", staticMeshAsset->NumberOfMaterials);

		SVector a = SVector(staticMeshAsset->BoundsMin.X, staticMeshAsset->BoundsMin.Y, staticMeshAsset->BoundsMin.Z);
		SVector b = SVector(staticMeshAsset->BoundsMin.X, staticMeshAsset->BoundsMin.Y, staticMeshAsset->BoundsMax.Z);
		SVector c = SVector(staticMeshAsset->BoundsMax.X, staticMeshAsset->BoundsMin.Y, staticMeshAsset->BoundsMax.Z);
		SVector d = SVector(staticMeshAsset->BoundsMax.X, staticMeshAsset->BoundsMin.Y, staticMeshAsset->BoundsMin.Z);
		SVector e = SVector(staticMeshAsset->BoundsMin.X, staticMeshAsset->BoundsMax.Y, staticMeshAsset->BoundsMin.Z);
		SVector f = SVector(staticMeshAsset->BoundsMax.X, staticMeshAsset->BoundsMax.Y, staticMeshAsset->BoundsMin.Z);
		SVector g = SVector(staticMeshAsset->BoundsMax.X, staticMeshAsset->BoundsMax.Y, staticMeshAsset->BoundsMax.Z);
		SVector h = SVector(staticMeshAsset->BoundsMin.X, staticMeshAsset->BoundsMax.Y, staticMeshAsset->BoundsMax.Z);

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

		return { EComponentViewResultLabel::InspectAssetComponent, staticMesh, &staticMesh->AssetReference, nullptr, EAssetType::StaticMesh };
    }

	bool SStaticMeshComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Static Mesh Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SStaticMeshComponent>(entity);
		scene->AddComponentEditorContext(entity, &SStaticMeshComponentEditorContext::Context);
		return true;
	}

	bool SStaticMeshComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##16"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SStaticMeshComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SStaticMeshComponentEditorContext::Context);
		return true;
	}

	U8 SStaticMeshComponentEditorContext::GetSortingPriority() const
	{
		return 2;
	}
}
