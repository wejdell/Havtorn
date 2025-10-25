// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include <variant>
#include "ECS/Entity.h"
#include "ECS/Component.h"

#define PIN_DATA_TYPES PIN_LITERAL_TYPES, PIN_MATH_TYPES, Havtorn::SEntity, Havtorn::SComponent*, std::vector<Havtorn::SEntity>, std::vector<Havtorn::SComponent*>

namespace Havtorn
{
	namespace HexRune
	{
        // TODO.NW: Add functional types, like add node or advanced section?
        // TODO.NW: Figure out enum support
        enum class ENGINE_API EPinType : U8
        {
            Unknown,
            Bool,
            Int,
            Float,
            String,
            Vector,
            Matrix,
            Quaternion,
            Entity,
            ComponentPtr,
            Asset,
            EntityList,
            ComponentPtrList,

            //Other stuff
            Delegate,
            Function,
            Flow
        };

        enum class EPinDirection
        {
            Input,
            Output
        };

        struct SNode;

        // TODO.NW: Figure out convertible pins
        struct SPin
        {
            U64 UID = 0;
            std::string Name = "";
            EPinType Type = EPinType::Flow;
            EPinDirection Direction = EPinDirection::Input;
            SNode* OwningNode = nullptr;
            SPin* LinkedPin = nullptr;
#pragma warning(suppress : 4324)
            std::variant<PIN_DATA_TYPES> Data;
            U64 DataSize = 0;

            void ClearData();
            void DeriveInput();
            void SetDataFromLinkedPin();
            bool IsDataUnset() const;
            ENGINE_API bool IsPinTypeLiteral() const;
        };

        // TODO.NW: Put in HEX utility or something
        template<typename T>
        std::vector<T*> CastComponents(const std::vector<SComponent*>& baseComponents)
        {
            if (baseComponents.empty())
                return {};

            std::vector<T*> specializedComponents;
            specializedComponents.resize(baseComponents.size());
            memcpy(&specializedComponents[0], baseComponents.data(), sizeof(T*) * baseComponents.size());
            
            return specializedComponents;
        }
	}
}