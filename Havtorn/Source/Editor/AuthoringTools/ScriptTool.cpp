// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"

#include <HexRune/HexRune.h>
#include <ECS/GUIDManager.h>
#include <ECS/Components/TransformComponent.h>
#include <HexRune/CoreNodes/CoreNodes.h>
#include <FileSystem/FileSystem.h>

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

	void SComponentsNode::Construct()
	{
		//Name = "Transform Components";
		//AddInput(UGUIDManager::Generate(), EPinType::Flow);
		//Inputs.back().Name = "In";
		//AddOutput(UGUIDManager::Generate(), EPinType::Flow);
		//Outputs.back().Name = "Out";
		//EditorColor = SColor::Orange;
		AddOutput(UGUIDManager::Generate(), EPinType::ObjectArray);
		Outputs.back().Name = "Components";
	}

	I8 SComponentsNode::OnExecute()
	{
		SetDataOnPin(EPinDirection::Output, 0, OwningScript->Scene->GetBaseComponents<STransformComponent>());
		return 0;
	}

	void STestNode::Construct()
	{
		//Name = "Test Node";
		//EditorColor = SColor::Red;
		AddInput(UGUIDManager::Generate(), EPinType::Flow);
		Inputs.back().Name = "In";
		AddOutput(UGUIDManager::Generate(), EPinType::Flow);
		Outputs.back().Name = "Out";

		AddInput(UGUIDManager::Generate(), EPinType::Float);
		AddInput(UGUIDManager::Generate(), EPinType::String);
		AddInput(UGUIDManager::Generate(), EPinType::ObjectArray);
	}

	I8 STestNode::OnExecute()
	{
		F32 pinData = 0.0f;
		GetDataOnPin(&Inputs[1], pinData);

		std::string otherPinData = {};
		GetDataOnPin(&Inputs[2], otherPinData);

		std::vector<SComponent*> transforms;
		GetDataOnPin(&Inputs[3], transforms);
		std::vector<STransformComponent*> transformComponents = CastComponents<STransformComponent>(transforms);


		HV_LOG_INFO("Script says: %s %f", otherPinData.c_str(), pinData);

		return 0;
	}

	void SLiteralFloatNode::Construct()
	{
		//Name = "Float";
		Type = ENodeType::Simple;
		AddInput(UGUIDManager::Generate(), EPinType::Float);
		AddOutput(UGUIDManager::Generate(), EPinType::Float);

		F32 input = 35.0f;
		SetDataOnPin(EPinDirection::Input, 0, input);
	}

	I8 SLiteralFloatNode::OnExecute()
	{
		// TODO.NW: Have pin index version that logs errors if outside of range?
		F32 pinData = 0.0f;
		GetDataOnPin(EPinDirection::Input, 0, pinData);
		SetDataOnPin(EPinDirection::Output, 0, pinData);
		return -1;
	}

	void SLiteralStringNode::Construct()
	{
		//TODO.NW: String array funkar inte heller, testa int/float och senare math types men kolla renderingen nästa gång
		//Name = "String";
		Type = ENodeType::Simple;
		AddInput(UGUIDManager::Generate(), EPinType::String);
		AddOutput(UGUIDManager::Generate(), EPinType::String);

		SetDataOnPin(EPinDirection::Input, 0, std::string("Hello:"));
	}

	I8 SLiteralStringNode::OnExecute()
	{
		std::string pinData = {};
		GetDataOnPin(EPinDirection::Input, 0, pinData);
		SetDataOnPin(EPinDirection::Output, 0, pinData);
		return -1;
	}

	CScriptTool::CScriptTool(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
	{
		CurrentScript = new SScript();
		CurrentScript->FileName = "Assets/Scripts/Test.hva";
		STestNode* testNode = CurrentScript->AddNode<STestNode>(UGUIDManager::Generate());
		SLiteralFloatNode* floatNode = CurrentScript->AddNode<SLiteralFloatNode>(UGUIDManager::Generate());
		CurrentScript->AddNode<SLiteralFloatNode>(UGUIDManager::Generate());
		SLiteralStringNode* stringNode = CurrentScript->AddNode<SLiteralStringNode>(UGUIDManager::Generate());
		CurrentScript->Link(&floatNode->Outputs[0], &testNode->Inputs[1]);
		CurrentScript->Link(&stringNode->Outputs[0], &testNode->Inputs[2]);

		SComponentsNode* componentsNode = CurrentScript->AddNode<SComponentsNode>(UGUIDManager::Generate());
		CurrentScript->Link(&componentsNode->Outputs[0], &testNode->Inputs[3]);

		CurrentScript->AddNode<SBranchNode>(UGUIDManager::Generate());
		//CurrentScript->Link(&floatNode2->Outputs[0], &floatNode->Inputs[0]);
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
				SScriptFileHeader asset;
				asset.Script = CurrentScript;
				const auto data = new char[asset.GetSize()];

				asset.Serialize(data);
				GEngine::GetFileSystem()->Serialize(CurrentScript->FileName, &data[0], asset.GetSize());
				
				std::filesystem::directory_entry newDir;
				newDir.assign(std::filesystem::path(CurrentScript->FileName));
				Manager->RemoveAssetRep(newDir);
				Manager->CreateAssetRep(newDir);
			}

			GUI::Separator();
			GUI::EndChild();
		}

		{ // Folder Tree
			GUI::BeginChild("FolderTree", SVector2<F32>(150.0f, 0.0f), { EChildFlag::Borders, EChildFlag::ResizeX });
			GUI::Text("Stuff");
			GUI::Separator();
			GUI::EndChild();
			GUI::SameLine();
		}

		LoadGUIElements();
		CommitEdit(GUI::RenderScript(GUINodes, GUILinks, GUIContexts));

		CurrentScript->TraverseScript(Manager->GetCurrentScene());

		GUI::End();
	}

	void CScriptTool::OnDisable()
	{
	}

	void CScriptTool::OpenScript(SScript* script)
	{
		CurrentScript = script;
		SetEnabled(true);

		LoadGUIElements();
		GUI::OpenScript(GUINodes, GUILinks);
	}

	void CScriptTool::CloseScript()
	{
		SetEnabled(false);

		LoadGUIElements();
		GUI::CloseScript(GUINodes, GUILinks);

		for (const SGUINode& node : GUINodes)
			CurrentScript->GetNodeEditorContext(node.UID)->Position = node.Position;
			//CurrentScript->GetNode(node.UID)->EditorPosition = node.Position;

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
			guiNode.Type = static_cast<EGUINodeType>(node->Type);

			SNodeEditorContext* editorContext = CurrentScript->GetNodeEditorContext(node->UID);
			guiNode.Name = editorContext ? editorContext->Name : /*node->Name*/"Missing Context";
			guiNode.Color = editorContext ? editorContext->Color : /*node->EditorColor*/SColor::Orange;
			guiNode.Position = editorContext ? editorContext->Position : /*node->EditorPosition*/SVector2<F32>::Zero;
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
			//newContext->HasBeenInitialized = true;
		}

		if (!edit.ModifiedLiteralValuePin.IsDataUnset())
			CurrentScript->SetDataOnInput(edit.ModifiedLiteralValuePin.UID, GetEngineTypeData(edit.ModifiedLiteralValuePin.Data));

		for (auto& removedNode : edit.RemovedNodes)
			CurrentScript->RemoveNode(removedNode.UID);

		if (edit.NewLink.UID != 0)
			CurrentScript->Link(edit.NewLink.StartPinID, edit.NewLink.EndPinID);

		for (auto& removedLink : edit.RemovedLinks)
			CurrentScript->Unlink(removedLink.StartPinID, removedLink.EndPinID);
	}
}
