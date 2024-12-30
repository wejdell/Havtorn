// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Transform2DComponentEditorContext.h"

#include "ECS/Components/Transform2DComponent.h"
#include "Scene/Scene.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
	STransform2DComponentEditorContext STransform2DComponentEditorContext::Context = {};

    SComponentViewResult STransform2DComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		if (!ImGui::UUtils::TryOpenComponentView("Transform2D"))
			return SComponentViewResult();

		// TODO.NR: Make editable with gizmo
		STransform2DComponent* transform2DComp = scene->GetComponent<STransform2DComponent>(entityOwner);

		F32 position[2] = { transform2DComp->Position.X, transform2DComp->Position.Y };
		F32 scale[2] = { transform2DComp->Scale.X, transform2DComp->Scale.Y };

		ImGui::DragFloat2("Position", position, ImGui::UUtils::SliderSpeed);
		ImGui::DragFloat("DegreesRoll", &transform2DComp->DegreesRoll, ImGui::UUtils::SliderSpeed);
		ImGui::DragFloat2("Scale", scale, ImGui::UUtils::SliderSpeed);

		transform2DComp->Position = { position[0], position[1] };
		transform2DComp->Scale = { scale[0], scale[1] };

        return SComponentViewResult();
    }

	bool STransform2DComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("Transform 2D Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<STransform2DComponent>(entity);
		scene->AddComponentEditorContext(entity, &STransform2DComponentEditorContext::Context);
		return true;
	}

	bool STransform2DComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!ImGui::Button("X##16"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<STransform2DComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &STransform2DComponentEditorContext::Context);
		return true;
	}
}
