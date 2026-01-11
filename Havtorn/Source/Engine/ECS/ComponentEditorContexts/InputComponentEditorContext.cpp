// Copyright 2025 Team Havtorn. All Rights Reserved.
#include "InputComponentEditorContext.h"
#include "../Components/InputComponent.h"

#include <GUI.h>
#include <Scene/Scene.h>

namespace Havtorn
{
    SInputComponentEditorContext SInputComponentEditorContext::Context = { };
    SComponentViewResult SInputComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
    {
        if (!GUI::TryOpenComponentView("Input Component"))
            return SComponentViewResult();

        SInputComponent* component = scene->GetComponent<SInputComponent>(entityOwner);
        GUI::Checkbox("IsActive", component->IsActive);
        GUI::DragFloat3("Move Input", component->MoveInput);

        return SComponentViewResult();
    }
    bool SInputComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
    {
        if (!GUI::Button("Input Component"))
            return false;

        if (scene == nullptr || !entity.IsValid())
            return false;

        scene->AddComponent<SInputComponent>(entity);
        scene->AddComponentEditorContext(entity, &SInputComponentEditorContext::Context);
        return true;
    }
    bool SInputComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
    {
        if (!GUI::Button("X"))
            return false;

        if (scene == nullptr || !entity.IsValid())
            return false;

        scene->RemoveComponent<SInputComponent>(entity);
        scene->RemoveComponentEditorContext(entity, &SInputComponentEditorContext::Context);
        return true;
    }
}