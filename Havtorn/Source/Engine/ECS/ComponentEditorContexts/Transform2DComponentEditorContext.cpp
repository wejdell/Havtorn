// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Transform2DComponentEditorContext.h"

#include "ECS/Components/Transform2DComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>


namespace Havtorn
{
	STransform2DComponentEditorContext STransform2DComponentEditorContext::Context = {};

    SComponentViewResult STransform2DComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
		// TODO.NR: Make editable with gizmo
		STransform2DComponent* transform2DComp = scene->GetComponent<STransform2DComponent>(entityOwner);

		GUI::DragFloat2("Position", transform2DComp->Position, GUI::SliderSpeed);
		GUI::DragFloat("DegreesRoll", transform2DComp->DegreesRoll, GUI::SliderSpeed);
		GUI::DragFloat2("Scale", transform2DComp->Scale, GUI::SliderSpeed);

        return SComponentViewResult();
    }

	bool STransform2DComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<STransform2DComponent>(entity);
		scene->AddComponentEditorContext(entity, &STransform2DComponentEditorContext::Context);
		return true;
	}

	bool STransform2DComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<STransform2DComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &STransform2DComponentEditorContext::Context);
		return true;
	}
}
