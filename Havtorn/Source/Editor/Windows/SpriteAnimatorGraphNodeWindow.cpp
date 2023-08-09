// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "SpriteAnimatorGraphNodeWindow.h"
#include "EditorManager.h"

#include "ECS/Components/SpriteAnimatorGraphComponent.h"
#include "ECS/Components/SpriteAnimatorGraphNode.h"

#include <imgui.h>

namespace ImGui
{
	CSpriteAnimatorGraphNodeWindow::CSpriteAnimatorGraphNodeWindow(const char* displayName, Havtorn::CEditorManager* manager)
		: CWindow(displayName, manager, false)
		, Component(nullptr)
	{ }

	void CSpriteAnimatorGraphNodeWindow::OnEnable()
	{ }

	void CSpriteAnimatorGraphNodeWindow::OnDisable()
	{ }

	void CSpriteAnimatorGraphNodeWindow::OnInspectorGUI()
	{
		if (ImGui::Begin(Name(), Open()))
		{
			RecursiveTree(&Component->Graph);
		}
		ImGui::End();
	}

	void CSpriteAnimatorGraphNodeWindow::Inspect(Havtorn::SSpriteAnimatorGraphComponent& component)
	{
		Component = &component;
		Enable(true);
	}

	void CSpriteAnimatorGraphNodeWindow::RecursiveTree(Havtorn::SSpriteAnimatorGraphNode* node)
	{
		if (ImGui::TreeNode(node->Name.Data()))
		{
			if (node->AnimationClipKey == -1)
			{
				for (Havtorn::SSpriteAnimatorGraphNode& childNode : node->Nodes)
					RecursiveTree(&childNode);
			}

			if (node->AnimationClipKey >= 0 && node->AnimationClipKey < Component->AnimationClips.size())
			{
				Havtorn::I16 animationClipKey = node->AnimationClipKey;
				Havtorn::SSpriteAnimationClip& animationClip = Component->AnimationClips[animationClipKey];

				ImGui::Text("Animation Clip Settings");
				ImGui::PushID(node->Name.Data());
				for (Havtorn::U32 i = 0; i < animationClip.UVRects.size(); i++)
				{
					ImGui::PushID(i);

					Havtorn::SVector4& rect = animationClip.UVRects[i];
					Havtorn::F32 uvRect[4] = { rect.X, rect.Y, rect.Z, rect.W };
					if (ImGui::DragFloat4("UVRect", uvRect, UVRectSlideSpeed))
					{
						animationClip.UVRects[i].X = uvRect[0];
						animationClip.UVRects[i].Y = uvRect[1];
						animationClip.UVRects[i].Z = uvRect[2];
						animationClip.UVRects[i].W = uvRect[3];
					}

					Havtorn::F32 duration = animationClip.Durations[i];
					if (ImGui::DragFloat("Duration", &duration, DurationSlideSpeed))
					{
						animationClip.Durations[i] = duration;
					}

					ImGui::PopID();
				}

				ImGui::PopID();
			}

			ImGui::TreePop();
		}
	}
}