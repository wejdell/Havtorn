// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SequencerKeyframe.h"

namespace Havtorn
{
    U32 Havtorn::SSequencerKeyframe::GetSize() const
    {
        U32 size = 0;
        size += sizeof(U32);
        size += sizeof(bool) * 3;
        return size;
    }

    void Havtorn::SSequencerKeyframe::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeSimple(FrameNumber, toData, pointerPosition);
        SerializeSimple(ShouldBlendLeft, toData, pointerPosition);
        SerializeSimple(ShouldBlendRight, toData, pointerPosition);
        SerializeSimple(ShouldRecord, toData, pointerPosition);
    }

    void Havtorn::SSequencerKeyframe::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeSimple(FrameNumber, fromData, pointerPosition);
        DeserializeSimple(ShouldBlendLeft, fromData, pointerPosition);
        DeserializeSimple(ShouldBlendRight, fromData, pointerPosition);
        DeserializeSimple(ShouldRecord, fromData, pointerPosition);
    }
}
