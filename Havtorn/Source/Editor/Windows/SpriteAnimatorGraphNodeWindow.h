// Copyright 2022 Team Havtorn. All Rights Reserved.

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
		~CSpriteAnimatorGraphNodeWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;
		void Inspect(Havtorn::SSpriteAnimatorGraphComponent& component);

	private:
		void RecursiveTree(Havtorn::SSpriteAnimatorGraphNode* node);


		Havtorn::SSpriteAnimatorGraphComponent* Component;
	};
}