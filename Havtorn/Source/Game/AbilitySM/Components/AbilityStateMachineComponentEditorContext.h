// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include <ECS/ComponentEditorContext.h>


namespace Havtorn
{
	struct SAbility;
	struct SAbilityStateMachineComponent;

	struct SAbilityStateMachineComponentEditorContext : public SComponentEditorContext
	{
		SComponentViewResult View(const SEntity& entityOwner, CScene* scene) const override;
		bool AddComponent(const SEntity& entity, CScene* scene) const override;
		bool RemoveComponent(const SEntity& entity, CScene* scene) const override;

		bool IsActive(const SAbility& ability, SAbilityStateMachineComponent* component) const;
		static SAbilityStateMachineComponentEditorContext Context;

	private:
		bool CustomEditorWindow(SAbilityStateMachineComponent* component, CScene* scene);
		bool IsOpen = false;

	};


}