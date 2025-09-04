// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include <unordered_map>
#include "Pin.h"
#include "Color.h"
#include "NodeEditorContext.h"
#include "ECS/GUIDManager.h"

namespace Havtorn
{
    class CScene;

	namespace HexRune
	{
        constexpr U64 BeginPlayNodeID = 1;
        constexpr U64 TickNodeID = 2;
        constexpr U64 EndPlayNodeID = 3;
        constexpr U64 OnBeginOverlapNodeID = 4;
        constexpr U64 OnEndOverlapNodeID = 5;

        struct SScript;

        enum class EFlowType
        {
            Execution,
            Simple,
            Tree,
            Comment,
        };

        enum class ENodeType
        {
            Standard,
            DataBindingGetNode,
            DataBindingSetNode,
            None
        };

        enum class ENGINE_API EObjectDataType : U8
        {
            None,
            Entity,
            Component
        };

        // TODO.NW: Figure out conversions between similar node types
        // TODO.NW: Add minimum node width?
        struct SNode
        {
            SNode(const U64 id, const U32 typeID, SScript* owningScript, ENodeType nodeType);
            
            U64 UID = 0;
            U32 TypeID = 0;
            
            EFlowType FlowType = EFlowType::Execution;
            ENodeType NodeType = ENodeType::Standard;

            std::vector<SPin> Inputs;
            std::vector<SPin> Outputs;
            
            SScript* OwningScript = nullptr;

            ENGINE_API SPin& AddInput(const U64 id, const EPinType type, const std::string& name = "");
            ENGINE_API SPin& AddOutput(const U64 id, const EPinType type, const std::string& name = "");

            ENGINE_API void Execute();

            // Return output index to continue with, if not all
            ENGINE_API virtual I8 OnExecute(); 

            template<typename T>
            void SetDataOnPin(SPin* pin, const T& data);
            template<typename T>
            void SetDataOnPin(const EPinDirection direction, const U64 pinIndex, const T& data);

            template<typename T>
            void GetDataOnPin(SPin* pin, T& destination);
            template<typename T>
            void GetDataOnPin(const EPinDirection direction, const U64 pinIndex, T& destination);
        };

        template<typename T>
        void SNode::SetDataOnPin(SPin* pin, const T& data)
        {
            pin->Data = data;
        }

        template<typename T>
        void SNode::GetDataOnPin(SPin* pin, T& destination)
        {
            if (pin == nullptr || !std::holds_alternative<T>(pin->Data))
                return;

            destination = std::get<T>(pin->Data);
        }

        // TODO.NW: Make GetDataOnPin<T, U> that can directly cast the result, U will not be inferred

        template<typename T>
        inline void SNode::SetDataOnPin(const EPinDirection direction, const U64 pinIndex, const T& data)
        {
            std::vector<SPin>& pins = direction == EPinDirection::Input ? Inputs : Outputs;
            if (!UMath::IsWithin(pinIndex, STATIC_U64(0), pins.size()))
                return;

            SetDataOnPin(&pins[pinIndex], data);
        }

        template<typename T>
        inline void SNode::GetDataOnPin(const EPinDirection direction, const U64 pinIndex, T& destination)
        {
            std::vector<SPin>& pins = direction == EPinDirection::Input ? Inputs : Outputs;
            if (!UMath::IsWithin(pinIndex, STATIC_U64(0), pins.size()))
                return;

            GetDataOnPin(&pins[pinIndex], destination);
        }

        struct SLink
        {
            U64 UID = 0;
            U64 StartPinUID = 0;
            U64 EndPinUID = 0;

            SLink() = default;
            SLink(const U64 id, const U64 startPinId, const U64 endPinId) 
                : UID(id)
                , StartPinUID(startPinId)
                , EndPinUID(endPinId)
            {
            }
        };

        struct SNodeContextStorage
        {
            std::unordered_map<U64, U64> NodeIDToContextIndices;
            std::vector<SNodeEditorContext*> Contexts;
        };

        struct SScriptDataBinding
        {
            U64 UID = 0;
            std::string Name = "";
            EPinType Type = EPinType::Entity;
            // TODO.NW: Figure out how to deal with these subtypes. Maybe list them explicitly as pin data types?
            EObjectDataType ObjectType = EObjectDataType::None;
            EAssetType AssetType = EAssetType::None;
            std::variant<PIN_DATA_TYPES> Data;

            ENGINE_API [[nodiscard]] U32 GetSize() const;
            ENGINE_API void Serialize(char* toData, U64& pointerPosition) const;
            ENGINE_API void Deserialize(const char* fromData, U64& pointerPosition);
        };

        struct SScript
        {
            ENGINE_API SScript();

            //Serialize
            std::vector<SNode*> Nodes;
            std::vector<SLink> Links;
            std::vector<SScriptDataBinding> DataBindings;
            std::vector<SNodeEditorContext*> RegisteredEditorContexts;
            //-------
            
            struct SNodeFactory* NodeFactory;
            //std::unordered_map<U64, SNodeEditorContext*> NodeEditorContexts;

            std::unordered_map<U64, U64> ContextTypeToStorageIndices;
            std::vector<SNodeContextStorage> ContextStorages;

            // NW: Mapping UID to node
            std::unordered_map<U64, U64> NodeIndices;
            
            CScene* Scene = nullptr;
            std::string FileName = "";

            // TODO.NW: Input params to the script (with connection to owning entity or instance properties) should be loaded from the corresponding component?

            template<typename T, typename... Params>
            T* AddNode(U64 id, const U32 typeID, Params... params)
            {
                if (id == 0)
                    id = UGUIDManager::Generate();

                NodeIndices.emplace(id, Nodes.size());
                Nodes.emplace_back(new T(id, typeID, this, params...));
                
                SNode* node = Nodes.back();
                return dynamic_cast<T*>(node);
            }

            template<typename T, typename... Params>
            T* AddEditorContext(const U64 nodeID, Params... params)
            {
                const U64 typeIDHashCode = typeid(T).hash_code();
                if (!ContextTypeToStorageIndices.contains(typeIDHashCode))
                {
                    ContextTypeToStorageIndices.emplace(typeIDHashCode, ContextStorages.size());
                    ContextStorages.emplace_back();
                }

                SNodeContextStorage& contextStorage = ContextStorages[ContextTypeToStorageIndices.at(typeIDHashCode)];

                if (contextStorage.NodeIDToContextIndices.contains(nodeID))
                {
                    *(dynamic_cast<T*>(contextStorage.Contexts[contextStorage.NodeIDToContextIndices.at(nodeID)])) = T(params...);
                }
                else
                {
                    contextStorage.NodeIDToContextIndices.emplace(nodeID, contextStorage.Contexts.size());
                    contextStorage.Contexts.emplace_back(new T(params...));
                }

                return dynamic_cast<T*>(contextStorage.Contexts.back());
            }

            template<typename T>
            void RemoveContext(const U64 nodeID)
            {
                const U64 typeIDHashCode = typeid(T).hash_code();
                if (!ContextTypeToStorageIndices.contains(typeIDHashCode))
                    return;
                

                SNodeContextStorage& contextStorage = ContextStorages[ContextTypeToStorageIndices.at(typeIDHashCode)];
                std::unordered_map<U64, U64>& contextIndices = contextStorage.NodeIDToContextIndices;
                std::vector<SNodeEditorContext*>& contexts = contextStorage.Contexts;

                if (!contextIndices.contains(nodeID))
                    return;

                const std::pair<U64, U64>& maxIndexEntry = *std::ranges::find_if(contextIndices,
                    [contexts](const auto& entry) { return entry.second == contexts.size() - 1; });

                std::swap(contexts[contextIndices.at(nodeID)], contexts[contextIndices.at(maxIndexEntry.first)]);
                std::swap(contextIndices.at(maxIndexEntry.first), contextIndices.at(nodeID));

                T* contextToBeRemoved = reinterpret_cast<T*>(contexts[contextIndices.at(nodeID)]);
                delete contextToBeRemoved;
                contextToBeRemoved = nullptr;

                contexts.erase(contexts.begin() + contextIndices.at(nodeID));
                contextIndices.erase(nodeID);
            }

            template<typename T>
            T* GetNode(const U64 id) const
            {
                if (!NodeIndices.contains(id))
                    return nullptr;

                return dynamic_cast<T*>(Nodes[NodeIndices.at(id)]);
            }

            template<typename T>
            T* GetNodeEditorContext(const U64 nodeID) const
            {
                const U64 typeIDHashCode = typeid(T).hash_code();
                if (!ContextTypeToStorageIndices.contains(typeIDHashCode))
                    return nullptr;

                const SNodeContextStorage& contextStorage = ContextStorages[ContextTypeToStorageIndices.at(typeIDHashCode)];

                if (!contextStorage.NodeIDToContextIndices.contains(nodeID))
                    return nullptr;

                U64 index = contextStorage.NodeIDToContextIndices.at(nodeID);
                return dynamic_cast<T*>(contextStorage.Contexts[index]);
            }

            template<typename T>
            std::vector<T*> GetNodeEditorContexts() const
            {
                const U64 typeIDHashCode = typeid(T).hash_code();
                if (!ContextTypeToStorageIndices.contains(typeIDHashCode))
                    return {};

                const SNodeContextStorage& contextStorage = ContextStorages[ContextTypeToStorageIndices.at(typeIDHashCode)];
                if (contextStorage.Contexts.empty())
                    return {};

                std::vector<T*> specializedContexts;
                specializedContexts.resize(contextStorage.Contexts.size());
                memcpy(&specializedContexts[0], contextStorage.Contexts.data(), sizeof(T*) * contextStorage.Contexts.size());

                return specializedContexts;
            }

            // TODO.NW: Deal with serialization?
            ENGINE_API void AddDataBinding(const char* name, const EPinType type, const EObjectDataType objectType, const EAssetType assetType);
            ENGINE_API void RemoveDataBinding(const U64 id);
            ENGINE_API void RemoveNode(const U64 id);

            ENGINE_API void Initialize();
            ENGINE_API void TraverseFromNode(const U64 startNodeID, CScene* owningScene);
            ENGINE_API void TraverseFromNode(SNode* startNode, CScene* owningScene);

            ENGINE_API void Link(U64 leftPinID, U64 rightPinID);
            ENGINE_API void Link(SPin* leftPin, SPin* rightPin);
            ENGINE_API void Unlink(U64 leftPinID, U64 rightPinID);
            ENGINE_API void Unlink(SPin* leftPin, SPin* rightPin);

            ENGINE_API void SetDataOnInput(U64 pinID, const std::variant<PIN_DATA_TYPES>& data);

            ENGINE_API SNode* GetNode(const U64 id) const;
            ENGINE_API bool HasNode(const U64 id) const;

            ENGINE_API void RemoveContext(const U64 nodeID);
            ENGINE_API SNodeEditorContext* GetNodeEditorContext(const U64 nodeID) const;
            
            ENGINE_API virtual [[nodiscard]] U32 GetSize() const;
            ENGINE_API virtual void Serialize(char* toData, U64& pointerPosition) const;
            ENGINE_API virtual void Deserialize(const char* fromData, U64& pointerPosition);
        };

        struct SNodeFactory
        {
            template<typename TNode, typename TNodeEditorContext>
            void RegisterNodeType(SScript* script, U32 typeID)
            {
                script->RegisteredEditorContexts.emplace_back(&TNodeEditorContext::Context);
                script->RegisteredEditorContexts.back()->TypeID = typeID;

                BasicNodeFactoryMap[typeID] =
                    [](U64 id, const U32 typeID, SScript* script)
                    {
                        TNode* node = script->AddNode<TNode>(id, typeID);
                        auto context = script->AddEditorContext<TNodeEditorContext>(id);
                        context->TypeID = typeID;
                        return node;
                    };
            }

            template<typename TNode, typename TNodeEditorContext>
            void RegisterDatabindingNode(U32 typeID)
            {
                DatabindingNodeFactoryMap[typeID] =
                    [](U64 id, const U32 typeID, SScript* script, const U64 databindingId)
                    {
                        TNode* node = script->AddNode<TNode>(id, typeID, databindingId);
                        auto context = script->AddEditorContext<TNodeEditorContext>(id, script, databindingId);
                        context->TypeID = typeID;
                        return node;
                    };
            }

            SNode* CreateNode(U32 typeID, U64 id, SScript* script);
            SNode* CreateNode(U32 typeID, U64 id, SScript* script, const U64 databindingId);

        private:
            std::unordered_map<U32, std::function<SNode* (const U64, const U32, SScript*)>> BasicNodeFactoryMap;
            std::unordered_map<U32, std::function<SNode* (const U64, const U32, SScript*, const U64)>> DatabindingNodeFactoryMap;
        };

	}
}