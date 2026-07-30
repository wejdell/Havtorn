// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "StaticMeshComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"
#include "MaterialComponentView.h"

#include <ECS/Components/StaticMeshComponent.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/MaterialComponent.h>
#include <ECS/ComponentAlgo.h>
#include <Assets/AssetReference.h>
#include <Assets/AssetRegistry.h>
#include <Scene/Scene.h>
#include <Engine.h>

#include <Graphics/Debug/DebugDrawUtility.h>

#include <GUI.h>

namespace Havtorn
{
    void SStaticMeshComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transform))
			return;

		SStaticMeshComponent* staticMesh = scene->GetComponent<SStaticMeshComponent>(entityOwner);
		const SStaticMeshAsset* staticMeshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SStaticMeshAsset>(staticMesh->AssetReference, entityOwner.GUID);
		if (staticMeshAsset == nullptr)
		{
			CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
			inspector->InspectAssetComponent(staticMesh, EAssetType::StaticMesh, SAssetReference::ConvertToPointers(staticMesh->AssetReference));
			return;
		}

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

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->InspectAssetComponent(staticMesh, EAssetType::StaticMesh, SAssetReference::ConvertToPointers(staticMesh->AssetReference));
    }

	U8 SStaticMeshComponentView::GetSortingPriority() const
	{
		return 2;
	}
}
