// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace Havtorn
{
	struct SSpriteAnimatorGraphComponent;
	struct SSpriteAnimatorGraphNode;

	class CSpriteAnimatorGraphNodeWindow : public CWindow
	{
	public:
		CSpriteAnimatorGraphNodeWindow(const char* displayName, CEditorManager* manager);
		~CSpriteAnimatorGraphNodeWindow() override = default;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;
		void Inspect(SSpriteAnimatorGraphComponent& component);

	private:
		void RecursiveTree(SSpriteAnimatorGraphNode* node);

	private:
		SSpriteAnimatorGraphComponent* Component;
		F32 DurationSlideSpeed = 0.01f;
		F32 UVRectSlideSpeed = 0.01f;
	};
}