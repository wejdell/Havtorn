// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "HexRune.h"
#include "ECS/GUIDManager.h"
#include "NodeEditorContexts/CoreNodeEditorContexts.h"
#include "CoreNodes/CoreNodes.h"

namespace Havtorn
{
	namespace HexRune
	{
        SScript::SScript()
        {
            Initialize();
        }

        void SScript::AddDataBinding(const char* name, const EPinType type, const EObjectDataType objectType)
        {
            std::variant<PIN_DATA_TYPES> data;
            switch (type)
            {
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
                data = "";
                break;
            case EPinType::Vector:
                break;
            case EPinType::IntArray:
                break;
            case EPinType::FloatArray:
                break;
            case EPinType::StringArray:
                break;
            case EPinType::Object:
                if (objectType == EObjectDataType::Entity)
                    data = SEntity::Null;
                else if (objectType == EObjectDataType::Component)
                    data = nullptr;
                break;
            case EPinType::ObjectArray:
                if (objectType == EObjectDataType::Entity)
                    data = { SEntity::Null };
                else if (objectType == EObjectDataType::Component)
                    data = { nullptr };
                break;
            }

            DataBindings.emplace_back(SScriptDataBinding());
            DataBindings.back().UID = UGUIDManager::Generate();
            DataBindings.back().Name = std::string(name);
            DataBindings.back().Type = type;
            DataBindings.back().ObjectType = objectType;
            DataBindings.back().Data = data;
            RegisteredEditorContexts.emplace_back(new SDataBindingGetNodeEditorContext(this, DataBindings.back().UID));
            RegisteredEditorContexts.emplace_back(new SDataBindingSetNodeEditorContext(this, DataBindings.back().UID));
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

                if (nodeToBeRemoved->IsStartNode())
                {
                    if (auto it = std::ranges::find(StartNodes, nodeToBeRemoved); it != StartNodes.end())
                        StartNodes.erase(it);
                }

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
            RegisteredEditorContexts.emplace_back(&SBranchNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SSequenceNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SEntityLoopNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SComponentLoopNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SDelayNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SBeginPlayNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&STickNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SEndPlayNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SPrintStringNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SAppendStringNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SFloatLessThanNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SFloatMoreThanNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SFloatLessOrEqualNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SFloatMoreOrEqualNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SFloatEqualNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SFloatNotEqualNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SIntLessThanNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SIntMoreThanNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SIntLessOrEqualNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SIntMoreOrEqualNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SIntEqualNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SIntNotEqualNodeEditorContext::Context);
            RegisteredEditorContexts.emplace_back(&SPrintEntityNameNodeEditorContext::Context);
        }

        void SScript::TraverseScript(CScene* owningScene)
        {
            if (owningScene == nullptr)
                return;

            Scene = owningScene;

            for (SNode* node : StartNodes)
                node->Execute();
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
            Links.push_back(SLink{UGUIDManager::Generate(), leftPinID, rightPinID});

            leftPin->LinkedPin = rightPin;
            rightPin->LinkedPin = leftPin;
        }

        void SScript::Link(SPin* leftPin, SPin* rightPin)
        {
            Links.push_back(SLink{UGUIDManager::Generate(), leftPin->UID, rightPin->UID});
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
            U32 size = 0;
            size += GetDataSize(Links);

            //U32 branchNodes = 0;
            auto contexts = GetNodeEditorContexts<SBranchNodeEditorContext>();
            size += STATIC_U32(contexts.size());
            //for (auto& [key, val] : NodeEditorContexts)
            //{
            //    if (SBranchNodeEditorContext* context = dynamic_cast<SBranchNodeEditorContext*>(val))
            //        branchNodes++;
            //}

            //for (auto& [key, val] : NodeEditorContexts)
            //{
            //    size += GetDataSize(branchNodes);
            //    size += GetDataSize()
            //}
            // TODO.NW: Maybe move this to nodeserialization file, same for CScene?
            // TODO.NW: Serialize nodes through protocol

            return size;
        }

        void SScript::Serialize(char* toData, U64& pointerPosition) const
        {
            // TODO.NW: Serialize nodes through protocol
            SerializeData(Links, toData, pointerPosition);
        }

        void SScript::Deserialize(const char* fromData, U64& pointerPosition)
        {
            // TODO.NW: Serialize nodes through protocol
            DeserializeData(Links, fromData, pointerPosition);
        }

        SNode* SScript::GetNode(const U64 id) const
        {
            if (!NodeIndices.contains(id))
                return nullptr;

            return Nodes[NodeIndices.at(id)];
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

        //SNodeEditorContext* SScript::GetNodeEditorContext(const U64 nodeID)
        //{
        //    if (!NodeEditorContexts.contains(nodeID))
        //        return nullptr;

        //    return NodeEditorContexts.at(nodeID);
        //}

        SNode::SNode(const U64 id, SScript* owningScript)
            : UID(id)
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

        //SScriptDataBinding::SScriptDataBinding(const U64 id, const std::string& name, const EPinType pinType, const EObjectDataType objectType, const std::variant<PIN_DATA_TYPES>& data)
        //    : UID(id)
        //    , Name(name)
        //    , Type(pinType)
        //    , ObjectType(objectType)
        //    , Data(data)
        //{
        //}
}
}