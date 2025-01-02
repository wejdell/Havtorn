// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct SComponent;
	struct SEntity;
	class CScene;

	enum class HAVTORN_API EComponentViewResultLabel
	{
		PassThrough,
		UpdateTransformGizmo,
		InspectAssetComponent,
		OpenAssetTool,
	};

	struct HAVTORN_API SComponentViewResult
	{
		EComponentViewResultLabel Label = EComponentViewResultLabel::PassThrough;
		SComponent* ComponentViewed = nullptr;
		U8 ComponentSubIndex = 0;
	};

	struct HAVTORN_API SComponentEditorContext
	{
		virtual SComponentViewResult View(const SEntity& /*entityOwner*/, CScene* /*scene*/) const { return SComponentViewResult(); };
		virtual bool AddComponent(const SEntity& /*entity*/, CScene* /*scene*/) const { return false; };
		virtual bool RemoveComponent(const SEntity& /*entity*/, CScene* /*scene*/) const { return false; };
		virtual U8 GetSortingPriority() const { return UMath::MaxU8; };
	};
}
