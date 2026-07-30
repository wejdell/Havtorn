// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Transform2DComponentView.h"

#include <ECS/Components/Transform2DComponent.h>
#include <Scene/Scene.h>

#include <GUI.h>

namespace Havtorn
{
    void STransform2DComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		// TODO.NR: Make editable with gizmo
		STransform2DComponent* transform2DComp = scene->GetComponent<STransform2DComponent>(entityOwner);

		GUI::DragFloat2("Position", transform2DComp->Position, GUI::SliderSpeed);
		GUI::DragFloat("DegreesRoll", transform2DComp->DegreesRoll, GUI::SliderSpeed);
		GUI::DragFloat2("Scale", transform2DComp->Scale, GUI::SliderSpeed);
    }
}
