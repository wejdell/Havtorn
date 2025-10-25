// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "HexRune.h"
#include "ECS/GUIDManager.h"
#include "NodeEditorContexts/CoreNodeEditorContexts.h"
#include "NodeEditorContexts/ECSNodeEditorContexts.h"
#include "CoreNodes/CoreNodes.h"
#include "ECSNodes/ECSNodes.h"
#include <stack>

namespace Havtorn
{
	namespace HexRune
	{
		SScript::SScript()
		{

		}

		SScript::~SScript()
		{

		}

		void SScript::AddDataBinding(const char* name, const EPinType type, const EObjectDataType objectType, const EAssetType assetType)
		{
			std::variant<PIN_DATA_TYPES> data;
			switch (type)
			{
			case EPinType::Unknown:
				data = std::monostate{};
			case EPinType::Bool:
				data = false;
				break;
			case EPinType::Int:
				data = 0;
				break;
			case EPinType::Float:
				data = 0.0f;
				break;
			case EPinType::String:
				data = std::string("");
				break;
			case EPinType::Vector:
				data = SVector();
				break;
			case EPinType::Matrix:
				data = SMatrix();
				break;
			case EPinType::Quaternion:
				data = SQuaternion();
				break;
			case EPinType::Entity:
				data = SEntity::Null;
				break;
			case EPinType::ComponentPtr:
				data = nullptr;
				break;
			case EPinType::Asset:
				data = std::string();
				break;
			case EPinType::EntityList:
				data = std::vector<SEntity>();
				break;
			case EPinType::ComponentPtrList:
				data = std::vector<SComponent*>();
				break;
			}

			DataBindings.emplace_back(SScriptDataBinding());
			DataBindings.back().UID = UGUIDManager::Generate();
			DataBindings.back().Name = std::string(name);
			DataBindings.back().Type = type;
			DataBindings.back().ObjectType = objectType;
			DataBindings.back().AssetType = assetType;
			DataBindings.back().Data = data;
			auto context = RegisteredEditorContexts.emplace_back(new SDataBindingGetNodeEditorContext(this, DataBindings.back().UID));
			context->TypeID = 0;
			context = RegisteredEditorContexts.emplace_back(new SDataBindingSetNodeEditorContext(this, DataBindings.back().UID));
			context->TypeID = 1;
		}

		void SScript::RemoveDataBinding(const U64 id)
		{
			auto bindingIterator = std::ranges::find_if(DataBindings, [id](const SScriptDataBinding& binding) { return id == binding.UID; });
			if (bindingIterator == DataBindings.end())
				return;

			// TODO.NW: Make algo library for find_all_if
			std::vector<U64> nodesToRemove;
			for (auto node : Nodes)
			{
				if (SDataBindingGetNode* dataBindingNode = static_cast<SDataBindingGetNode*>(node))
				{
					if (dataBindingNode->DataBinding == &(*bindingIterator))
						nodesToRemove.push_back(dataBindingNode->UID);
				}
			}
			for (const U64 nodeId : nodesToRemove)
				RemoveNode(nodeId);

			// TODO.NW: Make sure contexts get deleted properly
			auto getterContextIterator = std::ranges::find_if(RegisteredEditorContexts, [id](const SNodeEditorContext* registeredContext)
															  {
																  if (const SDataBindingGetNodeEditorContext* context = static_cast<const SDataBindingGetNodeEditorContext*>(registeredContext))
																  {
																	  return context->DataBindingID == id;
																  }
																  return false;
															  });
			if (getterContextIterator != RegisteredEditorContexts.end())
				RegisteredEditorContexts.erase(getterContextIterator);

			auto setterContextIterator = std::ranges::find_if(RegisteredEditorContexts, [id](const SNodeEditorContext* registeredContext)
															  {
																  if (const SDataBindingSetNodeEditorContext* context = static_cast<const SDataBindingSetNodeEditorContext*>(registeredContext))
																  {
																	  return context->DataBindingID == id;
																  }
																  return false;
															  });
			if (setterContextIterator != RegisteredEditorContexts.end())
				RegisteredEditorContexts.erase(setterContextIterator);

			DataBindings.erase(bindingIterator);
		}

		void SScript::RemoveNode(const U64 id)
		{
			if (!NodeIndices.contains(id))
			{
				HV_LOG_ERROR("SScript::RemoveNode: Tried to remove node that doesn't exist!");
				return;
			}

			if (Nodes.empty())
			{
				HV_LOG_ERROR("SScript::RemoveNode: Tried to remove node from empty script!");
				return;
			}

			// TODO.NW: Make sure this removes the right thing?
			SNode*& nodeToBeRemoved = Nodes.back();
			if (nodeToBeRemoved != nullptr)
			{
				NodeIndices.at(nodeToBeRemoved->UID) = NodeIndices.at(id);
				std::swap(Nodes[NodeIndices[id]], Nodes.back());

				for (SPin& input : nodeToBeRemoved->Inputs)
				{
					if (input.LinkedPin)
						Unlink(input.LinkedPin, &input);
				}

				for (SPin& output : nodeToBeRemoved->Outputs)
				{
					if (output.LinkedPin)
						Unlink(&output, output.LinkedPin);
				}

				//nodeToBeRemoved->IsDeleted(this);
				delete nodeToBeRemoved;
				nodeToBeRemoved = nullptr;
			}

			RemoveContext(id);

			//if (NodeEditorContexts.contains(id))
			//    NodeEditorContexts.erase(id);

			Nodes.pop_back();
			NodeIndices.erase(id);
		}

		void SScript::Initialize()
		{
			U32 typeID = 0;
			NodeFactory = new SNodeFactory();
			NodeFactory->RegisterDatabindingNode<SDataBindingGetNode, SDataBindingGetNodeEditorContext>(typeID++);
			NodeFactory->RegisterDatabindingNode<SDataBindingSetNode, SDataBindingSetNodeEditorContext>(typeID++);
			NodeFactory->RegisterNodeType<SBranchNode, SBranchNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SSequenceNode, SSequenceNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SEntityLoopNode, SEntityLoopNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SComponentLoopNode, SComponentLoopNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SDelayNode, SDelayNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SBeginPlayNode, SBeginPlayNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<STickNode, STickNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SEndPlayNode, SEndPlayNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SPrintStringNode, SPrintStringNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SAppendStringNode, SAppendStringNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SFloatLessThanNode, SFloatLessThanNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SFloatMoreThanNode, SFloatMoreThanNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SFloatLessOrEqualNode, SFloatLessOrEqualNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SFloatMoreOrEqualNode, SFloatMoreOrEqualNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SFloatEqualNode, SFloatEqualNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SFloatNotEqualNode, SFloatNotEqualNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SIntLessThanNode, SIntLessThanNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SIntMoreThanNode, SIntMoreThanNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SIntLessOrEqualNode, SIntLessOrEqualNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SIntMoreOrEqualNode, SIntMoreOrEqualNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SIntEqualNode, SIntEqualNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SIntNotEqualNode, SIntNotEqualNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SPrintEntityNameNode, SPrintEntityNameNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SSetStaticMeshNode, SSetStaticMeshNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<STogglePointLightNode, STogglePointLightNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SOnBeginOverlapNode, SOnBeginOverlapNodeEditorContext>(this, typeID++);
			NodeFactory->RegisterNodeType<SOnEndOverlapNode, SOnEndOverlapNodeEditorContext>(this, typeID++);
			InitializeGame(typeID);
		}

		void SScript::TraverseFromNode(const U64 startNodeID, CScene* owningScene)
		{
			if (owningScene == nullptr)
				return;

			if (SNode* startNode = GetNode(startNodeID))
			{
				Scene = owningScene;
				startNode->Execute();
			}
		}

		void SScript::TraverseFromNode(SNode* startNode, CScene* owningScene)
		{
			if (owningScene == nullptr)
				return;

			Scene = owningScene;
			startNode->Execute();
		}

		void SScript::Link(U64 leftPinID, U64 rightPinID)
		{
			SPin* leftPin = nullptr;
			SPin* rightPin = nullptr;
			for (SNode* node : Nodes)
			{
				for (SPin& output : node->Outputs)
					if (output.UID == leftPinID)
						leftPin = &output;

				for (SPin& input : node->Inputs)
					if (input.UID == rightPinID)
						rightPin = &input;
			}

			if (leftPin == nullptr || rightPin == nullptr)
				return;

			// TODO.NW: Guard against linking the same link again
			Links.push_back(SLink{ UGUIDManager::Generate(), leftPinID, rightPinID });

			leftPin->LinkedPin = rightPin;
			rightPin->LinkedPin = leftPin;
		}

		void SScript::Link(SPin* leftPin, SPin* rightPin)
		{
			Links.push_back(SLink{ UGUIDManager::Generate(), leftPin->UID, rightPin->UID });
			leftPin->LinkedPin = rightPin;
			rightPin->LinkedPin = leftPin;
		}

		void SScript::Unlink(U64 leftPinID, U64 rightPinID)
		{
			SPin* leftPin = nullptr;
			SPin* rightPin = nullptr;
			for (SNode* node : Nodes)
			{
				for (SPin& output : node->Outputs)
					if (output.UID == leftPinID)
						leftPin = &output;

				for (SPin& input : node->Inputs)
					if (input.UID == rightPinID)
						rightPin = &input;
			}

			if (leftPin == nullptr || rightPin == nullptr)
				return;

			Unlink(leftPin, rightPin);
		}

		void SScript::Unlink(SPin* leftPin, SPin* rightPin)
		{
			U64 leftPinID = leftPin->UID;
			U64 rightPinID = rightPin->UID;

			auto it = std::ranges::find_if(Links, [leftPinID, rightPinID](const SLink& link) { return link.StartPinUID == leftPinID && link.EndPinUID == rightPinID; });
			if (it != Links.end())
				Links.erase(it);

			rightPin->ClearData();

			leftPin->LinkedPin = nullptr;
			rightPin->LinkedPin = nullptr;
		}

		void SScript::SetDataOnInput(U64 pinID, const std::variant<PIN_DATA_TYPES>& data)
		{
			for (auto node : Nodes)
			{
				auto it = std::ranges::find_if(node->Inputs, [pinID](const SPin& pin) { return pin.UID == pinID; });
				if (it != node->Inputs.end())
				{
					it->Data = data;
				}
			}
		}

		U32 SScript::GetSize() const
		{
			//Databindings -> Nodes -> Links
			U32 size = 0;
			size += sizeof(U32);
			for (auto& databinding : DataBindings)
			{
				size += databinding.GetSize();
			}

			size += sizeof(U32);
			for (auto& node : Nodes)
			{
				size += GetDataSize(node->UID);
				size += GetDataSize(node->TypeID);
				size += GetDataSize(node->NodeType);

				if (node->NodeType == ENodeType::DataBindingGetNode || node->NodeType == ENodeType::DataBindingSetNode)
				{
					size += sizeof(U64);
				}

				size += STATIC_U32(sizeof(SVector2<F32>)); // Position
				size += STATIC_U32(sizeof(U32));
				size += STATIC_U32(node->Inputs.size() * sizeof(U64));
				size += STATIC_U32(sizeof(U32));
				size += STATIC_U32(node->Outputs.size() * sizeof(U64));
			}

			size += GetDataSize(Links);
			return size;
		}
		void SScript::Serialize(char* toData, U64& pointerPosition) const
		{
			//Databindings -> Nodes -> Links
			SerializeData(STATIC_U32(DataBindings.size()), toData, pointerPosition);

			for (auto& db : DataBindings)
				db.Serialize(toData, pointerPosition);

			U32 nodeCount = STATIC_U32(Nodes.size());
			SerializeData(nodeCount, toData, pointerPosition);
			for (SNode* node : Nodes)
			{				
				SerializeData(node->UID, toData, pointerPosition);
				SerializeData(node->TypeID, toData, pointerPosition);
				SerializeData(node->NodeType, toData, pointerPosition);

				if (node->NodeType == ENodeType::DataBindingGetNode)
				{
					SDataBindingGetNode* dbNode = dynamic_cast<SDataBindingGetNode*>(node);
					SerializeData(dbNode->DataBinding->UID, toData, pointerPosition);
				}
				if (node->NodeType == ENodeType::DataBindingSetNode)
				{
					SDataBindingSetNode* dbNode = dynamic_cast<SDataBindingSetNode*>(node);
					SerializeData(dbNode->DataBinding->UID, toData, pointerPosition);
				}

				SerializeData(GetNodeEditorContext(node->UID)->Position, toData, pointerPosition);

				std::vector<U64> inputPinIds;
				for (auto& pin : node->Inputs)
					inputPinIds.emplace_back(pin.UID);
				SerializeData(inputPinIds, toData, pointerPosition);

				std::vector<U64> outputPinIds;
				for (auto& pin : node->Outputs)
					outputPinIds.emplace_back(pin.UID);
				SerializeData(outputPinIds, toData, pointerPosition);
			}

			SerializeData(Links, toData, pointerPosition);

		}
		void SScript::Deserialize(const char* fromData, U64& pointerPosition)
		{
			// TODO.NW: Serialize nodes through protocol
			U32 databindingCount = 0;
			DeserializeData(databindingCount, fromData, pointerPosition);

			for (U32 i = 0; i < databindingCount; i++)
			{
				SScriptDataBinding databinding = {};
				databinding.Deserialize(fromData, pointerPosition);
				DataBindings.emplace_back(databinding);
				RegisteredEditorContexts.emplace_back(new SDataBindingGetNodeEditorContext(this, databinding.UID));
				RegisteredEditorContexts.emplace_back(new SDataBindingSetNodeEditorContext(this, databinding.UID));
			}

			U32 nodeCount = 0;
			DeserializeData(nodeCount, fromData, pointerPosition);

			for (U32 i = 0; i < nodeCount; i++)
			{
				U64 uid;
				DeserializeData(uid, fromData, pointerPosition);

				U32 nodeTypeId;
				DeserializeData(nodeTypeId, fromData, pointerPosition);
			
				ENodeType nodeType;
				DeserializeData(nodeType, fromData, pointerPosition);

				SNode* node = nullptr;
				if (nodeType == ENodeType::DataBindingGetNode || nodeType == ENodeType::DataBindingSetNode)
				{
					U64 dbUID;
					DeserializeData(dbUID, fromData, pointerPosition);
					node = NodeFactory->CreateNode(nodeTypeId, uid, this, dbUID);
				}
				else
				{
					node = NodeFactory->CreateNode(nodeTypeId, uid, this);
				}

				SVector2<F32> nodeEditorPosition;
				DeserializeData(nodeEditorPosition, fromData, pointerPosition);

				SNodeEditorContext* editorContext = GetNodeEditorContext(uid);
				editorContext->Position = nodeEditorPosition;

				std::vector<U64> inputPinIds;
				DeserializeData(inputPinIds, fromData, pointerPosition);

				// TODO.NW: This part is a bit sketchy, ideally these vectors should be
				// the same length but should we change a node implementation somewhere 
				// then the one saved on disk will be wrong. This might not be handled 
				// gracefully enough here.
				node->Inputs.resize(inputPinIds.size());
				for (U32 pinIndex = 0; pinIndex < inputPinIds.size(); pinIndex++)
				{
					node->Inputs[pinIndex].UID = inputPinIds[pinIndex];
				}

				std::vector<U64> outputPinIds;
				DeserializeData(outputPinIds, fromData, pointerPosition);

				node->Outputs.resize(outputPinIds.size());
				for (U32 pinIndex = 0; pinIndex < outputPinIds.size(); pinIndex++)
				{
					node->Outputs[pinIndex].UID = outputPinIds[pinIndex];
				}
			}

			DeserializeData(Links, fromData, pointerPosition);
		}

		SNode* SScript::GetNode(const U64 id) const
		{
			if (!HasNode(id))
				return nullptr;

			return Nodes[NodeIndices.at(id)];
		}

		bool SScript::HasNode(const U64 id) const
		{
			return NodeIndices.contains(id);
		}

		void SScript::RemoveContext(const U64 nodeID)
		{
			// TODO.NW: Think about if it's nicer to map node type to context type and access directly here, without looping
			for (SNodeContextStorage& storage : ContextStorages)
			{
				if (!storage.NodeIDToContextIndices.contains(nodeID))
					continue;

				std::unordered_map<U64, U64>& contextIndices = storage.NodeIDToContextIndices;
				std::vector<SNodeEditorContext*>& contexts = storage.Contexts;

				const std::pair<U64, U64>& maxIndexEntry = *std::ranges::find_if(contextIndices,
																				 [contexts](const auto& entry) { return entry.second == contexts.size() - 1; });

				std::swap(contexts[contextIndices.at(nodeID)], contexts[contextIndices.at(maxIndexEntry.first)]);
				std::swap(contextIndices.at(maxIndexEntry.first), contextIndices.at(nodeID));

				SNodeEditorContext* contextToBeRemoved = contexts[contextIndices.at(nodeID)];
				delete contextToBeRemoved;
				contextToBeRemoved = nullptr;

				contexts.erase(contexts.begin() + contextIndices.at(nodeID));
				contextIndices.erase(nodeID);
				break;
			}
		}

		SNodeEditorContext* SScript::GetNodeEditorContext(const U64 nodeID) const
		{
			// TODO.NW: Think about if it's nicer to map node type to context type and access directly here, without looping
			for (const SNodeContextStorage& storage : ContextStorages)
			{
				if (!storage.NodeIDToContextIndices.contains(nodeID))
					continue;

				// TODO.NW: Would maybe be nice to make our own Storage data structure for this, as well as ECS components
				return storage.Contexts[storage.NodeIDToContextIndices.at(nodeID)];
			}

			return nullptr;
		}

		SNode::SNode(const U64 id, const U32 typeID, SScript* owningScript, ENodeType nodeType)
			: UID(id)
			, TypeID(typeID)
			, NodeType(nodeType)
			, OwningScript(owningScript)
		{
		}

		SPin& SNode::AddInput(const U64 id, const EPinType type, const std::string& name)
		{
			SPin& pin = Inputs.emplace_back();
			pin.UID = id;
			pin.Name = name;
			pin.Type = type;
			pin.Direction = EPinDirection::Input;
			pin.OwningNode = this;

			return pin;
		}

		SPin& SNode::AddOutput(const U64 id, const EPinType type, const std::string& name)
		{
			SPin& pin = Outputs.emplace_back();
			pin.UID = id;
			pin.Name = name;
			pin.Type = type;
			pin.Direction = EPinDirection::Output;
			pin.OwningNode = this;

			return pin;
		}

		void SNode::Execute()
		{
			// Validate inputs
			for (SPin& pin : Inputs)
			{
				// Only flow input pins can be multiply linked
				pin.DeriveInput();
			}

			// Run custom logic, set data on output pins
			I8 pinIndex = OnExecute();

			if (UMath::IsWithin(pinIndex, STATIC_I8(0), STATIC_I8(Outputs.size())))
			{
				SPin& pinToExecute = Outputs[pinIndex];
				if (pinToExecute.Type == EPinType::Flow)
				{
					if (pinToExecute.LinkedPin != nullptr)
						pinToExecute.LinkedPin->OwningNode->Execute();
				}
			}
			else if (pinIndex == -1)
			{
				for (SPin& pin : Outputs)
				{
					// Only non-flow output pins can be multiply linked
					if (pin.Type == EPinType::Flow)
					{
						if (pin.LinkedPin != nullptr)
							pin.LinkedPin->OwningNode->Execute();
					}
				}
			}
			// else if (pinIndex == -2), defer execution
		}

		I8 SNode::OnExecute()
		{
			return -1;
		}

		U32 SScriptDataBinding::GetSize() const
		{
			U32 size = 0;
			size += GetDataSize(UID);
			size += GetDataSize(Name);
			size += GetDataSize(Type);
			size += GetDataSize(ObjectType);
			size += GetDataSize(AssetType);

			size += sizeof(EPinType);
			switch (Type)
			{
			case EPinType::Unknown:
				//size += sizeof(std::monostate);
				break;
			case EPinType::Bool:
				size += sizeof(bool);
				break;
			case EPinType::Int:
				size += sizeof(I32);
				break;
			case EPinType::Float:
				size += sizeof(F32);
				break;
			case EPinType::String:
			{
				std::string stringData = std::get<std::string>(Data);
				size += GetDataSize(stringData);
				break;
			}
			case EPinType::Vector:
				size += sizeof(SVector);
				break;
			case EPinType::Matrix:
				size += sizeof(SMatrix);
				break;
			case EPinType::Quaternion:
				size += sizeof(SQuaternion);
				break;
			case EPinType::Entity:
				size += sizeof(SEntity);
				break;
			}
			return size;
		}
		void SScriptDataBinding::Serialize(char* toData, U64& pointerPosition) const
		{
			SerializeData(UID, toData, pointerPosition);
			SerializeData(Name, toData, pointerPosition);
			SerializeData(Type, toData, pointerPosition);
			SerializeData(ObjectType, toData, pointerPosition);
			SerializeData(AssetType, toData, pointerPosition);

			//EPinType dataType = static_cast<EPinType>(Data.index());
			//SerializeData(dataType, toData, pointerPosition);
			switch (Type)
			{
			case EPinType::Unknown:
				break;
			case EPinType::Bool:
			{
				auto value = std::get<bool>(Data);
				SerializeData(value, toData, pointerPosition);
			}
			break;
			case EPinType::Int:
			{
				auto value = std::get<I32>(Data);
				SerializeData(value, toData, pointerPosition);
			}
			break;
			break;
			case EPinType::Float:
			{
				auto value = std::get<F32>(Data);
				SerializeData(value, toData, pointerPosition);
			}
			break;
			break;
			case EPinType::String:
			{
				auto value = std::get<std::string>(Data);
				SerializeData(value, toData, pointerPosition);
			}
			break;
			break;
			case EPinType::Vector:
			{
				auto value = std::get<SVector>(Data);
				SerializeData(value, toData, pointerPosition);
			}
			break;
			break;
			case EPinType::Matrix:
			{
				auto value = std::get<SMatrix>(Data);
				SerializeData(value, toData, pointerPosition);
			}
			break;
			break;
			case EPinType::Quaternion:
			{
				auto value = std::get<SQuaternion>(Data);
				SerializeData(value, toData, pointerPosition);
			}
			break;
			break;
			case EPinType::Entity:
			{
				auto value = std::get<SEntity>(Data);
				SerializeData(value, toData, pointerPosition);
			}
			break;
			}
		}
		void SScriptDataBinding::Deserialize(const char* fromData, U64& pointerPosition)
		{
			DeserializeData(UID, fromData, pointerPosition);
			DeserializeData(Name, fromData, pointerPosition);
			DeserializeData(Type, fromData, pointerPosition);
			DeserializeData(ObjectType, fromData, pointerPosition);
			DeserializeData(AssetType, fromData, pointerPosition);
			switch (Type)
			{
			case EPinType::Unknown:
			{
				Data = std::monostate{};
			}
			break;
			case EPinType::Bool:
			{
				auto value = false;
				DeserializeData(value, fromData, pointerPosition);
				Data = value;
			}
			break;
			case EPinType::Int:
			{
				auto value = I32{};
				DeserializeData(value, fromData, pointerPosition);
				Data = value;
			}
			break;
			case EPinType::Float:
			{
				auto value = F32{};
				DeserializeData(value, fromData, pointerPosition);
				Data = value;
			}
			break;
			case EPinType::String:
			{
				auto value = std::string();
				DeserializeData(value, fromData, pointerPosition);
				Data = value;
			}
			break;
			case EPinType::Vector:
			{
				auto value = SVector{};
				DeserializeData(value, fromData, pointerPosition);
				Data = value;
			}
			break;
			case EPinType::Matrix:
			{
				auto value = SMatrix{};
				DeserializeData(value, fromData, pointerPosition);
				Data = value;
			}
			break;
			case EPinType::Quaternion:
			{
				auto value = SQuaternion{};
				DeserializeData(value, fromData, pointerPosition);
				Data = value;
			}
			break;
			case EPinType::Entity:
			{
				auto value = SEntity::Null;
				DeserializeData(value, fromData, pointerPosition);
				Data = value;
			}
			break;
			}
		}

		SNode* SNodeFactory::CreateNode(U32 typeID, U64 id, SScript* script)
		{
			return BasicNodeFactoryMap[typeID](id, typeID, script);
		}
		SNode* SNodeFactory::CreateNode(U32 typeID, U64 id, SScript* script, const U64 databindingId)
		{
			return DatabindingNodeFactoryMap[typeID](id, typeID, script, databindingId);
		}
	}
}