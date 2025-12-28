// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "UICanvasComponent.h"

namespace Havtorn 
{
	void SUIElement::Serialize(char* /*toData*/, U64& /*pointerPosition*/) const
	{
	}

	void SUIElement::Deserialize(const char* /*fromData*/, U64& /*pointerPosition*/)
	{
	}

	U32 SUIElement::GetSize() const
	{
		U32 size = 0;


		return size;
	}

	void SUICanvasComponent::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(Owner, toData, pointerPosition);

		SerializeData(static_cast<U32>(Elements.size()), toData, pointerPosition);
		for (const SUIElement& element : Elements)
			element.Serialize(toData, pointerPosition);
	}

	void SUICanvasComponent::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(Owner, fromData, pointerPosition);

		U32 elementsCount = 0;
		DeserializeData(elementsCount, fromData, pointerPosition);
		for (U32 i = 0; i < elementsCount; i++)
		{
			SUIElement element = {};
			element.Deserialize(fromData, pointerPosition);
			Elements.emplace_back(element);
		}
	}

	U32 SUICanvasComponent::GetSize() const
	{
		U32 size = 0;

		size += GetDataSize(Owner);

		size += sizeof(U32);
		for (const SUIElement& element : Elements)
			size += element.GetSize();

		return size;
	}

	void SUICanvasComponent::IsDeleted(CScene* /*fromScene*/)
	{
		for (const SUIElement& element : Elements)
		{
			for (const SAssetReference& assetReference : element.StateAssetReferences)
				GEngine::GetAssetRegistry()->UnrequestAsset(assetReference, Owner.GUID);
		}
	}
}
