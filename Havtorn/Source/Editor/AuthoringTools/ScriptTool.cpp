// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"

#include <HexRune/HexRune.h>
#include <ECS/GUIDManager.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Systems/CameraSystem.h>
#include <HexRune/CoreNodes/CoreNodes.h>

#include "NodeViews/CoreNodeViews.h"

#include <FileSystem.h>
#include <magic_enum.h>

#include "ScriptTool.h"

#include <Assets/AssetRegistry.h>

#include <../Game/GameScript.h>

using Havtorn::I32;
using Havtorn::F32;
using Havtorn::U64;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

template<typename T>
concept IsPinLiteralType = std::is_same_v<T, std::monostate> || std::is_same_v<T, bool> || std::is_same_v<T, I32> || std::is_same_v<T, F32> || std::is_same_v<T, std::string> || std::is_same_v<T, Havtorn::SVector> || std::is_same_v<T, Havtorn::SMatrix> || std::is_same_v<T, Havtorn::SQuaternion>;

std::variant<PIN_LITERAL_TYPES, PIN_MATH_TYPES> GetLiteralTypeData(const std::variant<PIN_DATA_TYPES>& engineData)
{
	return std::visit(overloaded
		{
			[] <IsPinLiteralType T>(const T & x) { return std::variant<PIN_LITERAL_TYPES, PIN_MATH_TYPES>{x}; },
			[](auto&) { return std::variant<PIN_LITERAL_TYPES, PIN_MATH_TYPES>{}; }
		}, engineData
	);
}

std::variant<PIN_DATA_TYPES> GetEngineTypeData(const std::variant<PIN_LITERAL_TYPES, PIN_MATH_TYPES>& editorData)
{
	return std::visit(overloaded
		{
			[] <IsPinLiteralType T>(const T & x) { return std::variant<PIN_DATA_TYPES>{x}; }
		}, editorData
	);
}

namespace Havtorn
{
	CScriptTool::CScriptTool(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager, false)
	{
	}

	void CScriptTool::OnEnable()
	{
	}

	void CScriptTool::OnInspectorGUI()
	{
		// TODO.NW: Make ON_SCOPE_EXIT equivalent?

		if (!GUI::Begin(Name(), &IsEnabled))
		{
			GUI::End();
			return;
		}

		IsHovered = IsEnabled && GUI::IsWindowHovered();

		if (CurrentScriptAsset == nullptr || CurrentScript == nullptr)
		{
			GUI::End();
			return;
		}

		const bool isWindowHovered = GUI::IsWindowHovered();
		if (!IsHoveringWindow && isWindowHovered)
			GEngine::GetWorld()->BlockSystem<CCameraSystem>(this);
		else if (IsHoveringWindow && !isWindowHovered)
			GEngine::GetWorld()->UnblockSystem<CCameraSystem>(this);

		IsHoveringWindow = isWindowHovered;

		{ // Menu Bar
			GUI::BeginChild("ScriptMenuBar", SVector2<F32>(0.0f, 30.0f));
			GUI::Text(CurrentScript->Name.c_str());
			GUI::SameLine();
			Filter.Draw("Search", 180);

			GUI::SameLine();
			if (GUI::Button("Save"))
				SaveScript();

			GUI::Separator();
			GUI::EndChild();
		}

		{ // Data Bindings
			GUI::BeginChild("DataBindings", SVector2<F32>(150.0f, 0.0f), { EChildFlag::Borders, EChildFlag::ResizeX });
			GUI::Text("Data Bindings");
			GUI::Separator();

			for (SScriptDataBinding& dataBinding : CurrentScript->DataBindings)
			{
				GUI::Text(dataBinding.Name.c_str());
				if (GUI::IsItemHovered())
				{
					if (dataBinding.Type == EPinType::Asset)
						GUI::SetTooltip("%s | %s", "Asset", magic_enum::enum_name(dataBinding.AssetType).data());
					else
						GUI::SetTooltip("%s", magic_enum::enum_name(dataBinding.Type).data());
				}

				CEditorManager::DataBindingDragData.TrySet(dataBinding, dataBinding.Name.c_str(), { EDragDropFlag::SourceAllowNullID });

				if (GUI::BeginPopupContextWindow())
				{
					if (GUI::MenuItem("Delete"))
						Edit.RemovedBindingID = dataBinding.UID;

					GUI::EndPopup();
				}
			}
			GUI::EndChild();
			GUI::SameLine();
		}

		GUI::BeginScript("Node Script Editor");
		RenderScript();
		CommitEdit();
		GUI::EndScript();

		auto result = CEditorManager::DataBindingDragData.TryDeliver({ EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNoDrawDefaultRect, EDragDropFlag::AcceptNopreviewTooltip });
		if (result.Payload != nullptr)
		{
			SScriptDataBinding* dataBinding = result.Payload;

			std::string tooltip = "Add Get ";
			tooltip.append(dataBinding->Name);
			tooltip.append(" node?");
			GUI::SetTooltip(tooltip.c_str());

			if (result.Result == EDragDeliverResult::Delivered)
			{
				// TODO.NW: Make sure to catch keybinds here
				for (const Ptr<SNodeView>& view : Manager->GetNodeViewsVector())
				{
					// TODO.NW: Maybe remove whitespace from names? Then they need extra care to display properly.
					if (view->Name == "Get " + dataBinding->Name)
					{
						Edit.NewNodeView = view.get();
						Edit.NewNodePosition = GUI::GetMousePosition();
					}
				}
			}
		}

		GUI::End();
	}

	void CScriptTool::OnDisable()
	{
		CloseScript();

		GEngine::GetWorld()->UnblockSystem<CCameraSystem>(this);
	}

	void CScriptTool::OpenScript(SEditorAssetRepresentation* asset)
	{
		// TODO.NW: Add error handling?

		if (IsEnabled)
			CloseScript();

		CurrentScriptAssetRef = SAssetReference(asset->DirectoryEntry.path().string());
		CurrentScriptAsset = GEngine::GetAssetRegistry()->RequestAssetData<SScriptAsset>(CurrentScriptAssetRef, CAssetRegistry::EditorManagerRequestID);
		CurrentScript = CurrentScriptAsset->Script.get();

		for (const SScriptDataBinding& binding : CurrentScriptAsset->Script->DataBindings)
		{
			Manager->RegisterDataBindingNodeView<SDataBindingGetNodeEditorContext, SDataBindingGetNode>(CurrentScriptAsset->Script.get(), binding.UID);
			Manager->RegisterDataBindingNodeView<SDataBindingSetNodeEditorContext, SDataBindingSetNode>(CurrentScriptAsset->Script.get(), binding.UID);
		}

		SetEnabled(true);
	}

	void CScriptTool::SaveScript()
	{
		// TODO.NW: Check redirections?

		SScriptFileHeader fileHeader;
		fileHeader.Name = CurrentScriptAsset->Script->Name;
		fileHeader.Script = CurrentScriptAsset->Script.get();
		fileHeader.NodePositionMap = CurrentScriptAsset->NodePositionMap;
		GEngine::GetAssetRegistry()->SaveAsset(UGeneralUtils::ExtractParentDirectoryFromPath(CurrentScriptAssetRef.FilePath) + "/", fileHeader);
	}

	void CScriptTool::CloseScript()
	{
		// TODO.NW: Ask user if they want to save?

		for (const SScriptDataBinding& binding : CurrentScriptAsset->Script->DataBindings)
		{
			Manager->RemoveDataBindingNodeView<SDataBindingGetNodeEditorContext, SDataBindingGetNode>(binding.UID);
			Manager->RemoveDataBindingNodeView<SDataBindingSetNodeEditorContext, SDataBindingSetNode>(binding.UID);
		}

		GEngine::GetAssetRegistry()->UnrequestAsset(CurrentScriptAssetRef, CAssetRegistry::EditorManagerRequestID);
		CurrentScriptAssetRef = SAssetReference();
		CurrentScriptAsset = nullptr;
		CurrentScript = nullptr;
		SetEnabled(false);
	}

	void CScriptTool::CommitEdit()
	{
		for (const SNode* node : CurrentScript->Nodes)
		{
			if (node == nullptr)
				continue;

			CurrentScriptAsset->NodePositionMap[node->UID] = GUI::GetNodePosition(node->UID);
		}

		if (Edit.NewNodeView != nullptr)
		{
			const SDataBindingGetNodeEditorContext* getterContext = dynamic_cast<SDataBindingGetNodeEditorContext*>(Edit.NewNodeView);
			const SDataBindingSetNodeEditorContext* setterContext = dynamic_cast<SDataBindingSetNodeEditorContext*>(Edit.NewNodeView);
			
			U64 newNodeUID = 0;
			if (getterContext != nullptr)
				newNodeUID = CurrentScript->NodeFactory->CreateNode(Edit.NewNodeView->GetRuntimeHash(), 0, CurrentScript, getterContext->DataBindingID)->UID;
			else if (setterContext != nullptr)
				newNodeUID = CurrentScript->NodeFactory->CreateNode(Edit.NewNodeView->GetRuntimeHash(), 0, CurrentScript, setterContext->DataBindingID)->UID;
			else
				newNodeUID = CurrentScript->NodeFactory->CreateNode(Edit.NewNodeView->GetRuntimeHash(), 0, CurrentScript)->UID;
			
			CurrentScriptAsset->NodePositionMap.emplace(newNodeUID, Edit.NewNodePosition);
			GUI::SetNodePosition(newNodeUID, Edit.NewNodePosition);
		}

		if (Edit.NewBinding.has_value())
		{
			const SDataBindingInitData newBindingData = Edit.NewBinding.value();
			const SScriptDataBinding& newBinding = CurrentScript->AddDataBinding(0, newBindingData.Name.AsString().c_str(), newBindingData.Type, newBindingData.ObjectType, newBindingData.AssetType);
			Manager->RegisterDataBindingNodeView<SDataBindingGetNodeEditorContext, SDataBindingGetNode>(CurrentScript, newBinding.UID);
			Manager->RegisterDataBindingNodeView<SDataBindingSetNodeEditorContext, SDataBindingSetNode>(CurrentScript, newBinding.UID);
			Edit.NewBinding.reset();
		}

		if (Edit.RemovedBindingID != 0)
		{
			CurrentScript->RemoveDataBinding(Edit.RemovedBindingID);
			Manager->RemoveDataBindingNodeView<SDataBindingGetNodeEditorContext, SDataBindingGetNode>(Edit.RemovedBindingID);
			Manager->RemoveDataBindingNodeView<SDataBindingSetNodeEditorContext, SDataBindingSetNode>(Edit.RemovedBindingID);
		}

		if (Edit.ModifiedLiteralValuePin != nullptr && !Edit.ModifiedLiteralValuePin->IsDataUnset()) // NW: Only literal data types need to set data from GUI->Engine, when they are unpinned.	
			CurrentScript->SetDataOnInput(Edit.ModifiedLiteralValuePin->UID, GetEngineTypeData(GetLiteralTypeData(Edit.ModifiedLiteralValuePin->Data)));

		for (const U64& nodeUID : Edit.RemovedNodes)
		{
			CurrentScript->RemoveNode(nodeUID);
		}

		if (Edit.NewLink.UID != 0)
			CurrentScript->Link(Edit.NewLink.StartPinUID, Edit.NewLink.EndPinUID);

		for (const SLink& removedLink : Edit.RemovedLinks)
			CurrentScript->Unlink(removedLink.StartPinUID, removedLink.EndPinUID);

		Edit = SNodeOperation();
	}

	void CScriptTool::RenderScript()
	{
		GUI::PushScriptStyleColor(EScriptStyleColor::Background, SColor(60));

		RenderNodes();
		
		CurrentDragPinType = EPinType::Unknown;

		for (const SLink& link : CurrentScript->Links)
		{
			SPin* startPin = GetPinFromID(link.StartPinUID, CurrentScript->Nodes);
			GUI::Link(link.UID, link.StartPinUID, link.EndPinUID, startPin != nullptr ? GetPinTypeColor(startPin->Type) : SColor::White);
		}

		HandleCreateAction();
		HandleDeleteAction();

		ContextMenu();
		CommandQueue();
	}

	void CScriptTool::RenderNodes()
	{
		const std::unordered_map<U64, Ptr<SNodeView>>& views = Manager->GetNodeViewsMap();
			
		for (SNode* node : CurrentScript->Nodes)
		{
			const U64 runtimeHash = CurrentScript->NodeIDToRuntimeHash.at(node->UID);
			const SNodeView* view = views.at(runtimeHash).get();

			const SVector2<F32> requiredSize = GetNodeSize(node, view);
			GUI::PushScriptStyleVar(EScriptStyleVar::NodePadding, SVector4(8.0f, 4.0f, 8.0f, 8.0f));
			GUI::BeginNode(node->UID);

			GUI::PushID(node->UID);
			GUI::BeginVertical("node", requiredSize);

			GUI::BeginHorizontal("header", SVector2<F32>(requiredSize.X, HeaderHeight));
			SVector2<F32> nodeNameCursorStart = GUI::GetCursorPos();
			nodeNameCursorStart.Y += 2.0f;
			GUI::SetCursorPos(nodeNameCursorStart);
			GUI::TextUnformatted(view->Name.c_str());
			GUI::EndHorizontal();

			const SVector4 headerRect = GUI::GetLastRect();
			GUI::Spring(0, GUI::GetStyleVar(EStyleVar::ItemSpacing).Y * 3.0f);

			const U64 maxPinColumnLength = UMath::Max(node->Inputs.size(), node->Outputs.size());
			for (U64 i = 0; i < maxPinColumnLength; i++)
			{
				SPin* inputPin = node->Inputs.size() > i ? &node->Inputs[i] : nullptr;
				SPin* outputPin = node->Outputs.size() > i ? &node->Outputs[i] : nullptr;

				if (inputPin != nullptr && inputPin->Direction == EPinDirection::Input)
				{
					GUI::PushScriptStyleVar(EScriptStyleVar::PivotAlignment, SVector2<F32>(0.1f, 0.5f));
					GUI::BeginPin(inputPin->UID, EGUIPinDirection::Input);

					const bool isPinLinked = IsPinLinked(inputPin->UID, CurrentScript->Links);

					if (!isPinLinked && inputPin->Type == CurrentDragPinType)
					{
						DrawPinIcon(*inputPin, isPinLinked, 255, true);
					}
					else if (IsPinTypeLiteral(*inputPin) && !isPinLinked)
					{
						const bool wasPinValueModified = DrawLiteralTypePin(*inputPin);
						if (wasPinValueModified)
							Edit.ModifiedLiteralValuePin = inputPin;
					}
					else
					{
						DrawPinIcon(*inputPin, isPinLinked, 200, false);
					}

					GUI::SameLine(0, 0);
					const F32 cursorY = GUI::GetCursorPosY();
					GUI::SetCursorPosY(cursorY + PinNameOffset);
					GUI::Text(inputPin->Name.c_str());
					GUI::SetCursorPosY(cursorY - PinNameOffset);

					GUI::EndPin();
					GUI::PopScriptStyleVar();
				}

				if (outputPin != nullptr && outputPin->Direction == EPinDirection::Output)
				{
					if (inputPin != nullptr)
						GUI::SameLine();

					constexpr F32 iconSize = 24.0f;
					const F32 nameWidth = GUI::CalculateTextSize(outputPin->Name.c_str()).X;
					const F32 indent = requiredSize.X - nameWidth - iconSize;
					GUI::Indent(indent);

					GUI::PushScriptStyleVar(EScriptStyleVar::PivotAlignment, SVector2<F32>(0.9f, 0.5f));
					GUI::BeginPin(outputPin->UID, EGUIPinDirection::Output);

					const F32 cursorX = GUI::GetCursorPosX();
					const F32 cursorY = GUI::GetCursorPosY();
					GUI::SetCursorPosY(cursorY + PinNameOffset);
					GUI::Text(outputPin->Name.c_str());
					GUI::SetCursorPos(SVector2<F32>(cursorX + nameWidth, cursorY));

					DrawPinIcon(*outputPin, IsPinLinked(outputPin->UID, CurrentScript->Links), 200, false);
					GUI::EndPin();
					GUI::PopScriptStyleVar();
					GUI::Unindent(indent);
				}
			}

			GUI::EndVertical();
			GUI::EndNode();

			if (GUI::IsItemVisible())
			{
				constexpr F32 nodeRounding = 2.5f;
				constexpr F32 nodeBorderWidth = 1.5f;
				const F32 halfBorderWidth = nodeBorderWidth * 0.5f;
				const SVector2<F32> uv = SVector2<F32>(
					(headerRect.Z - headerRect.X) / (F32)(4.0f * 64.0f),//width
					(headerRect.W - headerRect.Y) / (F32)(4.0f * 64.0f));//height

				const SVector2<F32> imagePadding = SVector2<F32>(8 - halfBorderWidth, 4 - halfBorderWidth);
				const SVector2<F32> imagePaddingMax = SVector2<F32>(8 - halfBorderWidth, 10 - halfBorderWidth);
				const SVector2<F32> imageMin = SVector2<F32>(headerRect.X - imagePadding.X, headerRect.Y - imagePadding.Y);
				const SVector2<F32> imageMax = SVector2<F32>(headerRect.Z + imagePaddingMax.X, headerRect.W + imagePaddingMax.Y);
				GUI::DrawNodeHeader(node->UID, Manager->GetResourceManager()->GetStaticEditorTextureResource(EEditorTexture::NodeBackground), imageMin, imageMax, SVector2<F32>(0.0f), uv, view->Color, nodeRounding);
			}
			GUI::PopID();
			GUI::PopScriptStyleVar();
		}
	}

	void CScriptTool::HandleCreateAction()
	{
		if (!GUI::BeginScriptCreate())
			return GUI::EndScriptCreate();

		U64 inputPinId, outputPinId = 0;
		if (!GUI::QueryNewLink(inputPinId, outputPinId))
		{
			const SPin* originPin = GetPinFromID(inputPinId, CurrentScript->Nodes);
			if (originPin->Type != EPinType::Unknown)
			{
				CurrentDragPinType = originPin->Type;
			}

			return GUI::EndScriptCreate();
		}

		if (inputPinId == 0 || outputPinId == 0)
			return GUI::EndScriptCreate();

		if (!GUI::AcceptNewScriptItem())
			return GUI::EndScriptCreate();

		SNode* firstNode = GetNodeFromPinID(inputPinId, CurrentScript->Nodes);
		SNode* secondNode = GetNodeFromPinID(outputPinId, CurrentScript->Nodes);
		assert(firstNode);
		assert(secondNode);

		if (firstNode == secondNode)
			return GUI::EndScriptCreate();

		const SPin* firstPin = GetPinFromID(inputPinId, firstNode);
		const SPin* secondPin = GetPinFromID(outputPinId, secondNode);

		bool canAddlink = true;
		if (firstPin && secondPin)
		{
			if (firstPin->Direction == EPinDirection::Input && secondPin->Direction == EPinDirection::Input)
			{
				canAddlink = false;
			}
		}

		if (firstPin && secondPin)
		{
			if (firstPin->Type != secondPin->Type)
			{
				canAddlink = false;
			}
		}

		// TODO.NW: Think about these, certain rules apply to flows vs nonflows/inputs vs outputs
		//if (!firstNode->CanAddLink(inputPinId))
		//{
		//	canAddlink = false;
		//}
		//if (!secondNode->CanAddLink(outputPinId))
		//{
		//	canAddlink = false;
		//}

		//if (firstNode->HasLinkBetween(inputPinId, outputPinId))
		//{
		//	canAddlink = false;
		//}

		if (canAddlink)
		{
			// TODO.NW: Add functions to populate this with function call
			static U64 linkID = 99;
			Edit.NewLink.UID = linkID++;
			Edit.NewLink.StartPinUID = firstPin->UID;
			Edit.NewLink.EndPinUID = secondPin->UID;

			//if (secondPin->Type == EPinType::Unknown)
			//{
			//	secondNode->ChangPinTypes(firstPin->Type);
			//}
			//int linkId = myNextLinkIdCounter++;
			//firstNode->AddLinkToVia(secondNode, inputPinId, outputPinId, linkId);
			//secondNode->AddLinkToVia(firstNode, outputPinId, inputPinId, linkId);

			//bool aIsCyclic = false;
			//WillBeCyclic(firstNode, secondNode, aIsCyclic, firstNode);
			//if (aIsCyclic || !canAddlink)
			//{
			//	firstNode->RemoveLinkToVia(secondNode, inputPinId);
			//	secondNode->RemoveLinkToVia(firstNode, outputPinId);
			//}
			//else
			//{
			//	// Depending on if you drew the new link from the output to the input we need to create the link as the flow FROM->TO to visualize the correct flow
			//	if (firstPin->Direction == EGUIPinDirection::Input)
			//	{
			//		myLinks.push_back({ GUI::LinkId(linkId), outputPinId, inputPinId });
			//	}
			//	else
			//	{
			//		myLinks.push_back({ GUI::LinkId(linkId), inputPinId, outputPinId });
			//	}		

			//	std::cout << "push add link command!" << std::endl;
			//	myUndoCommands.push({ CommandAction::AddLink, firstNode, secondNode, myLinks.back(), 0});
			//
			//	ReTriggerTree();
			//}
		}

		GUI::EndScriptCreate();
	}

	void CScriptTool::HandleDeleteAction()
	{
		if (!GUI::BeginScriptDelete())
			return GUI::EndScriptDelete();

		U64 deletedLinkId = 0;
		while (GUI::QueryDeletedLink(deletedLinkId))
		{
			if (GUI::AcceptDeletedScriptItem())
			{
				for (const SLink& link : CurrentScript->Links)
				{
					if (link.UID == deletedLinkId)
					{
						Edit.RemovedLinks.emplace_back(link);

						//if (myShouldPushCommand)
						//{
						//	std::cout << "push remove link action!" << std::endl;
						//	myUndoCommands.push({ CommandAction::RemoveLink, firstNode, secondNode, link, 0/*static_cast<unsigned int>(link.Id)*//*, static_cast<unsigned int>(link.UID), static_cast<unsigned int>(link.OutputId)*/ });
						//}
					}
				}
			}
		}

		U64 nodeId = 0;
		while (GUI::QueryDeletedNode(nodeId))
		{
			if (GUI::AcceptDeletedScriptItem())
			{
				for (const SNode* node : CurrentScript->Nodes)
				{
					if (node->UID == nodeId)
					{
						Edit.RemovedNodes.emplace_back(node->UID);

						//if (myShouldPushCommand) 
						//{
						//	std::cout << "Push delete command!" << std::endl;
						//	myUndoCommands.push({ CommandAction::Delete, (*it), nullptr,  {0,0,0}, (*it)->UID });
						//}
					}
				}
			}
		}
		GUI::EndScriptDelete();
	}

	void CScriptTool::ContextMenu()
	{
		const SVector2<F32> openPopupPosition = GUI::GetMousePosition();
		GUI::SuspendScript();

		if (GUI::ShowScriptContextMenu())
			GUI::OpenPopup("Create New Node");

		GUI::ResumeScript();

		GUI::SuspendScript();
		GUI::PushStyleVar(EStyleVar::WindowPadding, SVector2<F32>(8.0f, 8.0f));

		if (GUI::BeginPopup("Create New Node"))
		{
			const SVector2<F32> newNodePostion = openPopupPosition;
			
			for (const Ptr<SNodeView>& view : Manager->GetNodeViewsVector())
			{
				if (GUI::BeginMenu(view->Category.c_str()))
				{
					if (GUI::MenuItem(view->Name.c_str()))
					{
						Edit.NewNodeView = view.get();
						Edit.NewNodePosition = { openPopupPosition.X, openPopupPosition.Y };
					}
					GUI::EndMenu();
				}
			}

			GUI::Separator();
			if (GUI::MenuItem("Create New Data Binding"))
			{
				GUI::CloseCurrentPopup();
				GUI::EndPopup();
				GUI::OpenPopup("Create Data Binding");
			}
			else
				GUI::EndPopup();
		}

		if (GUI::BeginPopup("Create Data Binding"))
		{
			GUI::Text("New Data Binding");
			GUI::Separator();
			GUI::InputText("Name", &DataBindingCandidate.Name);

			GUI::ComboEnum("Pin Type", DataBindingCandidate.Type, { EPinType::Unknown, EPinType::Flow, EPinType::Delegate, EPinType::Function, EPinType::ComponentPtr, EPinType::ComponentPtrList, EPinType::EntityList });

			if (DataBindingCandidate.Type == EPinType::Asset)
			{
				GUI::ComboEnum("Asset Type", DataBindingCandidate.AssetType, { EAssetType::None });
			}
			else
			{
				DataBindingCandidate.AssetType = EAssetType::None;
			}

			if (DataBindingCandidate.Type == EPinType::ComponentPtr)
			{
				GUI::ComboEnum("Object Type", DataBindingCandidate.ObjectType, { EObjectDataType::None });
			}
			else
			{
				DataBindingCandidate.ObjectType = EObjectDataType::None;
			}

			if (GUI::Button("Create"))
			{
				Edit.NewBinding = DataBindingCandidate;
				DataBindingCandidate = { };
				GUI::CloseCurrentPopup();
			}
			if (GUI::Button("Cancel"))
			{
				DataBindingCandidate = { };
				GUI::CloseCurrentPopup();
			}
			GUI::EndPopup();
		}

		GUI::PopStyleVar();
		GUI::ResumeScript();
	}

	void CScriptTool::CommandQueue()
	{
		//myShouldPushCommand = true;

		//if (GUI::BeginShortcut())
		//{
			/*if (GUI::AcceptCopy())
			{
				SaveNodesToClipboard();
			}

			if (GUI::AcceptPaste())
			{
				LoadNodesFromClipboard();
			}

			if (GUI::AcceptUndo())
			{
				if (!myUndoCommands.empty())
				{
					myShouldPushCommand = false;
					GUI::ResetShortCutAction();
					auto& command = myUndoCommands.top();
					EditorCommand inverseCommand = command;
					CPin* firstPin;
					CPin* secondPin;

					switch (command.myAction)
					{
					case CGraphManager::CommandAction::Create:
						inverseCommand.myAction = CommandAction::Delete;
						GUI::DeleteNode(command.myResourceUID);
						break;
					case CGraphManager::CommandAction::Delete:
						inverseCommand.myAction = CommandAction::Create;
						myNodeInstancesInGraph.push_back(command.myNodeInstance);
						break;
					case CGraphManager::CommandAction::AddLink:
						inverseCommand.myAction = CommandAction::RemoveLink;
						GUI::DeleteLink(command.myEditorLinkInfo.Id);
						break;
					case CGraphManager::CommandAction::RemoveLink:
						inverseCommand.myAction = CommandAction::AddLink;
						command.myNodeInstance->AddLinkToVia(command.mySecondNodeInstance, command.myEditorLinkInfo.InputId, command.myEditorLinkInfo.OutputId, command.myResourceUID);
						command.mySecondNodeInstance->AddLinkToVia(command.myNodeInstance, command.myEditorLinkInfo.OutputId, command.myEditorLinkInfo.InputId, command.myResourceUID);

						firstPin = command.myNodeInstance->GetPinFromID(command.myEditorLinkInfo.InputId);
						secondPin = command.mySecondNodeInstance->GetPinFromID(command.myEditorLinkInfo.OutputId);

						if (firstPin->Direction == EGUIPinDirection::Input)
							myLinks.push_back({ command.myEditorLinkInfo.Id, command.myEditorLinkInfo.InputId, command.myEditorLinkInfo.OutputId });
						else
							myLinks.push_back({ command.myEditorLinkInfo.Id, command.myEditorLinkInfo.OutputId, command.myEditorLinkInfo.InputId });
						ReTriggerTree();
						break;
					default:
						break;
					}
					std::cout << "undo!" << std::endl;
					myUndoCommands.pop();
					std::cout << "Push redo command!" << std::endl;
					myRedoCommands.push(inverseCommand);
				}
			}

			if (GUI::AcceptRedo())
			{
				if (!myRedoCommands.empty())
				{
					myShouldPushCommand = false;
					GUI::ResetShortCutAction();
					auto& command = myRedoCommands.top();
					EditorCommand inverseCommand = command;
					CPin* firstPin;
					CPin* secondPin;

					switch (command.myAction)
					{
					case CGraphManager::CommandAction::Create:
						inverseCommand.myAction = CommandAction::Delete;
						GUI::DeleteNode(command.myResourceUID);
						break;
					case CGraphManager::CommandAction::Delete:
						inverseCommand.myAction = CommandAction::Create;
						myNodeInstancesInGraph.push_back(command.myNodeInstance);
						break;
					case CGraphManager::CommandAction::AddLink:
						inverseCommand.myAction = CommandAction::RemoveLink;
						GUI::DeleteLink(command.myEditorLinkInfo.Id);
						break;
					case CGraphManager::CommandAction::RemoveLink:
						inverseCommand.myAction = CommandAction::AddLink;
						command.myNodeInstance->AddLinkToVia(command.mySecondNodeInstance, command.myEditorLinkInfo.InputId, command.myEditorLinkInfo.OutputId, command.myResourceUID);
						command.mySecondNodeInstance->AddLinkToVia(command.myNodeInstance, command.myEditorLinkInfo.OutputId, command.myEditorLinkInfo.InputId, command.myResourceUID);

						firstPin = command.myNodeInstance->GetPinFromID(command.myEditorLinkInfo.InputId);
						secondPin = command.mySecondNodeInstance->GetPinFromID(command.myEditorLinkInfo.OutputId);

						if (firstPin->Direction == EGUIPinDirection::Input)
							myLinks.push_back({ command.myEditorLinkInfo.Id, command.myEditorLinkInfo.InputId, command.myEditorLinkInfo.OutputId });
						else
							myLinks.push_back({ command.myEditorLinkInfo.Id, command.myEditorLinkInfo.OutputId, command.myEditorLinkInfo.InputId });
						ReTriggerTree();
						break;
					default:
						break;
					}
					std::cout << "redo!" << std::endl;
					myRedoCommands.pop();
					std::cout << "Push undo command!" << std::endl;
					myUndoCommands.push(inverseCommand);
				}
			}*/
			//}
	}

	SVector2<F32> CScriptTool::GetNodeSize(const HexRune::SNode* node, const SNodeView* view)
	{
		constexpr F32 iconSize = 24.0f;
		constexpr F32 iconNamePadding = 6.0f;
		constexpr F32 iconPadding = iconSize + iconNamePadding * 1.5f;

		const I64 maxPinColumnLength = UMath::Max(node->Inputs.size(), node->Outputs.size());
		F32 inputMaxRequired = 0.0f;
		F32 outputMaxRequired = 0.0f;

		for (auto& pin : node->Inputs)
		{
			F32 nameWidth = GUI::CalculateTextSize(pin.Name.c_str()).X;
			if (nameWidth > inputMaxRequired)
				inputMaxRequired = nameWidth + iconPadding;
		}
		for (auto& pin : node->Outputs)
		{
			F32 nameWidth = GUI::CalculateTextSize(pin.Name.c_str()).X;
			if (nameWidth > outputMaxRequired)
				outputMaxRequired = nameWidth + iconPadding;
		}

		F32 requiredWidth = UMath::Max(GUI::CalculateTextSize(view->Name.c_str()).X, inputMaxRequired + outputMaxRequired);
		if (node->FlowType == EFlowType::Simple)
			requiredWidth += 50.0f;
		requiredWidth = UMath::Max(requiredWidth, 100.0f);
		return SVector2(requiredWidth, HeaderHeight + 1.5f * iconNamePadding + iconPadding * F32(maxPinColumnLength));
	}

	bool CScriptTool::IsPinLinked(U64 id, const std::vector<SLink>& links)
	{
		if (!id)
			return false;

		for (const SLink& link : links)
			if (link.StartPinUID == id || link.EndPinUID == id)
				return true;

		return false;
	}

	bool CScriptTool::IsPinTypeLiteral(SPin& pin)
	{
		return pin.Type == EPinType::String || pin.Type == EPinType::Bool || pin.Type == EPinType::Int || pin.Type == EPinType::Float;
	}

	bool CScriptTool::DrawLiteralTypePin(SPin& pin)
	{
		bool wasPinValueModified = false;
		constexpr F32 emptyItemWidth = 50.0f;

		const F32 cursorPosY = GUI::GetCursorPosY();
		GUI::SetCursorPosY(cursorPosY + 2.0f);

		GUI::PushID(pin.UID);
		GUI::PushItemWidth(emptyItemWidth);

		switch (pin.Type)
		{
		case EPinType::String:
		{
			if (pin.IsDataUnset())
				pin.Data = "";

			wasPinValueModified = GUI::InputText("##edit", std::get<std::string>(pin.Data));
			break;
		}
		case EPinType::Int:
		{
			if (pin.IsDataUnset())
				pin.Data = 0;

			wasPinValueModified = GUI::InputInt("##edit", std::get<I32>(pin.Data));
			break;
		}
		case EPinType::Bool:
		{
			if (pin.IsDataUnset())
				pin.Data = false;

			wasPinValueModified = GUI::Checkbox("##edit", std::get<bool>(pin.Data));
			break;
		}
		case EPinType::Float:
		{
			if (pin.IsDataUnset())
				pin.Data = 0.0f;

			wasPinValueModified = GUI::InputFloat("##edit", std::get<F32>(pin.Data));
			break;
		}
		default:
			assert(0);
		}

		GUI::PopItemWidth();
		GUI::PopID();

		return wasPinValueModified;
	}

	void CScriptTool::DrawPinIcon(const SPin& pin, bool connected, U8 alpha, bool highlighted)
	{
		EGUIIconType iconType = EGUIIconType::Flow;
		SColor color = GetPinTypeColor(pin.Type);
		color.A = alpha;
		
		switch (pin.Type)
		{
		case EPinType::Flow:     iconType = EGUIIconType::Flow;   break;
		case EPinType::Bool:     iconType = EGUIIconType::Circle; break;
		case EPinType::Int:      iconType = EGUIIconType::Circle; break;
		case EPinType::Float:    iconType = EGUIIconType::Circle; break;
		case EPinType::String:   iconType = EGUIIconType::Circle; break;
		case EPinType::Vector:   iconType = EGUIIconType::Circle; break;

		case EPinType::ComponentPtr:   iconType = EGUIIconType::Circle; break;
		case EPinType::ComponentPtrList:   iconType = EGUIIconType::Grid; break;

		case EPinType::Entity:   iconType = EGUIIconType::Circle; break;
		case EPinType::EntityList:   iconType = EGUIIconType::Grid; break;

		case EPinType::Asset:    iconType = EGUIIconType::Circle; break;
		case EPinType::Function: iconType = EGUIIconType::Circle; break;
		case EPinType::Delegate: iconType = EGUIIconType::Square; break;
		default:
			return;
		}

		GUI::DrawPinIcon(SVector2<F32>(24.0f), iconType, connected, color, highlighted);
	};

	SColor CScriptTool::GetPinTypeColor(EPinType type)
	{
		switch (type)
		{
		default:
		case EPinType::Flow:     return SColor(255, 255, 255);
		case EPinType::Bool:     return SColor(220, 48, 48);
		case EPinType::Int:      return SColor(68, 201, 156);
		case EPinType::Float:    return SColor(147, 226, 74);
		case EPinType::String:   return SColor(124, 21, 153);
		case EPinType::Vector:   return SColor(255, 206, 27);

		case EPinType::ComponentPtr:   return SColor(51, 150, 215);
		case EPinType::ComponentPtrList:   return SColor(51, 150, 215);

		case EPinType::Entity:   return SColor(51, 150, 215);
		case EPinType::EntityList:   return SColor(51, 150, 215);

		case EPinType::Asset:   return SColor(124, 21, 153);
		case EPinType::Function: return SColor(218, 0, 183);
		case EPinType::Delegate: return SColor(255, 48, 48);
		}
	};

	SNode* CScriptTool::GetNodeFromPinID(U64 id, std::vector<SNode*>& nodes)
	{
		for (SNode* node : nodes)
		{
			const SPin* pin = GetPinFromID(id, node);
			if (pin != nullptr && pin->UID == id)
				return node;
		}

		return nullptr;
	}

	SPin* CScriptTool::GetPinFromID(U64 id, SNode* node)
	{
		for (SPin& pin : node->Inputs)
		{
			if (pin.UID == id)
				return &pin;
		}

		for (SPin& pin : node->Outputs)
		{
			if (pin.UID == id)
				return &pin;
		}

		return nullptr;
	}

	SPin* CScriptTool::GetPinFromID(U64 id, std::vector<SNode*>& nodes)
	{
		for (SNode* node : nodes)
		{
			SPin* pin = GetPinFromID(id, node);
			if (pin != nullptr && pin->UID == id)
				return pin;
		}

		return nullptr;
	}

	SPin* CScriptTool::GetOutputPinFromID(U64 id, std::vector<SNode*>& nodes)
	{
		for (SNode* node : nodes)
		{
			for (SPin& pin : node->Outputs)
			{
				if (pin.UID == id)
					return &pin;
			}
		}
		return nullptr;
	}
}
