// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct SComponent;
	struct SAssetReference;
	struct SEntity;
	class CScene;

	enum class EDITOR_API EComponentViewResultLabel
	{
		PassThrough,
		UpdateTransformGizmo,
		RenderPreview,
		InspectAssetComponent,
		OpenAssetTool,
	};

	struct SComponentViewResult
	{
		EComponentViewResultLabel Label = EComponentViewResultLabel::PassThrough;
		SComponent* ComponentViewed = nullptr;
		std::vector<SAssetReference*> AssetReferences;
		EAssetType AssetType = EAssetType::None;
	};

	struct EDITOR_API SComponentView
	{
		friend class CEditorManager;

		virtual SComponentViewResult View(const SEntity& /*entityOwner*/, CScene* /*scene*/) const { return SComponentViewResult(); };
		virtual const char* GetComponentName() const { return "Component"; };
		virtual U8 GetSortingPriority() const { return UMath::MaxU8; };
		U64 GetRuntimeHash() const { return RuntimeHash; }

	private:
		U64 RuntimeHash = 0;
	};
}
