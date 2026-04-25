// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "StaticMeshComponentEditorContext.h"

#include "Engine.h"
#include "Assets/AssetRegistry.h"
#include "ECS/ComponentAlgo.h"
#include "ECS/Components/StaticMeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/ComponentEditorContexts/MaterialComponentEditorContext.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Assets/AssetRegistry.h"
#include "Assets/AssetReference.h"

#include "Graphics/Debug/DebugDrawUtility.h"

#include <GUI.h>

namespace Havtorn
{
	SStaticMeshComponentEditorContext SStaticMeshComponentEditorContext::Context = {};

    SComponentViewResult SStaticMeshComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transform))
			return SComponentViewResult();

		SStaticMeshComponent* staticMesh = scene->GetComponent<SStaticMeshComponent>(entityOwner);
		const SStaticMeshAsset* staticMeshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SStaticMeshAsset>(staticMesh->AssetReference, entityOwner.GUID);
		if (staticMeshAsset == nullptr)
			return { EComponentViewResultLabel::InspectAssetComponent, staticMesh, SAssetReference::ConvertToPointers(staticMesh->AssetReference), EAssetType::StaticMesh };

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

		return { EComponentViewResultLabel::InspectAssetComponent, staticMesh, SAssetReference::ConvertToPointers(staticMesh->AssetReference), EAssetType::StaticMesh };
    }

	bool SStaticMeshComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SStaticMeshComponent>(entity);
		scene->AddComponentEditorContext(entity, &SStaticMeshComponentEditorContext::Context);

		// TODO.NW: Deal with component dependencies systemically? Or let it be more strictly modular without real dependencies?
		SMaterialComponent* materialComponent = scene->GetComponent<SMaterialComponent>(entity);
		if (materialComponent == nullptr)
		{
			scene->AddComponent<SMaterialComponent>(entity);
			scene->AddComponentEditorContext(entity, &SMaterialComponentEditorContext::Context);
		}

		return true;
	}

	bool SStaticMeshComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SStaticMeshComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SStaticMeshComponentEditorContext::Context);
		return true;
	}

	U8 SStaticMeshComponentEditorContext::GetSortingPriority() const
	{
		return 2;
	}
}
