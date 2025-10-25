// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include <HexRune\HexRune.h>

namespace Havtorn
{
    struct SGameScript : public HexRune::SScript
    {
    public:
        GAME_API SGameScript();
        GAME_API ~SGameScript() override;
        GAME_API void InitializeGame(U32 typeId) override;
    };
}

