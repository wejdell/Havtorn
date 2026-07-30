// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AbilityComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/AbilityComponent.h>
#include <ECS/Components/MetaDataComponent.h>
#include <Scene/Scene.h>
#include <Scene/World.h>
#include <Engine.h>
#include <GameplayTags/GameplayTagManager.h>
#include <GameplayTags/GameplayTag.h>

#include <GUI.h>

namespace Havtorn 
{
	void SAbilityComponentView::View(const SEntity& entityOwner, CScene* scene) const
	{
		SAbilityComponent* component = scene->GetComponent<SAbilityComponent>(entityOwner);

		GUI::TextDisabled("Abilities");

		// TODO.NW: Make Array gui element, templated or something
		GUI::SameLine();
		if (GUI::Button("Add Ability"))
			component->Abilities.push_back(SAbilityState());

		GUI::SameLine();
		if (GUI::Button("Clear Abilities"))
			component->Abilities.clear();

		if (component->Abilities.empty())
			return;

		I32 abilityToRemoveIndex = -1;

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();

		for (I32 i = 0; i < STATIC_I32(component->Abilities.size()); i++)
		{
			SAbilityState& ability = component->Abilities[i];
			GUI::PushID(i);
			GUI::Separator();

			if (GUI::Button("X"))
				abilityToRemoveIndex = i;
			GUI::SameLine();

			std::string abilityName = "Ability ";
			abilityName.append(std::to_string(i));
			if (GUI::TreeNode(abilityName.c_str()))
			{
				SGameplayTagContainer singleTagContainer = ability.AbilityTag;
				GUI::TagPickerDropdown("Ability Tag", "Tag identifying this ability", singleTagContainer);
				if (singleTagContainer.Tags.size() > 0)
					ability.AbilityTag = singleTagContainer.Tags.back();
				else if (singleTagContainer.Tags.empty())
					ability.AbilityTag = SGameplayTag::None;

				GUI::TagPickerDropdown("Activation Blocking Tags", "If present, block activation", ability.ActivationBlockingTags);

				GUI::Separator();
				if (GUI::TreeNode("Show More"))
				{
					GUI::TagPickerDropdown("Activation Required Tags", "Required for activation", ability.ActivationRequiredTags);
					GUI::TagPickerDropdown("Activation Granted Tags", "When activated, apply these tags", ability.ActivationGrantedTags);
					GUI::TagPickerDropdown("Activation Cancel Tags", "When activated, cancel abilities with these tags", ability.ActivationCancelTags);
					GUI::TagPickerDropdown("Continuous Required Tags", "If not present, ability will end", ability.ContinuousRequiredTags);
					GUI::TagPickerDropdown("Continuous Blocking Tags", "If present, ability will end", ability.ContinuousBlockingTags);
					GUI::TreePop();
				}
				GUI::Separator();

				inspector->InspectAssetComponent(component, EAssetType::Script, { &ability.ScriptReference });

				GUI::TreePop();
			}
			GUI::PopID();
		}

		if (abilityToRemoveIndex != -1)
			component->Abilities.erase(component->Abilities.begin() + abilityToRemoveIndex);
	}
}
