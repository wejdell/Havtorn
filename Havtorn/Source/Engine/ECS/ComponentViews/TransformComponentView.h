// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/ComponentView.h"

namespace Havtorn
{
	struct STransformComponentView : public SComponentEditorContext
	{
		HAVTORN_API SComponentViewResult View(const SEntity& entityOwner, CScene* scene) override;
		HAVTORN_API void AddComponent(const SEntity& entity, CScene* scene) override;
		HAVTORN_API void RemoveComponent(const SEntity& entity, CScene* scene) override;

		static STransformComponentView Context;
	};
}
