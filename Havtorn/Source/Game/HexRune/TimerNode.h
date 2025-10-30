// Copyright 2025 Team Havtorn. All Rights Reserved.


            #pragma once
            #include <HexRune/HexRune.h>

            namespace Havtorn
            {
                namespace HexRune
                {
                    struct STimerNode : public SNode
                    {
                        GAME_API STimerNode(const U64 id, const U32 typeID, SScript* owningScript);
                        virtual GAME_API I8 OnExecute() override;
                    }; 
                }
            }
            