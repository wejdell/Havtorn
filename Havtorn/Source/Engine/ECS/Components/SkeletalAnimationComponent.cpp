// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "SkeletalAnimationComponent.h"

namespace Havtorn
{
    U32 SSkeletalAnimationComponent::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(Owner);

        return size;
    }

    void SSkeletalAnimationComponent::Serialize(char* toData, U64& pointerPosition) const
    {
        SerializeData(Owner, toData, pointerPosition);
    }

    void SSkeletalAnimationComponent::Deserialize(const char* fromData, U64& pointerPosition)
    {
        DeserializeData(Owner, fromData, pointerPosition);
    }
}
