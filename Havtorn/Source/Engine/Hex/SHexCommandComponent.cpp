// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SHexCommandComponent.h"

namespace Havtorn
{
	void SHexCommandComponent::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(Owner, toData, pointerPosition);
		SerializeData(TagsToListenFor, toData, pointerPosition);
	}

	void SHexCommandComponent::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(Owner, fromData, pointerPosition);
		DeserializeData(TagsToListenFor, fromData, pointerPosition);
	}

	U32 SHexCommandComponent::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(Owner);
		size += GetDataSize(TagsToListenFor);

		return size;
	}
}
