// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include <unordered_map>
#include "Pin.h"
#include "Color.h"
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
        struct SNode
        {
            ENGINE_API SNode(const U64 id, const U32 typeID, SScript* owningScript, ENodeType nodeType);
            
            U64 UID = 0;
            U32 TypeID = 0; // NW: TypeID is used to bypass protocol serialization, instead mapping types to serialization functions
            
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

        struct SScriptDataBinding
        {
            U64 UID = 0;
            std::string Name = "";
            EPinType Type = EPinType::Entity;
            // TODO.NW: Figure out how to deal with these subtypes. Maybe list them explicitly as pin data types?
            EObjectDataType ObjectType = EObjectDataType::None;
            EAssetType AssetType = EAssetType::None;
#pragma warning(suppress : 4324)
            std::variant<PIN_DATA_TYPES> Data;

        private:
            void DeserializeDataVariant(std::variant<PIN_DATA_TYPES>& data, const EPinType pinType, const char* fromData, Havtorn::U64& pointerPosition);

            template<typename T>
            void DeserializeVariant(std::variant<PIN_DATA_TYPES>& data, const char* fromData, U64& pointerPosition)
            {
                T value;
                DeserializeData(value, fromData, pointerPosition);
                data = value;
            }
        public:

            ENGINE_API [[nodiscard]] U32 GetSize() const;
            ENGINE_API void Serialize(char* toData, U64& pointerPosition) const;
            ENGINE_API void Deserialize(const char* fromData, U64& pointerPosition);
        };

        struct SInputCallbackBinding
        {
            U64 UID = 0;
            EInputParamType ParamType = EInputParamType::Void;

            ENGINE_API [[nodiscard]] U32 GetSize() const;
            ENGINE_API void Serialize(char* toData, U64& pointerPosition) const;
            ENGINE_API void Deserialize(const char* fromData, U64& pointerPosition);
        };

        struct SScript
        {
            ENGINE_API SScript();
            ENGINE_API virtual ~SScript();

            //Serialize
            std::vector<SNode*> Nodes;
            std::vector<SLink> Links;
            std::vector<SScriptDataBinding> DataBindings;
            std::vector<SInputCallbackBinding> InputCallbackDataBindings;
            //-------
            
            struct SNodeFactory* NodeFactory = nullptr;

            // NW: Mapping UID to node
            std::unordered_map<U64, U64> NodeIndices;
            std::unordered_map<U64, U64> NodeIDToRuntimeHash;

            CScene* Scene = nullptr;
            std::string Name = "";

            // TODO.NW: Input params to the script (with connection to owning entity or instance properties) should be loaded from the corresponding component?

            template<typename T>
            T* AddNode(U64 uid, U32 typeID)
            {
                if (uid == 0)
                    uid = UGUIDManager::Generate();

                NodeIndices.emplace(uid, Nodes.size());
                NodeIDToRuntimeHash.emplace(uid, typeid(T).hash_code());
                Nodes.emplace_back(new T(uid, typeID, this));
                
                SNode* node = Nodes.back();
                return dynamic_cast<T*>(node);
            }

            template<typename T>
            T* AddDataBindingNode(U64 uid, U32 typeID, const U64 databindingID)
            {
                if (uid == 0)
                    uid = UGUIDManager::Generate();

                NodeIndices.emplace(uid, Nodes.size());
                NodeIDToRuntimeHash.emplace(uid, typeid(T).hash_code() + databindingID);
                Nodes.emplace_back(new T(uid, typeID, this, databindingID));

                SNode* node = Nodes.back();
                return dynamic_cast<T*>(node);
            }

            template<typename T>
            T* GetNode(const U64 id) const
            {
                if (!NodeIndices.contains(id))
                    return nullptr;

                return dynamic_cast<T*>(Nodes[NodeIndices.at(id)]);
            }

            // TODO.NW: Deal with serialization?
            ENGINE_API const SScriptDataBinding& AddDataBinding(const char* name, const EPinType type, const EObjectDataType objectType, const EAssetType assetType);
            ENGINE_API const SScriptDataBinding& AddDataBinding(const SScriptDataBinding& dataCopy);
            ENGINE_API void RemoveDataBinding(const U64 id);
            ENGINE_API void RemoveNode(const U64 id);

            ENGINE_API virtual void Init();

            ENGINE_API void TraverseFromNode(const U64 startNodeID, CScene* owningScene);
            ENGINE_API void TraverseFromNode(SNode* startNode, CScene* owningScene);

            ENGINE_API void Link(U64 leftPinID, U64 rightPinID);
            ENGINE_API void Link(SPin* leftPin, SPin* rightPin);
            ENGINE_API void LinkSerialized();
            ENGINE_API void Unlink(U64 leftPinID, U64 rightPinID);
            ENGINE_API void Unlink(SPin* leftPin, SPin* rightPin);

            ENGINE_API void SetDataOnInput(U64 pinID, const std::variant<PIN_DATA_TYPES>& data);

            ENGINE_API SNode* GetNode(const U64 id) const;
            ENGINE_API bool HasNode(const U64 id) const;

            ENGINE_API virtual [[nodiscard]] U32 GetSize() const;
            ENGINE_API virtual void Serialize(char* toData, U64& pointerPosition) const;
            ENGINE_API virtual void Deserialize(const char* fromData, U64& pointerPosition);
        };

        struct SNodeFactory
        {
            template<typename TNode>
            void RegisterNodeType(U32 typeID)
            {
                RuntimeHashToTypeID.emplace(typeid(TNode).hash_code(), typeID);

                BasicNodeFactoryMap[typeID] =
                    [](U64 id, U32 nodeTypeID, SScript* script)
                    {
                        return script->AddNode<TNode>(id, nodeTypeID);
                    };
            }

            template<typename TNode>
            void RegisterDatabindingNode(U32 typeID, const U64 databindingID)
            {
                RuntimeHashToTypeID.emplace(typeid(TNode).hash_code() + databindingID, typeID);

                DatabindingNodeFactoryMap[typeID] =
                    [](U64 id, U32 nodeTypeID, SScript* script, const U64 databindingID)
                    {
                        return script->AddDataBindingNode<TNode>(id, nodeTypeID, databindingID);
                    };
            }

            template<typename TNode>
            void RemoveDatabindingNode(const U64 databindingID)
            {
                const U32 typeID = RuntimeHashToTypeID.at(typeid(TNode).hash_code() + databindingID);
                RuntimeHashToTypeID.erase(typeid(TNode).hash_code() + databindingID);

                DatabindingNodeFactoryMap.erase(typeID);
            }

            ENGINE_API SNode* CreateNode(U32 typeID, U64 id, SScript* script);
            ENGINE_API SNode* CreateNode(U32 typeID, U64 id, SScript* script, const U64 databindingId);
            ENGINE_API SNode* CreateNode(U64 runtimeHash, U64 id, SScript* script);
            ENGINE_API SNode* CreateNode(U64 runtimeHash, U64 id, SScript* script, const U64 databindingId);

        private:
            std::unordered_map<U32, std::function<SNode*(const U64, const U32, SScript*)>> BasicNodeFactoryMap;
            std::unordered_map<U32, std::function<SNode*(const U64, const U32, SScript*, const U64)>> DatabindingNodeFactoryMap;
            std::unordered_map<U64, U32> RuntimeHashToTypeID;
        };
	}
}
