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
		SHexCommandComponent* component = scene->GetComponent<SHexCommandComponent>(entityOwner);
		GUI::TagPickerDropdown("Hex Command Tag", "Tag identifying this HexCommand", component->TagsToListenFor);

		std::stack<SHexCommand> hexCommandCopy = component->HexCommands;
		U64 id = 0;
		while (!hexCommandCopy.empty())
		{
			GUI::PushID(id++);
			SHexCommand& top = hexCommandCopy.top();
			const std::string& tagName = top.Tag.Name;
			switch (top.DataType)
			{
			case EHexCommandDataType::Bool:
				GUI::Checkbox(tagName.c_str(), std::get<bool>(top.Data));
				break;
			case EHexCommandDataType::Float:
				GUI::DragFloat(tagName.c_str(), std::get<F32>(top.Data));
				break;
			case EHexCommandDataType::Vector2:
				GUI::DragFloat2(tagName.c_str(), std::get<SVector2<F32>>(top.Data));
				break;
			}

			hexCommandCopy.pop();
			GUI::PopID();
		}


		return SComponentViewResult();
	}

	bool SHexCommandComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		scene->AddComponent<SHexCommandComponent>(entity);
		scene->AddComponentEditorContext(entity, &SHexCommandComponentEditorContext::Context);
		return true;
	}

	bool SHexCommandComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		scene->RemoveComponent<SHexCommandComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SHexCommandComponentEditorContext::Context);
		return true;
	}

}