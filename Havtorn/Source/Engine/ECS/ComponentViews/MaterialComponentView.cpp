// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponentView.h"
#include "ECS/Components/MaterialComponent.h"
#include "Scene/Scene.h"

#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult Havtorn::SMaterialComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		if (!ImGui::UUtils::TryOpenComponentView("Material"))
			return SComponentViewResult();

		SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(entityOwner);

		return { EComponentViewResultLabel::InspectAssetComponent, materialComp, 0 };
    }
}
