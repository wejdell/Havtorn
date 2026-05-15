// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "InputTool.h"
#include "EditorManager.h"
#include <Assets/AssetRegistry.h>

namespace Havtorn
{
	CInputTool::CInputTool(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager, false)
	{
	}

	void CInputTool::OnEnable()
	{
	}

	void CInputTool::OnInspectorGUI()
	{
		if (!GUI::Begin(Name(), &IsEnabled))
		{
			GUI::End();
			return;
		}

		GUI::Text(UGeneralUtils::ExtractFileBaseNameFromPath(AssetReference.FilePath).c_str());
		GUI::SameLine();
		if (GUI::Button("Save"))
		{
			SInputAssetFileHeader fileHeader;
			fileHeader.Name = AssetName;
			fileHeader.InputActions = InputAsset->InputActions;
			fileHeader.AssetType = InputAsset->AssetType;

			GEngine::GetAssetRegistry()->SaveAsset(UGeneralUtils::ExtractParentDirectoryFromPath(AssetReference.FilePath) + "/", fileHeader);
		}

		U64 inputActionId = 0;
		for (auto& inputAction : InputAsset->InputActions)
		{
			GUI::PushID(inputActionId++);

			if (GUI::Button("Add Axis"))
			{
				SInputMapping newMapping = { .Data = SAxis{.Axis = EInputAxis::GamepadLeftStickHorizontal,.AxisPositiveKey = EInputKey::KeyD, .AxisNegativeKey = EInputKey::KeyA } };
				inputAction.InputMappings.push_back(newMapping);
			}

			GUI::SameLine();
			if (GUI::Button("Add Key"))
			{
				SInputMapping newMapping = { .Data = SKey{.Key = EInputKey::Space } };
				inputAction.InputMappings.push_back(newMapping);
			}

			U64 mapId = 0;
			for (auto& mapping : inputAction.InputMappings)
			{
				GUI::PushID(mapId++);

				const U32 typeIndex = STATIC_U32(mapping.Data.index());
				switch (typeIndex)
				{
				case 0:
				{
					SAxis& axis = std::get<SAxis>(mapping.Data);
					GUI::ComboEnum("Axis", axis.Axis, { EInputAxis::Count, EInputAxis::GamepadRegionStart });
					GUI::ComboEnum("Axis Button Negative", axis.AxisNegativeKey, { EInputKey::None, EInputKey::GamepadRegionStart });
					GUI::ComboEnum("Axis Button Positive", axis.AxisPositiveKey, { EInputKey::None, EInputKey::GamepadRegionStart });
				}
				break;
				case 1:
				{
					SKey& key = std::get<SKey>(mapping.Data);
					GUI::ComboEnum("Axis", key.Key, { EInputKey::None, EInputKey::GamepadRegionStart });
				}
				break;
				}

				GUI::PopID();
			}

			SGameplayTagContainer container = inputAction.Tag;
			GUI::TagPickerDropdown("Tag", "Which Tag you want to map to this Action", container);

			if (container.Tags.empty())
				inputAction.Tag = SGameplayTag::None;
			else
				inputAction.Tag = container.Tags.back();

			GUI::PopID();
		}

		if (GUI::Button("New Input Action"))
		{
			InputAsset->InputActions.push_back(SInputMapAction{ .Tag = SGameplayTag::None, .InputMappings = {} });
		}

		GUI::End();
	}

	void CInputTool::DrawInputTable()
	{
		EGUITableFlags tableFlags = EGUITableFlags::Resizable | EGUITableFlags::Borders;
		if (GUI::BeginTable("InputColumns", 2, STATIC_I32(tableFlags)))
		{
			GUI::TableNextColumn();
			GUI::Selectable("Actions");
			const I32 actioncount = 5;
			std::vector<const char*> names = { "Jump", "Crouch", "Move", "Pause", "Interact" };
			static I32 selectedIndex = -1;

			for (U32 i = 0; i < actioncount; i++)
			{
				if (GUI::TreeNode(names[i]))
				{
					selectedIndex = i;
					GUI::SameLine();
					if (GUI::Button("Add"))
					{

					}
					GUI::Text("<No Binding>");
					GUI::TreePop();
				}
			}

			GUI::TableNextColumn();
			GUI::Selectable("Properties");

			GUI::EndTable();
		}
	}

	void CInputTool::OnDisable()
	{
	}

	void CInputTool::OpenInputAsset(SEditorAssetRepresentation* asset)
	{
		AssetName = asset->Name;
		AssetReference = SAssetReference(asset->DirectoryEntry.path().string());
		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		InputAsset = assetRegistry->RequestAssetData<SInputAsset>(AssetReference, InputToolID);
		SetEnabled(true);
	}
}
