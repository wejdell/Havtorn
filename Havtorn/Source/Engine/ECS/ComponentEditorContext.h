// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct SComponent;
	struct SAssetReference;
	struct SEntity;
	class CScene;

	enum class ENGINE_API EComponentViewResultLabel
	{
		PassThrough,
		UpdateTransformGizmo,
		RenderPreview,
		InspectAssetComponent,
		OpenAssetTool,
	};

	struct ENGINE_API SComponentViewResult
	{
		EComponentViewResultLabel Label = EComponentViewResultLabel::PassThrough;
		SComponent* ComponentViewed = nullptr;
		SAssetReference* AssetReference = nullptr;
		std::vector<SAssetReference>* AssetReferences = nullptr;
		EAssetType AssetType = EAssetType::None;
	};

	struct ENGINE_API SComponentEditorContext
	{
		virtual SComponentViewResult View(const SEntity& /*entityOwner*/, CScene* /*scene*/) const { return SComponentViewResult(); };
		virtual bool AddComponent(const SEntity& /*entity*/, CScene* /*scene*/) const { return false; };
		virtual bool RemoveComponent(const SEntity& /*entity*/, CScene* /*scene*/) const { return false; };
		virtual U8 GetSortingPriority() const { return UMath::MaxU8; };
	};
}
