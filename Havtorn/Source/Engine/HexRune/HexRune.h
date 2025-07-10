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
        struct SScript;

        enum class ENodeType
        {
            Execution,
            Simple,
            Tree,
            Comment,
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
            SNode(const U64 id, SScript* owningScript);

            U64 UID = 0;
            //std::string Name = "";
            std::vector<SPin> Inputs;
            std::vector<SPin> Outputs;
            ENodeType Type = ENodeType::Execution;
            SScript* OwningScript = nullptr;

            // TODO.NW: Serialization

            // TODO.NW: Could potentially move this to NodeEditorContexts, that map these values to proper node IDs
            //SVector2<F32> EditorPosition = SVector2<F32>();

            ENGINE_API SPin& AddInput(const U64 id, const EPinType type, const std::string& name = "");
            ENGINE_API SPin& AddOutput(const U64 id, const EPinType type, const std::string& name = "");

            ENGINE_API void Execute();

            // Return output index to continue with, if not all
            ENGINE_API virtual I8 OnExecute(); 
            ENGINE_API virtual bool IsStartNode() const { return false; }

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
            EPinType Type = EPinType::Object;
            EObjectDataType ObjectType = EObjectDataType::None;
            std::variant<PIN_DATA_TYPES> Data;
        };

        // TODO.NW: Should be basis for runtime asset? May want script components
        // that have their own editor tool (window) where BP/visual script is shown.
        // An engine script system can deal with starting and running these scripts.
        // This struct should mimic a System proper though, deal with its own execution
        // and so on.
        struct SScript
        {
            ENGINE_API SScript();

            std::vector<SNode*> Nodes;
            std::vector<SLink> Links;
            //std::unordered_map<U64, SNodeEditorContext*> NodeEditorContexts;

            std::unordered_map<U64, U64> ContextTypeToStorageIndices;
            std::vector<SNodeContextStorage> ContextStorages;

            // NW: Mapping UID to node
            std::unordered_map<U64, U64> NodeIndices;
            std::vector<SNode*> StartNodes;
            
            std::vector<SNodeEditorContext*> RegisteredEditorContexts;
            std::vector<SScriptDataBinding> DataBindings;

            CScene* Scene = nullptr;
            std::string FileName = "";

            // TODO.NW: Input params to the script (with connection to owning entity or instance properties) should be loaded from the corresponding component?

            template<typename T, typename... Params>
            T* AddNode(U64 id, Params... params)
            {
                if (id == 0)
                    id = UGUIDManager::Generate();

                NodeIndices.emplace(id, Nodes.size());
                Nodes.emplace_back(new T(id, this, params...));
                
                SNode* node = Nodes.back();
                if (node->IsStartNode())
                    StartNodes.push_back(Nodes.back());

                //node->UID = id;
                //node->OwningScript = this;
                //node->Construct();

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
            ENGINE_API void AddDataBinding(const char* name, const EPinType type, const EObjectDataType objectType = EObjectDataType::None);
            ENGINE_API void RemoveDataBinding(const U64 id);
            ENGINE_API void RemoveNode(const U64 id);

            ENGINE_API void Initialize();
            ENGINE_API void TraverseScript(CScene* owningScene);

            ENGINE_API void Link(U64 leftPinID, U64 rightPinID);
            ENGINE_API void Link(SPin* leftPin, SPin* rightPin);
            ENGINE_API void Unlink(U64 leftPinID, U64 rightPinID);
            ENGINE_API void Unlink(SPin* leftPin, SPin* rightPin);

            ENGINE_API void SetDataOnInput(U64 pinID, const std::variant<PIN_DATA_TYPES>& data);

            // TODO.NW: Make explicit getters for standard entry points? BeginPlay, Tick, EndPlay.
            // Allow only one per script, and keep a separate pointer to them
            ENGINE_API SNode* GetNode(const U64 id) const;

            ENGINE_API [[nodiscard]] U32 GetSize() const;
            ENGINE_API void Serialize(char* toData, U64& pointerPosition) const;
            ENGINE_API void Deserialize(const char* fromData, U64& pointerPosition);
            
            ENGINE_API void RemoveContext(const U64 nodeID);
            ENGINE_API SNodeEditorContext* GetNodeEditorContext(const U64 nodeID) const;
        };
	}
}