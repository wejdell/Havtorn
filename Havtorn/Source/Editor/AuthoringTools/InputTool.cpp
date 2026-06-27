// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "InputTool.h"
#include "EditorManager.h"

#include <Assets/AssetRegistry.h>
#include <Input/InputMapper.h>

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

			GEngine::GetAssetRegistry()->SaveAsset(UGeneralUtils::ExtractParentDirectoryFromPath(AssetReference.FilePath) + "/", fileHeader);
		}

		U64 inputActionId = 0;
		for (auto& inputAction : InputAsset->InputActions)
		{
			GUI::PushID(inputActionId++);

			SGameplayTagContainer container = inputAction.Tag;
			GUI::TagPickerDropdown("Tag", "Which Tag you want to map to this Action", container);

			if (container.Tags.empty())
				inputAction.Tag = SGameplayTag::None;
			else
				inputAction.Tag = container.Tags.back();

			GUI::TextDisabled("Input Mapping");

			if (GUI::Button("Add Axis"))
			{
				SInputMapping newMapping = { .ActivationType = EInputActivationType::Continuous, .Data = SAxis{ .Axis = EInputAxis::GamepadLeftStickHorizontal,.AxisPositiveKey = EInputButton::KeyD, .AxisNegativeKey = EInputButton::KeyA } };
				inputAction.InputMappings.push_back(newMapping);
			}

			GUI::SameLine();
			if (GUI::Button("Add Key"))
			{
				SInputMapping newMapping = { .ActivationType = EInputActivationType::KeyDown, .Data = SKey{.Key = EInputButton::Space } };
				inputAction.InputMappings.push_back(newMapping);
			}

			GUI::SameLine();
			if (GUI::Button("Add 2D Axis"))
			{
				SInputMapping newMapping = { .ActivationType = EInputActivationType::Continuous, .Data = S2DAxis{ 
					.HorizontalAxis = SAxis{.Axis = EInputAxis::GamepadLeftStickHorizontal },
					.VerticalAxis = SAxis{.Axis = EInputAxis::GamepadLeftStickVertical }
				} };
				inputAction.InputMappings.push_back(newMapping);
			}

			GUI::SameLine();
			if (GUI::Button("Clear"))
				inputAction.InputMappings.clear();

			U64 mapId = 0;
			for (auto& mapping : inputAction.InputMappings)
			{
				GUI::PushID(mapId++);

				GUI::Separator();
				const U32 typeIndex = STATIC_U32(mapping.Data.index());
				switch (typeIndex)
				{
				case 0:
				{
					GUI::TextDisabled("Axis Mapping");

					SAxis& axis = std::get<SAxis>(mapping.Data);
					DrawAxisGUI("Axis", axis, mapping.ActivationType);
				}
				break;
				case 1:
				{
					GUI::TextDisabled("Key Mapping");

					EInputButton* key = &std::get<SKey>(mapping.Data).Key;

					if (!DrawAssignButtonKeyElement("Key", key))
					{
						GUI::SameLine();
						GUI::ComboEnum("Activation Type", mapping.ActivationType, {}, { EComboFlag::WidthFitPreview });
					}
				}
				break;
				case 2:
				{
					GUI::TextDisabled("2D Axis Mapping");

					S2DAxis& data = std::get<S2DAxis>(mapping.Data);
					GUI::PushID("Horizontal");
					DrawAxisGUI("Horizontal Axis", data.HorizontalAxis, mapping.ActivationType);
					GUI::PopID();
					GUI::PushID("Vertical");
					DrawAxisGUI("Vertical Axis", data.VerticalAxis, mapping.ActivationType);
					GUI::PopID();
				}
				break;
				}

				GUI::PopID();
			}

			constexpr F32 actionSeparation = 24.0f;
			GUI::Dummy({0.0f, actionSeparation});
			GUI::PopID();
		}

		GUI::Separator();
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

	void CInputTool::DrawAxisGUI(const char* label, SAxis& axisValue, EInputActivationType& mappingActivationType)
	{
		GUI::ComboEnum(label, axisValue.Axis, { EInputAxis::Count, EInputAxis::GamepadInvalid, EInputAxis::GamepadRegionStart }, { EComboFlag::WidthFitPreview });
		GUI::SameLine();
		GUI::ComboEnum("Activation Type", mappingActivationType, { EInputActivationType::KeyDown, EInputActivationType::KeyUp }, { EComboFlag::WidthFitPreview });
		
		if (axisValue.Axis != EInputAxis::Key)
			return;
		
		GUI::Indent(0.0f);

		EInputButton* key = &axisValue.AxisPositiveKey;
		DrawAssignButtonKeyElement("Axis Button Positive", key);

		key = &axisValue.AxisNegativeKey;
		DrawAssignButtonKeyElement("Axis Button Negative", key);

		GUI::Unindent(0.0f);
		
	}

	bool CInputTool::DrawAssignButtonKeyElement(const char* label, EInputButton* key)
	{
		if (CurrentButtonBeingAssigned == key)
		{
			GUI::TextDisabled("Waiting For Button Press...");
			return true;
		}

		GUI::PushID(label);
		if (GUI::Button("Assign Input"))
		{
			CurrentButtonBeingAssigned = key;
			EInputButton** currentTrackedProperty = &CurrentButtonBeingAssigned;
			GEngine::GetInput()->StartListenForButtonInput([key, currentTrackedProperty](const EInputButton button)
				{
					if (key != nullptr)
						*key = button;

					if (*currentTrackedProperty != nullptr)
						*currentTrackedProperty = nullptr;
				}
			);
		}
		GUI::PopID();
		GUI::SameLine();
		GUI::ComboEnum(label, *key, { EInputButton::None, EInputButton::GamepadRegionStart, EInputButton::GamepadInvalid }, { EComboFlag::WidthFitPreview });
		
		return false;
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
