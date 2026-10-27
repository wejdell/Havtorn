// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <GUI.h>

namespace Havtorn
{
	class IRenderPass;
	class CRenderResourceRegistry;
	struct SRenderResource;

	class CRenderDebugWindow : public CWindow
	{
	public:
		CRenderDebugWindow(const char* displayName, CEditorManager* manager);
		~CRenderDebugWindow() override = default;

		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		void DrawRegistry() const;
		void DrawGraph() const;
		void DrawPass(const Ref<IRenderPass>& pass, CRenderResourceRegistry& registry, const bool isCulled) const;

		void PlaceNodes() const;
		std::string ResolveLabel(const U64& renderViewID) const;
		SVector2<F32> GetNodeSize(const Ref<IRenderPass>& pass, CRenderResourceRegistry& registry) const;
		SColor GetResourceColor(const SRenderResource* renderResource) const;

	private:
		CRenderManager* RenderManager = nullptr;
		SGuiTextFilter Filter;
		U64 CurrentViewID = 0;
		bool IsHoveringWindow = false;

		const F32 HeaderHeight = 12.0f;
		const F32 PinNameOffset = 4.0f;
	};
}
