// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "ECS/Systems/SequencerSystem.h"

namespace Havtorn
{
	SSequencerEntityReference* Havtorn::CSequencerAsset::TryGetEntityReference(const U64 guid)
	{
		auto entityReferenceIterator = std::find(EntityReferences.begin(), EntityReferences.end(), guid);
		if (entityReferenceIterator != EntityReferences.end())
			return &(*entityReferenceIterator);

		return nullptr;
	}
}
