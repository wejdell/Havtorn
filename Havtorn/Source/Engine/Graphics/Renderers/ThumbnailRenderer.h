// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class CRenderStateManager;
	struct SRenderResource;

	struct SStaticMeshThumbnailPassData
	{
		SRenderResource* ObjectBuffer = nullptr;
		SRenderResource* FrameBuffer = nullptr;
		SRenderResource* VertexBuffer = nullptr;
		SRenderResource* IndexBuffer = nullptr;
		U32 IndexCount = 0;
	};

	struct SSkeletalMeshThumbnailPassData
	{
		SRenderResource* ObjectBuffer = nullptr;
		SRenderResource* FrameBuffer = nullptr;
		SRenderResource* VertexBuffer = nullptr;
		SRenderResource* IndexBuffer = nullptr;
		SRenderResource* BoneBuffer = nullptr;
		SRenderResource* AnimDataBuffer = nullptr; // TODO.NW: This should probably be removed 
		SRenderResource* TransformBuffer = nullptr; // TODO.NW: This is not needed for thumbnail rendering. Would need a new input layout and vertex shader input to  remove it
		U32 IndexCount = 0;
	};

	struct SCubemapTexturePassData
	{
		SRenderResource* TransformBuffer = nullptr;
		SRenderResource* UVRectBuffer = nullptr;
		SRenderResource* ColorBuffer = nullptr;
		SRenderResource* CubemapTexture = nullptr;
		U32 InstanceCount = 0;
	};

	class CThumbnailRenderer 
	{
	public:
		CThumbnailRenderer(CRenderStateManager* stateManager);
		~CThumbnailRenderer() = default;

		void RenderStaticMeshThumbnail(const SStaticMeshThumbnailPassData& passData);
		void RenderSkeletalMeshThumbnail(const SSkeletalMeshThumbnailPassData& passData);
		void RenderCubemapTextureThumbnail(const SCubemapTexturePassData& passData);
	
	private:
		U16 StaticMeshThumbnailPSOIndex = 0;
		U16 SkeletalMeshThumbnailPSOIndex = 0;
		U16 CubemapTextureThumbnailPSOIndex = 0;

		CRenderStateManager* RenderStateManager = nullptr;
	};
}
