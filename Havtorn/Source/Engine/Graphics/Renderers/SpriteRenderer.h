// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "SharedRendererData.h"

namespace Havtorn
{
	class CRenderStateManager;
	struct SRenderResource;

	struct SSpriteRenderData
	{
		SRenderResource* TransformBuffer = nullptr;
		SRenderResource* UVRectBuffer = nullptr;
		SRenderResource* ColorBuffer = nullptr;
		SRenderResource* SpriteTexture = nullptr;
		U32 InstanceCount = 0;
	};

	class CSpriteRenderer 
	{
	public:
		CSpriteRenderer(CRenderStateManager* stateManager);
		~CSpriteRenderer() = default;

		void SetupWorldSpaceRender(const SGeometryConstantRenderData& constantData, const SGeometryTargetRenderData& targetData);
		void SetupWorldSpaceRender(const SGeometryConstantRenderData& constantData, const SGeometryTargetRenderData& targetData, const SEditorRenderData& editorPassData);
		void SetupWidgetRender(const SGeometryConstantRenderData& constantData, const SMainTargetRenderData& targetData, const SEditorRenderData& editorPassData);

		// TODO.NW: Add Setup for editor screen space sprites if we ever need it (such as for a 2D game)
		void SetupScreenSpaceRender(const SMainTargetRenderData& targetData);

		void RenderWorldSpaceSprite(const SSpriteRenderData& passData);
		void RenderEditorWorldSpaceSprite(const SSpriteRenderData& passData, const SEditorRenderData& editorPassData);
		void RenderScreenSpaceSprite(const SSpriteRenderData& passData);
		void RenderEditorWorldSpaceWidget(const SSpriteRenderData& passData, const SEditorRenderData& editorPassData);
	
	private:
		U16 ScreenSpaceSpritePSOIndex = 0;
		U16 WorldSpaceSpritePSOIndex = 0;
		U16 WorldSpaceEditorSpritePSOIndex = 0;
		U16 WorldSpaceEditorWidgetPSOIndex = 0;

		CRenderStateManager* RenderStateManager = nullptr;
	};
}
