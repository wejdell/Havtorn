// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SkeletalMeshComponentEditorContext.h"

#include "ECS/Components/SkeletalMeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Scene/Scene.h"

#include "Graphics/Debug/DebugDrawUtility.h"

#include <GUI.h>
#include <Scene/AssetRegistry.h>

namespace Havtorn
{
	SSkeletalMeshComponentEditorContext SSkeletalMeshComponentEditorContext::Context = {};

    SComponentViewResult SSkeletalMeshComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!GUI::TryOpenComponentView("SkeletalMesh"))
			return SComponentViewResult();

		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transform))
			return SComponentViewResult();

		SSkeletalMeshComponent* skeletalMesh = scene->GetComponent<SSkeletalMeshComponent>(entityOwner);
		const SSkeletalMeshAsset* skeletalMeshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalMeshAsset>(skeletalMesh->AssetReference, entityOwner.GUID);
		GUI::TextDisabled("Number Of Materials: %i", skeletalMeshAsset->NumberOfMaterials);

		SVector a = SVector(skeletalMeshAsset->BoundsMin.X, skeletalMeshAsset->BoundsMin.Y, skeletalMeshAsset->BoundsMin.Z);
		SVector b = SVector(skeletalMeshAsset->BoundsMin.X, skeletalMeshAsset->BoundsMin.Y, skeletalMeshAsset->BoundsMax.Z);
		SVector c = SVector(skeletalMeshAsset->BoundsMax.X, skeletalMeshAsset->BoundsMin.Y, skeletalMeshAsset->BoundsMax.Z);
		SVector d = SVector(skeletalMeshAsset->BoundsMax.X, skeletalMeshAsset->BoundsMin.Y, skeletalMeshAsset->BoundsMin.Z);
		SVector e = SVector(skeletalMeshAsset->BoundsMin.X, skeletalMeshAsset->BoundsMax.Y, skeletalMeshAsset->BoundsMin.Z);
		SVector f = SVector(skeletalMeshAsset->BoundsMax.X, skeletalMeshAsset->BoundsMax.Y, skeletalMeshAsset->BoundsMin.Z);
		SVector g = SVector(skeletalMeshAsset->BoundsMax.X, skeletalMeshAsset->BoundsMax.Y, skeletalMeshAsset->BoundsMax.Z);
		SVector h = SVector(skeletalMeshAsset->BoundsMin.X, skeletalMeshAsset->BoundsMax.Y, skeletalMeshAsset->BoundsMax.Z);

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

		return { EComponentViewResultLabel::InspectAssetComponent, skeletalMesh, &skeletalMesh->AssetReference, nullptr, EAssetType::SkeletalMesh };
    }

	bool SSkeletalMeshComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Skeletal Mesh Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SSkeletalMeshComponent>(entity);
		scene->AddComponentEditorContext(entity, &SSkeletalMeshComponentEditorContext::Context);
		return true;
	}

	bool SSkeletalMeshComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##12"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SSkeletalMeshComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SSkeletalMeshComponentEditorContext::Context);
		return true;
	}

	U8 SSkeletalMeshComponentEditorContext::GetSortingPriority() const
	{
		return 2;
	}
}
