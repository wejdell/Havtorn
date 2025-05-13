// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "TransformComponent.h"
#include "Scene/Scene.h"

#include <ranges>

namespace Havtorn
{
	void STransformComponent::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(Owner, toData, pointerPosition);

		SMatrix nakedMatrix = Transform.GetMatrix();
		SerializeData(nakedMatrix, toData, pointerPosition);
		
		SerializeData(ParentEntity, toData, pointerPosition);
		SerializeData(AttachedEntities, toData, pointerPosition);
	}

	void STransformComponent::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(Owner, fromData, pointerPosition);
		
		// NW: Attachment is set up in deserialize post step, when all entities are alive
		SMatrix nakedMatrix = SMatrix::Identity;
		DeserializeData(nakedMatrix, fromData, pointerPosition);
		Transform.SetMatrix(nakedMatrix);
		
		DeserializeData(ParentEntity, fromData, pointerPosition);
		DeserializeData(AttachedEntities, fromData, pointerPosition);
	}

	U32 STransformComponent::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(Owner);
		size += GetDataSize(SMatrix::Identity);
		size += GetDataSize(ParentEntity);
		size += GetDataSize(AttachedEntities);
		return size;
	}

	void STransformComponent::IsDeleted(CScene* fromScene)
	{
		if (!ParentEntity.IsValid())
			return;

		STransformComponent* parentComponent = fromScene->GetComponent<STransformComponent>(ParentEntity);
		parentComponent->Detach(this);
	}

	void STransformComponent::Attach(STransformComponent* child)
	{
		child->Transform.SetParent(&Transform);
		child->ParentEntity = Owner;

		if (auto it = std::ranges::find(AttachedEntities, child->Owner); it == AttachedEntities.end())
			AttachedEntities.emplace_back(child->Owner);
		Transform.AddAttachment(&child->Transform);
	}

	void STransformComponent::Detach(STransformComponent* child)
	{
		child->Transform.SetParent(nullptr);
		child->ParentEntity = SEntity::Null;

		if (auto it = std::ranges::find(AttachedEntities, child->Owner); it != AttachedEntities.end())
			AttachedEntities.erase(it);
		Transform.RemoveAttachment(&child->Transform);
	}
}
