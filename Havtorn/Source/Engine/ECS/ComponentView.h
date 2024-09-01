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
		OpenSpriteAnimatorGraph,
	};

	struct HAVTORN_API SComponentViewResult
	{
		EComponentViewResultLabel Label = EComponentViewResultLabel::PassThrough;
		SComponent* ComponentViewed = nullptr;
		U8 ComponentSubIndex = 0;
	};

	struct HAVTORN_API SComponentView
	{
		virtual SComponentViewResult View(const SEntity& /*entityOwner*/, CScene* /*scene*/) { return SComponentViewResult(); };
	};
}
