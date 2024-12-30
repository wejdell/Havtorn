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

	struct SComponentEditorContext
	{
		virtual HAVTORN_API SComponentViewResult View(const SEntity& /*entityOwner*/, CScene* /*scene*/) { return SComponentViewResult(); };
		virtual HAVTORN_API void AddComponent(const SEntity& /*entity*/, CScene* /*scene*/) {};
		virtual HAVTORN_API void RemoveComponent(const SEntity& /*entity*/, CScene* /*scene*/) {};
	};

	struct HAVTORN_API SComponentView
	{
		virtual SComponentViewResult View(const SEntity& /*entityOwner*/, CScene* /*scene*/) { return SComponentViewResult(); };
	};
}
