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
        SerializeData(FrameNumber, toData, pointerPosition);
        SerializeData(ShouldBlendLeft, toData, pointerPosition);
        SerializeData(ShouldBlendRight, toData, pointerPosition);
        SerializeData(ShouldRecord, toData, pointerPosition);
    }

    void Havtorn::SSequencerKeyframe::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(FrameNumber, fromData, pointerPosition);
        DeserializeData(ShouldBlendLeft, fromData, pointerPosition);
        DeserializeData(ShouldBlendRight, fromData, pointerPosition);
        DeserializeData(ShouldRecord, fromData, pointerPosition);
    }
}
