// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "Pin.h"
#include "HexRune.h"

namespace Havtorn
{
    namespace HexRune
    {
        void SPin::ClearData()
        {
            Data = std::monostate();
        }

        void SPin::DeriveInput()
        {
            if (Direction != EPinDirection::Input)
                return;

            if (Type == EPinType::Flow)
                return;

            if (LinkedPin == nullptr)
                return;

            SetDataFromLinkedPin();

            LinkedPin->OwningNode->Execute();

            if (LinkedPin->IsDataUnset())
            {
                HV_LOG_ERROR("Could not derive input even after executing node!");
                return;
            }

            SetDataFromLinkedPin();
        }

        void SPin::SetDataFromLinkedPin()
        {
            if (LinkedPin == nullptr || LinkedPin->IsDataUnset())
                return;

            if (Type != LinkedPin->Type)
                return;

            Data = LinkedPin->Data;
        }

        bool SPin::IsDataUnset() const
        {
            return std::holds_alternative<std::monostate>(Data);
        }

        bool SPin::IsPinTypeLiteral() const
        {
            return Type == EPinType::String || Type == EPinType::Bool || Type == EPinType::Int || Type == EPinType::Float;
        }
    }
}