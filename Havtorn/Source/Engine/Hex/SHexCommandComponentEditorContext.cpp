// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SHexCommandComponentEditorContext.h"
#include "SHexCommandComponent.h"

#include "Scene/Scene.h"

#include <GUI.h>


namespace Havtorn
{
	SHexCommandComponentEditorContext SHexCommandComponentEditorContext::Context = { };

	SComponentViewResult SHexCommandComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("HexCommand Component"))
			return SComponentViewResult();

		SHexCommandComponent* component = scene->GetComponent<SHexCommandComponent>(entityOwner);
		GUI::TagPickerDropdown("Hex Command Tag", "Tag identifying this HexCommand", component->TagsToListenFor);


		/*if (GUI::Button("New Hex Command"))
		{
			component->HexCommands.push_back(SHexCommand{});
		}

		for (U32 i = 0; i < STATIC_U32(component->HexCommands.size()); i++)
		{
			GUI::PushID(STATIC_U64(i));
			{
				SHexCommand& command = component->HexCommands[i];
				SGameplayTagContainer tagContainer = component->HexCommands[i].Tag;
				GUI::TagPickerDropdown("Hex Command Tag", "Tag identifying this HexCommand", tagContainer);

				if (!tagContainer.Tags.empty())
					command.Tag = tagContainer.Tags.back();
				else 
					command.Tag = SGameplayTag::None;
			}
			GUI::PopID();
		}*/

		return SComponentViewResult();
	}

	bool SHexCommandComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("HexCommand Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SHexCommandComponent>(entity);
		scene->AddComponentEditorContext(entity, &SHexCommandComponentEditorContext::Context);
		return true;
	}

	bool SHexCommandComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##10"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SHexCommandComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SHexCommandComponentEditorContext::Context);
		return true;
	}

}