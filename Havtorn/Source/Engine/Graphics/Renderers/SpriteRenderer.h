// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

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

	struct SEditorSpriteRenderData
	{
		SRenderResource* EntityBuffer = nullptr;
	};

	class CSpriteRenderer 
	{
	public:
		CSpriteRenderer(CRenderStateManager* stateManager);
		~CSpriteRenderer() = default;

		void RenderScreenSpaceSprite(const SSpriteRenderData& passData);
		void RenderWorldSpaceSprite(const SSpriteRenderData& passData);
		void RenderEditorWorldSpaceSprite(const SSpriteRenderData& passData, const SEditorSpriteRenderData& editorPassData);
		void RenderEditorWorldSpaceWidget(const SSpriteRenderData& passData, const SEditorSpriteRenderData& editorPassData);
	
	private:
		U16 ScreenSpaceSpritePSOIndex = 0;
		U16 WorldSpaceSpritePSOIndex = 0;
		U16 WorldSpaceEditorSpritePSOIndex = 0;
		U16 WorldSpaceEditorWidgetPSOIndex = 0;

		CRenderStateManager* RenderStateManager = nullptr;
	};
}
