// Copyright 2025 Team Havtorn. All Rights Reserved.
#include "hvpch.h"

#include "AbilityStateMachineComponentEditorContext.h"
#include "AbilityStateMachineComponent.h"
#include "AbilitySM/AbilityStateMachineSystem.h"

#include <Scene/Scene.h>

#include <GUI.h>

namespace Havtorn
{
	SAbilityStateMachineComponentEditorContext SAbilityStateMachineComponentEditorContext::Context = {};

	SComponentViewResult SAbilityStateMachineComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		entityOwner;
		scene;

		if (!GUI::TryOpenComponentView("Ability State Machine"))
			return SComponentViewResult();

		SAbilityStateMachineComponent* component = scene->GetComponent<SAbilityStateMachineComponent>(entityOwner);

		if (GUI::Button("Open Editor"))
			Context.IsOpen = !Context.IsOpen;

		if (Context.IsOpen)
			Context.CustomEditorWindow(component, scene);


		GUI::Checkbox("IsFalling", component->Data.IsFalling);
		GUI::Checkbox("IsJumping", component->Data.IsJumping);




		for (auto& ability : component->Abilites)
		{
			bool isActive = IsActive(*ability, component);
			std::string abilityLabel = component->AbilityNameMap[ability->Guid].Data();
			abilityLabel.append(isActive ? ": Active" : ": InActive");
			GUI::Text(abilityLabel.c_str());
		}

		if (component->AbilityBlockingMap.size() > 0)
		{
			GUI::Text("Blocked Tags");
			for (auto& [key, value] : component->AbilityBlockingMap)
			{
				for (auto& blockingTag : value)
				{
					GUI::PushID(STATIC_I32(blockingTag));
					GUI::ComboEnum<EAbilityTag>("", blockingTag);
					GUI::PopID();
				}
			}
		}

		if (component->Abilites.size() == 0 && GUI::Button("Add Abilities"))
		{

			component->Abilites.emplace_back(new Locomotion());
			component->AbilityNameMap[component->Abilites.back()->Guid] = CHavtornStaticString<255>("Locomotion");

			component->Abilites.emplace_back(new CameraControl());
			component->AbilityNameMap[component->Abilites.back()->Guid] = CHavtornStaticString<255>("CameraControl");

			component->Abilites.emplace_back(new Jump());
			component->AbilityNameMap[component->Abilites.back()->Guid] = CHavtornStaticString<255>("Jump");

			component->Abilites.emplace_back(new Falling());
			component->AbilityNameMap[component->Abilites.back()->Guid] = CHavtornStaticString<255>("Falling");

		}
		return SComponentViewResult();
	}

	bool SAbilityStateMachineComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Ability StateMachine Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SAbilityStateMachineComponent>(entity);
		scene->AddComponentEditorContext(entity, &SAbilityStateMachineComponentEditorContext::Context);
		return true;
	}
	bool SAbilityStateMachineComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SAbilityStateMachineComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SAbilityStateMachineComponentEditorContext::Context);
		return true;
	}

	bool SAbilityStateMachineComponentEditorContext::IsActive(const SAbility& ability, SAbilityStateMachineComponent* component) const
	{
		for (const SAbility* activeAbility : component->ActiveAbilities)
			if (ability.Guid == activeAbility->Guid)
				return true;

		return false;
	}

	bool SAbilityStateMachineComponentEditorContext::CustomEditorWindow(SAbilityStateMachineComponent* component, CScene* scene)
	{

		GUI::Begin("Ability State Machine", &Context.IsOpen);

		SMetaDataComponent* metaData = scene->GetComponent<SMetaDataComponent>(component);

		GUI::Text("Hello Mr. %s", metaData->Name.Data());

		GUI::End();
		return true;
	}

}