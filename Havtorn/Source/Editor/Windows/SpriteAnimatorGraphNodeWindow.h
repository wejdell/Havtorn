// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace Havtorn
{
	struct SSpriteAnimatorGraphComponent;
	struct SSpriteAnimatorGraphNode;
}

namespace ImGui
{
	class CSpriteAnimatorGraphNodeWindow : public CWindow
	{
	public:
		CSpriteAnimatorGraphNodeWindow(const char* displayName, Havtorn::CEditorManager* manager);
		~CSpriteAnimatorGraphNodeWindow() override = default;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;
		void Inspect(Havtorn::SSpriteAnimatorGraphComponent& component);

	private:
		void RecursiveTree(Havtorn::SSpriteAnimatorGraphNode* node);

	private:
		Havtorn::SSpriteAnimatorGraphComponent* Component;
		Havtorn::F32 DurationSlideSpeed = 0.01f;
		Havtorn::F32 UVRectSlideSpeed = 0.01f;
	};
}