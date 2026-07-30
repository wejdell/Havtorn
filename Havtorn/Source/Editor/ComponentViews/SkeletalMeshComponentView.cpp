// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SkeletalMeshComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/SkeletalMeshComponent.h>
#include <ECS/Components/TransformComponent.h>
#include <Assets/AssetReference.h>
#include <Assets/AssetRegistry.h>
#include <Scene/Scene.h>
#include <Engine.h>

#include <Graphics/Debug/DebugDrawUtility.h>

#include <GUI.h>

namespace Havtorn
{
    void SSkeletalMeshComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transform))
			return;

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

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->InspectAssetComponent(skeletalMesh, EAssetType::SkeletalMesh, SAssetReference::ConvertToPointers(skeletalMesh->AssetReference));
    }

	U8 SSkeletalMeshComponentView::GetSortingPriority() const
	{
		return 2;
	}
}
