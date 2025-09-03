// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"

#include <HexRune/HexRune.h>
#include <ECS/GUIDManager.h>
#include <ECS/Components/TransformComponent.h>
#include <HexRune/CoreNodes/CoreNodes.h>
#include <FileSystem/FileSystem.h>
#include "magic_enum.h"
#include "ScriptTool.h"

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
			[]<IsPinLiteralType T>(const T& x) { return std::variant<PIN_LITERAL_TYPES, PIN_MATH_TYPES>{x}; },
			[](auto&) { return std::variant<PIN_LITERAL_TYPES, PIN_MATH_TYPES>{}; }
		}, engineData
	);
}

std::variant<PIN_DATA_TYPES> GetEngineTypeData(const std::variant<PIN_LITERAL_TYPES, PIN_MATH_TYPES>& editorData)
{
	return std::visit(overloaded
		{
			[] <IsPinLiteralType T>(const T &x) { return std::variant<PIN_DATA_TYPES>{x}; }
		}, editorData
	);
}

namespace Havtorn
{
	using namespace HexRune;

	CScriptTool::CScriptTool(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
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

		if (CurrentScript == nullptr)
		{
			GUI::End();
			return;
		}

		{ // Menu Bar
			GUI::BeginChild("ScriptMenuBar", SVector2<F32>(0.0f, 30.0f));
			GUI::Text(UGeneralUtils::ExtractFileBaseNameFromPath(CurrentScript->FileName).c_str());
			GUI::SameLine();
			Filter.Draw("Search", 180);

			GUI::SameLine();
			if (GUI::Button("Save"))
			{
				Save();
			}

			GUI::Separator();
			GUI::EndChild();
		}

		Edit = SNodeOperation();

		{ // Data Bindings
			GUI::BeginChild("DataBindings", SVector2<F32>(150.0f, 0.0f), { EChildFlag::Borders, EChildFlag::ResizeX });
			GUI::Text("Data Bindings");
			GUI::Separator();



			for (auto& dataBinding : CurrentScript->DataBindings)
			{
				GUI::Text(dataBinding.Name.c_str());
				if (GUI::IsItemHovered())
				{

					if (dataBinding.Type != EPinType::ComponentPtr)
					{
						const char* items[]{
								"Unknown",
								"Flow",
								"Bool",
								"Int",
								"Float",
								"String",
								"Vector",
								"Int Array",
								"Float Array",
								"String Array",
								"Object",
								"Object Array",
								"Asset",
								"Function",
								"Delegate"
						};
						GUI::SetTooltip("%s", items[STATIC_U8(dataBinding.Type)]);
					}
				}

				if (GUI::BeginDragDropSource({ EDragDropFlag::SourceAllowNullID }))
				{
					SGuiPayload payload = GUI::GetDragDropPayload();
					if (payload.Data == nullptr)
					{
						GUI::SetDragDropPayload("DataBindingDrag", &dataBinding, sizeof(SScriptDataBinding));
					}
					GUI::Text(dataBinding.Name.c_str());

					GUI::EndDragDropSource();
				}

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

		LoadGUIElements();

		GUI::BeginScript("Node Script Editor");

		if (GUI::BeginDragDropTarget())
		{
			SGuiPayload payload = GUI::AcceptDragDropPayload("DataBindingDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNoDrawDefaultRect, EDragDropFlag::AcceptNopreviewTooltip });
			if (payload.Data != nullptr)
			{
				// TODO.NW: Make all similar reinterpret_casts static_casts instead
				SScriptDataBinding* dataBinding = static_cast<SScriptDataBinding*>(payload.Data);

				std::string tooltip = "Add Get ";
				tooltip.append(dataBinding->Name);
				tooltip.append(" node?");
				GUI::SetTooltip(tooltip.c_str());

				if (payload.IsDelivery)
				{
					// TODO.NW: Make sure to catch keybinds here
					for (auto& context : GUIContexts)
					{
						// TODO.NW: Maybe remove whitespace from names? Then they need extra care to display properly. Need another ID then, which we may have if we remove the extra GUIContexts/GUINode/GUILink layer
						if (context.Name == "Get " + dataBinding->Name)
						{
							Edit.NewNodeContext = context;
							Edit.NewNodePosition = GUI::GetMousePosition();
						}
					}
				}
			}

			GUI::EndDragDropTarget();
		}

		RenderScript();
		CommitEdit(Edit);
		GUI::EndScript();

		GUI::End();
	}

	void CScriptTool::OnDisable()
	{
	}

	void CScriptTool::OpenScript(SScript* script)
	{
		CurrentScript = script;
		SetEnabled(true);

		//LoadGUIElements();
		//GUI::OpenScript(GUINodes, GUILinks);
	}

	void CScriptTool::CloseScript()
	{
		SetEnabled(false);

		//LoadGUIElements();
		//GUI::CloseScript(GUINodes, GUILinks);

		//for (const SGUINode& node : GUINodes)
		//	CurrentScript->GetNodeEditorContext(node.UID)->Position = node.Position;
		//	//CurrentScript->GetNode(node.UID)->EditorPosition = node.Position;

		CurrentScript = nullptr;
	}

	void CScriptTool::LoadGUIElements()
	{
		// Extract elements from CurrentScript
		GUINodes.clear();
		GUILinks.clear();
		GUIContexts.clear();

		for (auto& node : CurrentScript->Nodes)
		{
			// TODO.NW: Make constructors for GUI elements that extract info from data elements?
			GUINodes.emplace_back();
			auto& guiNode = GUINodes.back();
			guiNode.UID = node->UID;
			guiNode.Type = static_cast<EGUINodeType>(node->FlowType);

			SNodeEditorContext* editorContext = CurrentScript->GetNodeEditorContext(node->UID);
			guiNode.Name = editorContext ? editorContext->Name : "Missing Context";
			guiNode.Color = editorContext ? editorContext->Color : SColor::Orange;
			guiNode.Position = editorContext ? editorContext->Position : SVector2<F32>::Zero;
			guiNode.HasBeenInitialized = editorContext ? editorContext->HasBeenInitialized : true;

			for (auto& input : node->Inputs)
			{
				guiNode.Inputs.emplace_back(SGUIPin(input.UID, static_cast<EGUIPinType>(input.Type), static_cast<EGUIPinDirection>(input.Direction), &guiNode, input.Name));
				if (input.IsPinTypeLiteral())
					guiNode.Inputs.back().Data = GetLiteralTypeData(input.Data);
			}
			for (auto& output : node->Outputs)
			{
				guiNode.Outputs.emplace_back(SGUIPin(output.UID, static_cast<EGUIPinType>(output.Type), static_cast<EGUIPinDirection>(output.Direction), &guiNode, output.Name));
			}
		}

		for (auto& link : CurrentScript->Links)
			GUILinks.emplace_back(SGUILink{ link.UID, link.StartPinUID, link.EndPinUID });

		for (U64 i = 0; i < CurrentScript->RegisteredEditorContexts.size(); i++)
		{
			SNodeEditorContext* registeredContext = CurrentScript->RegisteredEditorContexts[i];
			GUIContexts.emplace_back(SGUINodeContext{ registeredContext->Name, registeredContext->Category, STATIC_I64(i) });
		}
	}

	void CScriptTool::CommitEdit(const SNodeOperation& edit)
	{
		// Edit CurrentScript here
		for (auto& node : GUINodes)
		{
			if (SNodeEditorContext* context = CurrentScript->GetNodeEditorContext(node.UID))
			{
				context->Position = node.Position;
				context->HasBeenInitialized = true;
			}
		}

		if (edit.NewNodeContext.Index > -1)
		{
			SNode* newNode = CurrentScript->RegisteredEditorContexts[edit.NewNodeContext.Index]->AddNode(CurrentScript, 0);
			SNodeEditorContext* newContext = CurrentScript->GetNodeEditorContext(newNode->UID);
			newContext->Position = edit.NewNodePosition;
		}

		if (edit.NewBinding.Type != EGUIPinType::Unknown)
			CurrentScript->AddDataBinding(edit.NewBinding.Name.AsString().c_str(), static_cast<HexRune::EPinType>(edit.NewBinding.Type), static_cast<HexRune::EObjectDataType>(edit.NewBinding.ObjectType), static_cast<EAssetType>(edit.NewBinding.AssetType));

		if (edit.RemovedBindingID != 0)
			CurrentScript->RemoveDataBinding(edit.RemovedBindingID);

		if (!edit.ModifiedLiteralValuePin.IsDataUnset())
			CurrentScript->SetDataOnInput(edit.ModifiedLiteralValuePin.UID, GetEngineTypeData(edit.ModifiedLiteralValuePin.Data));

		for (auto& removedNode : edit.RemovedNodes)
			CurrentScript->RemoveNode(removedNode.UID);

		if (edit.NewLink.UID != 0)
			CurrentScript->Link(edit.NewLink.StartPinID, edit.NewLink.EndPinID);

		for (auto& removedLink : edit.RemovedLinks)
			CurrentScript->Unlink(removedLink.StartPinID, removedLink.EndPinID);
	}

	void CScriptTool::RenderScript()
	{
		GUI::PushScriptStyleColor(EScriptStyleColor::Background, SColor(60));

		RenderNodes();
		CurrentDragPinType = EGUIPinType::Unknown;
		//PinIdPositionMap.clear();

		for (auto& linkInfo : GUILinks)
		{
			SGUIPin* startPin = GetPinFromID(linkInfo.StartPinID, GUINodes);
			GUI::Link(linkInfo.UID, linkInfo.StartPinID, linkInfo.EndPinID, startPin != nullptr ? GetPinTypeColor(startPin->Type) : SColor::White);
		}

		HandleCreateAction();
		HandleDeleteAction();

		ContextMenu();
		CommandQueue();
	}

	void CScriptTool::RenderNodes()
	{
		for (auto& node : GUINodes)
		{
			SVector2<F32> requiredSize = GetNodeSize(node);
			GUI::PushScriptStyleVar(EScriptStyleVar::NodePadding, SVector4(8.0f, 4.0f, 8.0f, 8.0f));
			GUI::BeginNode(node.UID);

			if (!node.HasBeenInitialized)
				GUI::SetNodePosition(node.UID, node.Position);

			GUI::PushID(node.UID);
			GUI::BeginVertical("node", requiredSize);

			SVector4 headerRect = SVector4();

			GUI::BeginHorizontal("header", SVector2<F32>(requiredSize.X, HeaderHeight));
			SVector2<F32> nodeNameCursorStart = GUI::GetCursorPos();
			nodeNameCursorStart.Y += 2.0f;
			GUI::SetCursorPos(nodeNameCursorStart);
			GUI::TextUnformatted(node.Name.c_str());
			GUI::EndHorizontal();
			headerRect = GUI::GetLastRect();
			GUI::Spring(0, GUI::GetStyleVar(EStyleVar::ItemSpacing).Y * 3.0f);

			U64 maxPinColumnLength = UMath::Max(node.Inputs.size(), node.Outputs.size());
			for (U64 i = 0; i < maxPinColumnLength; i++)
			{
				SGUIPin* inputPin = node.Inputs.size() > i ? &node.Inputs[i] : nullptr;
				SGUIPin* outputPin = node.Outputs.size() > i ? &node.Outputs[i] : nullptr;

				if (inputPin != nullptr && inputPin->Direction == EGUIPinDirection::Input)
				{
					GUI::PushScriptStyleVar(EScriptStyleVar::PivotAlignment, SVector2<F32>(0.1f, 0.5f));
					GUI::BeginPin(inputPin->UID, EGUIPinDirection::Input);

					const bool isPinLinked = IsPinLinked(inputPin->UID, GUILinks);

					if (!isPinLinked && inputPin->Type == CurrentDragPinType)
					{
						DrawPinIcon(*inputPin, isPinLinked, 255, true);
					}
					else if (IsPinTypeLiteral(*inputPin) && !isPinLinked)
					{
						const bool wasPinValueModified = DrawLiteralTypePin(*inputPin);
						if (wasPinValueModified)
							Edit.ModifiedLiteralValuePin = *inputPin;
					}
					else
					{
						DrawPinIcon(*inputPin, isPinLinked, 200, false);
					}

					GUI::SameLine(0, 0);
					F32 cursorY = GUI::GetCursorPosY();
					GUI::SetCursorPosY(cursorY + PinNameOffset);
					GUI::Text(inputPin->Name.c_str());
					GUI::SetCursorPosY(cursorY - PinNameOffset);

					GUI::EndPin();
					GUI::PopScriptStyleVar();
				}

				if (outputPin != nullptr && outputPin->Direction == EGUIPinDirection::Output)
				{
					if (inputPin != nullptr)
						GUI::SameLine();

					F32 nameWidth = GUI::CalculateTextSize(outputPin->Name.c_str()).X;
					constexpr F32 iconSize = 24.0f;
					F32 indent = requiredSize.X - nameWidth - iconSize;
					GUI::Indent(indent);

					GUI::PushScriptStyleVar(EScriptStyleVar::PivotAlignment, SVector2<F32>(0.9f, 0.5f));
					GUI::BeginPin(outputPin->UID, EGUIPinDirection::Output);

					F32 cursorX = GUI::GetCursorPosX();
					F32 cursorY = GUI::GetCursorPosY();
					GUI::SetCursorPosY(cursorY + PinNameOffset);
					GUI::Text(outputPin->Name.c_str());
					GUI::SetCursorPos(SVector2<F32>(cursorX + nameWidth, cursorY));

					DrawPinIcon(*outputPin, IsPinLinked(outputPin->UID, GUILinks), 200, false);
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
				const auto halfBorderWidth = nodeBorderWidth * 0.5f;
				const auto uv = SVector2<F32>(
					(headerRect.Z - headerRect.X) / (F32)(4.0f * 64.0f),//width
					(headerRect.W - headerRect.Y) / (F32)(4.0f * 64.0f));//height

				SVector2<F32> imagePadding = SVector2<F32>(8 - halfBorderWidth, 4 - halfBorderWidth);
				SVector2<F32> imagePaddingMax = SVector2<F32>(8 - halfBorderWidth, 10 - halfBorderWidth);
				SVector2<F32> imageMin = SVector2<F32>(headerRect.X - imagePadding.X, headerRect.Y - imagePadding.Y);
				SVector2<F32> imageMax = SVector2<F32>(headerRect.Z + imagePaddingMax.X, headerRect.W + imagePaddingMax.Y);
				GUI::DrawNodeHeader(node.UID, (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::NodeBackground).GetShaderResourceView(), imageMin, imageMax, SVector2<F32>(0.0f), uv, node.Color, nodeRounding);
			}
			GUI::PopID();
			GUI::PopScriptStyleVar();
		}
	}

	void CScriptTool::HandleCreateAction()
	{
		// Handle creation action, returns true if editor want to create new object (node or link)
		if (!GUI::BeginScriptCreate())
			return GUI::EndScriptCreate();

		U64 inputPinId, outputPinId = 0;
		if (!GUI::QueryNewLink(inputPinId, outputPinId))
		{
			SGUIPin* originPin = GetPinFromID(inputPinId, GUINodes);
			if (originPin->Type != EGUIPinType::Unknown)
			{
				CurrentDragPinType = originPin->Type;
			}


			return GUI::EndScriptCreate();
		}

		if (inputPinId == 0 || outputPinId == 0)
			return GUI::EndScriptCreate();

		if (!GUI::AcceptNewScriptItem())
			return GUI::EndScriptCreate();

		SGUINode* firstNode = GetNodeFromPinID(inputPinId, GUINodes);
		SGUINode* secondNode = GetNodeFromPinID(outputPinId, GUINodes);
		assert(firstNode);
		assert(secondNode);

		if (firstNode == secondNode)
			return GUI::EndScriptCreate();

		SGUIPin* firstPin = GetPinFromID(inputPinId, *firstNode);
		SGUIPin* secondPin = GetPinFromID(outputPinId, *secondNode);

		bool canAddlink = true;
		if (firstPin && secondPin)
		{
			if (firstPin->Direction == EGUIPinDirection::Input && secondPin->Direction == EGUIPinDirection::Input)
			{
				canAddlink = false;
			}
		}

		if (firstPin->Type != secondPin->Type)
		{
			canAddlink = false;
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
			Edit.NewLink.StartPinID = firstPin->UID;
			Edit.NewLink.EndPinID = secondPin->UID;

			//if (secondPin->Type == EGUIPinType::Unknown)
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
				for (SGUILink& link : GUILinks)
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
				for (SGUINode& node : GUINodes)
				{
					if (node.UID == nodeId)
					{
						Edit.RemovedNodes.emplace_back(node);

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
		SVector2<F32> openPopupPosition = GUI::GetMousePosition();
		GUI::SuspendScript();

		if (GUI::ShowScriptContextMenu())
			GUI::OpenPopup("Create New Node");

		GUI::ResumeScript();

		GUI::SuspendScript();
		GUI::PushStyleVar(EStyleVar::WindowPadding, SVector2<F32>(8.0f, 8.0f));

		if (GUI::BeginPopup("Create New Node"))
		{
			auto newNodePostion = openPopupPosition;
			//CNodeType** types = CNodeTypeCollector::GetAllNodeTypes();
			//unsigned short noOfTypes = CNodeTypeCollector::GetNodeTypeCount();
			//std::map<std::string, std::vector<CNodeType*>> cats;
			//for (int i = 0; i < noOfTypes; i++)
			//{
			//	cats[types[i]->GetNodeTypeCategory()].push_back(types[i]);
			//}
			//GUI::PushItemWidth(100.0f);
			//GUI::InputText("##edit", (char*)myMenuSeachField, 127);
			//if (mySetSearchFokus)
			//{
			//	GUI::SetKeyboardFocusHere(0);
			//}
			//mySetSearchFokus = false;
			//GUI::PopItemWidth();
			//if (myMenuSeachField[0] != '\0')
			//{
			//	std::vector<SDistBestResult> distanceResults;
			//	for (int i = 0; i < noOfTypes; i++)
			//	{
			//		distanceResults.push_back(SDistBestResult());
			//		SDistBestResult& dist = distanceResults.back();
			//		dist.ourInstance = types[i];
			//		dist.myScore = uiLevenshteinDistance<std::string>(types[i]->Name, myMenuSeachField);
			//	}
			//	std::sort(distanceResults.begin(), distanceResults.end(), less_than_key());
			//	int firstCost = distanceResults[0].myScore;
			//	for (int i = 0; i < distanceResults.size(); i++)
			//	{
			//		CNodeInstance* node = nullptr;
			//		if (GUI::MenuItem(distanceResults[i].ourInstance->Name.c_str()))
			//		{
			//			node = new CNodeInstance();
			//			int nodeType = i;
			//			node.myNodeType = distanceResults[i].ourInstance;
			//			node.ConstructUniquePins();
			//			GUI::SetNodePosition(node.UID, newNodePostion);
			//			node.myHasSetEditorPos = true;
			//			myNodeInstancesInGraph.push_back(node);
			//			if (myShouldPushCommand)
			//			{
			//				std::cout << "Push create command!" << std::endl;
			//				myUndoCommands.push({ CommandAction::Create, node, nullptr, {0,0,0}, node.UID});
			//			}
			//		}
			//		int distance = distanceResults[i].myScore - firstCost;
			//		if (distance > 3)
			//		{
			//			break;
			//		}
			//	}
			//}
			//else
			//{
				//for (auto& category : cats)
				//{
				//	std::string theCatName = category.first;
				//	if (theCatName.empty())
				//	{
				//		theCatName = "General";
				//	}
				//	if (GUI::BeginMenu(theCatName.c_str()))
				//	{
				//		CNodeInstance* node = nullptr;
				//		for (int i = 0; i < category.second.size(); i++)
				//		{
				//			CNodeType* type = category.second[i];
				//			if (GUI::MenuItem(type->Name.c_str()))
				//			{
				//				node = new CNodeInstance();
				//				int nodeType = i;
				//				node.myNodeType = type;
				//				node.ConstructUniquePins();
				//				GUI::SetNodePosition(node.UID, newNodePostion);
				//				node.myHasSetEditorPos = true;
				//				myNodeInstancesInGraph.push_back(node);
				//		
				//				if (myShouldPushCommand)
				//				{
				//					std::cout << "Push create command!" << std::endl;
				//					myUndoCommands.push({CommandAction::Create, node, nullptr, {0,0,0}, node.UID});
				//				}
				//			}
				//		}
				//		GUI::EndMenu();
				//	}
				//}

			for (auto& context : GUIContexts)
			{
				if (GUI::BeginMenu(context.Category.c_str()))
				{
					if (GUI::MenuItem(context.Name.c_str()))
					{
						Edit.NewNodeContext = context;
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
			//}		
		}

		if (GUI::BeginPopup("Create Data Binding"))
		{
			GUI::Text("New Data Binding");
			GUI::Separator();
			GUI::InputText("Name", &DataBindingCandidate.Name);


			DataBindingCandidate.Type = GUI::ComboEnum("Pin Type", DataBindingCandidate.Type); // <-- This could replace the commented code below 

			//GUI::SliderEnum("Type", DataBindingCandidate.Type,
			//				{ //TODO.NW: See if we can bind these strings at compile time or something, through a static a certain of string amount and enum Count?
			//					"Unknown",
			//					"Flow",
			//					"Bool",
			//					"Int",
			//					"Float",
			//					"String",
			//					"Vector",
			//					"Int Array",
			//					"Float Array",
			//					"String Array",
			//					"Object",
			//					"Object Array",
			//					"Asset",
			//					"Function",
			//					"Delegate"
			//				});

			// TODO.NW: Add filtering so we can't pick incorrect types e.g. unknown and flow
			if (DataBindingCandidate.Type == EGUIPinType::Unknown)
				DataBindingCandidate.Type = EGUIPinType::Bool;

			if (DataBindingCandidate.Type == EGUIPinType::Asset)
			{
				DataBindingCandidate.AssetType = GUI::ComboEnum("Asset Type", DataBindingCandidate.AssetType);

				//GUI::SliderEnum("Asset Type", DataBindingCandidate.AssetType,
				//	{
				//		"None",
				//		"StaticMesh",
				//		"SkeletalMesh",
				//		"Texture",
				//		"Material",
				//		"Animation",
				//		"SpriteAnimation",
				//		"AudioOneShot",
				//		"AudioCollection",
				//		"VisualFX",
				//		"Scene",
				//		"Sequencer",
				//		"Script"
				//	});

				if (DataBindingCandidate.AssetType == EGUIAssetType::None)
					DataBindingCandidate.AssetType = EGUIAssetType::StaticMesh;
			}
			else
			{
				DataBindingCandidate.AssetType = EGUIAssetType::None;
			}


			if (DataBindingCandidate.Type == EGUIPinType::ComponentPtr)
			{
				DataBindingCandidate.ObjectType = GUI::ComboEnum("Object Type", DataBindingCandidate.ObjectType);
				//GUI::SliderEnum("Object Type", DataBindingCandidate.ObjectType, { "None", "Entity", "Component" });

				if (DataBindingCandidate.ObjectType == EGUIObjectDataType::None)
					DataBindingCandidate.ObjectType = EGUIObjectDataType::Entity;
			}
			else
			{
				DataBindingCandidate.ObjectType = EGUIObjectDataType::None;
			}

			//GUI::TextDisabled("Name");
			//GUI::SameLine();
			//GUI::InputText("Name", (char*)name.c_str(), 64);

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

	void CScriptTool::Save()
	{
		GEngine::GetWorld()->SaveScript(CurrentScript->FileName);

		//SScriptFileHeader asset;
		//asset.Name = CurrentScript->FileName;
		//asset.AssetType = EAssetType::Script;
		//asset.Script = CurrentScript;
		////asset.Script = CurrentScript;
		//const auto data = new char[asset.GetSize()];
		//asset.Serialize(data);
		//GEngine::GetFileSystem()->Serialize(CurrentScript->FileName, &data[0], asset.GetSize());
		//std::filesystem::directory_entry newDir;
		//newDir.assign(std::filesystem::path(CurrentScript->FileName));
		//newDir.refresh();
		//Manager->RemoveAssetRep(newDir);
		//Manager->CreateAssetRep(newDir);
		/*
		if (Scenes.empty())
		{
			HV_LOG_ERROR("Tried to save empty Scene.");
			return;
		}
		const Ptr<CScene>& scene = Scenes.back();
		SSceneFileHeader fileHeader;
		fileHeader.Scene = scene.get();
		const U32 fileSize = GetDataSize(fileHeader.AssetType) + AssetRegistry->GetSize() + fileHeader.GetSize();
		char* data = new char[fileSize];
		U64 pointerPosition = 0;
		fileHeader.Serialize(data, pointerPosition, AssetRegistry.get());
		GEngine::GetFileSystem()->Serialize(destinationPath, data, fileSize);
		delete[] data;
		*/
	}

	SVector2<F32> CScriptTool::GetNodeSize(const SGUINode& node)
	{
		constexpr F32 iconSize = 24.0f;
		constexpr F32 iconNamePadding = 6.0f;
		constexpr F32 iconPadding = iconSize + iconNamePadding * 1.5f;

		I64 maxPinColumnLength = UMath::Max(node.Inputs.size(), node.Outputs.size());
		F32 inputMaxRequired = 0.0f;
		F32 outputMaxRequired = 0.0f;

		for (auto& pin : node.Inputs)
		{
			F32 nameWidth = GUI::CalculateTextSize(pin.Name.c_str()).X;
			if (nameWidth > inputMaxRequired)
				inputMaxRequired = nameWidth + iconPadding;
		}
		for (auto& pin : node.Outputs)
		{
			F32 nameWidth = GUI::CalculateTextSize(pin.Name.c_str()).X;
			if (nameWidth > outputMaxRequired)
				outputMaxRequired = nameWidth + iconPadding;
		}
		F32 requiredWidth = UMath::Max(GUI::CalculateTextSize(node.Name.c_str()).X, inputMaxRequired + outputMaxRequired);
		if (node.Type == EGUINodeType::Simple)
			requiredWidth += 50.0f;
		requiredWidth = UMath::Max(requiredWidth, 100.0f);
		return SVector2(requiredWidth, HeaderHeight + 1.5f * iconNamePadding + iconPadding * F32(maxPinColumnLength));
	}

	bool CScriptTool::IsPinLinked(U64 id, const std::vector<SGUILink>& links)
	{
		if (!id)
			return false;

		for (auto& link : links)
			if (link.StartPinID == id || link.EndPinID == id)
				return true;

		return false;
	}

	bool CScriptTool::IsPinTypeLiteral(SGUIPin& pin)
	{
		return pin.Type == EGUIPinType::String || pin.Type == EGUIPinType::Bool || pin.Type == EGUIPinType::Int || pin.Type == EGUIPinType::Float;
	}

	bool CScriptTool::DrawLiteralTypePin(SGUIPin& pin)
	{
		bool wasPinValueModified = false;
		constexpr F32 emptyItemWidth = 50.0f;

		F32 cursorPosY = GUI::GetCursorPosY();
		GUI::SetCursorPosY(cursorPosY + 2.0f);

		switch (pin.Type)
		{
		case EGUIPinType::String:
		{
			if (pin.IsDataUnset())
				pin.Data = "";

			GUI::PushID(pin.UID);
			GUI::PushItemWidth(emptyItemWidth);
			wasPinValueModified = GUI::InputText("##edit", std::get<std::string>(pin.Data));
			GUI::PopItemWidth();
			GUI::PopID();
			break;
		}
		case EGUIPinType::Int:
		{
			if (pin.IsDataUnset())
				pin.Data = 0;

			GUI::PushID(pin.UID);
			GUI::PushItemWidth(emptyItemWidth);
			wasPinValueModified = GUI::InputInt("##edit", std::get<I32>(pin.Data));
			GUI::PopItemWidth();
			GUI::PopID();
			break;
		}
		case EGUIPinType::Bool:
		{
			if (pin.IsDataUnset())
				pin.Data = false;

			GUI::PushID(pin.UID);
			GUI::PushItemWidth(emptyItemWidth);
			wasPinValueModified = GUI::Checkbox("##edit", std::get<bool>(pin.Data));
			GUI::PopItemWidth();
			GUI::PopID();
			break;
		}
		case EGUIPinType::Float:
		{
			if (pin.IsDataUnset())
				pin.Data = 0.0f;

			GUI::PushID(pin.UID);
			GUI::PushItemWidth(emptyItemWidth);
			wasPinValueModified = GUI::InputFloat("##edit", std::get<F32>(pin.Data));
			GUI::PopItemWidth();
			GUI::PopID();
			break;
		}
		default:
			assert(0);
		}

		return wasPinValueModified;
	}

	void CScriptTool::DrawPinIcon(const SGUIPin& pin, bool connected, U8 alpha, bool highlighted)
	{
		EGUIIconType iconType;
		SColor color = GetPinTypeColor(pin.Type);
		color.A = alpha;
		switch (pin.Type)
		{
		case EGUIPinType::Flow:     iconType = EGUIIconType::Flow;   break;
		case EGUIPinType::Bool:     iconType = EGUIIconType::Circle; break;
		case EGUIPinType::Int:      iconType = EGUIIconType::Circle; break;
		case EGUIPinType::Float:    iconType = EGUIIconType::Circle; break;
		case EGUIPinType::String:   iconType = EGUIIconType::Circle; break;
		case EGUIPinType::Vector:   iconType = EGUIIconType::Circle; break;
		
		case EGUIPinType::ComponentPtr:   iconType = EGUIIconType::Circle; break;
		case EGUIPinType::ComponentPtrList:   iconType = EGUIIconType::Grid; break;

		case EGUIPinType::Entity:   iconType = EGUIIconType::Circle; break;
		case EGUIPinType::EntityList:   iconType = EGUIIconType::Grid; break;
		
		case EGUIPinType::Asset:    iconType = EGUIIconType::Circle; break;
		case EGUIPinType::Function: iconType = EGUIIconType::Circle; break;
		case EGUIPinType::Delegate: iconType = EGUIIconType::Square; break;
			default:
				return;
		}

		GUI::DrawPinIcon(SVector2<F32>(24.0f), iconType, connected, color, highlighted);
	};

	SColor CScriptTool::GetPinTypeColor(EGUIPinType type)
	{
		switch (type)
		{
		default:
		case EGUIPinType::Flow:     return SColor(255, 255, 255);
		case EGUIPinType::Bool:     return SColor(220, 48, 48);
		case EGUIPinType::Int:      return SColor(68, 201, 156);
		case EGUIPinType::Float:    return SColor(147, 226, 74);
		case EGUIPinType::String:   return SColor(124, 21, 153);
		case EGUIPinType::Vector:   return SColor(255, 206, 27);
		
		case EGUIPinType::ComponentPtr:   return SColor(51, 150, 215);
		case EGUIPinType::ComponentPtrList:   return SColor(51, 150, 215);

		case EGUIPinType::Entity:   return SColor(51, 150, 215);
		case EGUIPinType::EntityList:   return SColor(51, 150, 215);

		case EGUIPinType::Asset:   return SColor(124, 21, 153);
		case EGUIPinType::Function: return SColor(218, 0, 183);
		case EGUIPinType::Delegate: return SColor(255, 48, 48);
		}
	};

	SGUINode* CScriptTool::GetNodeFromPinID(U64 id, std::vector<SGUINode>& nodes)
	{
		for (auto& node : nodes)
		{
			for (auto& pin : node.Inputs)
			{
				if (pin.UID == id)
				{
					return &node;
				}
			}

			for (auto& pin : node.Outputs)
			{
				if (pin.UID == id)
				{
					return &node;
				}
			}
		}

		return nullptr;
	}

	SGUIPin* CScriptTool::GetPinFromID(U64 id, SGUINode& node)
	{
		for (auto& pin : node.Inputs)
		{
			if (pin.UID == id)
				return &pin;
		}
		for (auto& pin : node.Outputs)
		{
			if (pin.UID == id)
				return &pin;
		}

		return nullptr;
	}

	SGUIPin* CScriptTool::GetPinFromID(U64 id, std::vector<SGUINode>& nodes)
	{
		for (auto& node : nodes)
		{
			for (auto& pin : node.Inputs)
			{
				if (pin.UID == id)
					return &pin;
			}
			for (auto& pin : node.Outputs)
			{
				if (pin.UID == id)
					return &pin;
			}
		}

		return nullptr;
	}

	SGUIPin* CScriptTool::GetOutputPinFromID(U64 id, std::vector<SGUINode>& nodes)
	{
		for (auto& node : nodes)
		{
			for (auto& pin : node.Outputs)
			{
				if (pin.UID == id)
					return &pin;
			}
		}
		return nullptr;
	}


}
