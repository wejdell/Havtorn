// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "StaticMeshComponentView.h"

#include "ECS/Components/StaticMeshComponent.h"
#include "Scene/Scene.h"

#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult SStaticMeshComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		if (!ImGui::UUtils::TryOpenComponentView("StaticMesh"))
			return SComponentViewResult();

		SStaticMeshComponent* staticMesh = scene->GetComponent<SStaticMeshComponent>(entityOwner);
		ImGui::TextDisabled("Number Of Materials: %i", staticMesh->NumberOfMaterials);

		return { EComponentViewResultLabel::InspectAssetComponent, staticMesh, 0 };
    }
}
