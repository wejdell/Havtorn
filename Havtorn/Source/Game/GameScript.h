// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include <HexRune\HexRune.h>

namespace Havtorn
{
    struct SGameScript : public HexRune::SScript
    {
    public:
        GAME_API virtual [[nodiscard]] U32 GetSize() const override;
        GAME_API virtual void Serialize(char* toData, U64& pointerPosition) const override;
        GAME_API virtual void Deserialize(const char* fromData, U64& pointerPosition) override;
    };
}

