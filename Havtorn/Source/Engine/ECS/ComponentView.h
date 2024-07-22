// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

//#include "Entity.h"
//#include "Scene/Scene.h"

namespace Havtorn
{
	struct SComponent;
	struct SEntity;
	class CScene;

	enum class EComponentViewResultLabel
	{
		PassThrough,
		UpdateCameraGizmo,
		OpenMeshAssetModal,
		OpenTextureAssetModal,
		OpenMaterialAssetModal,
		OpenSpriteAnimatorGraph,
	};

	struct SComponentViewResult
	{
		EComponentViewResultLabel Label = EComponentViewResultLabel::PassThrough;
		SComponent* ComponentViewed = nullptr;
		U8 ComponentSubIndex = 0;
	};

	struct SComponentView
	{
		virtual SComponentViewResult View(const SEntity& /*entityOwner*/, CScene* /*scene*/) {};
	};
}
