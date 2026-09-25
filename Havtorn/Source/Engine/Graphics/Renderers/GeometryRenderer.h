// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	class CRenderStateManager;
	struct SRenderResource;

	struct SMaterialBufferData
	{
		SMaterialBufferData() = default;

		SMaterialBufferData(const SEngineGraphicsMaterial& engineGraphicsMaterial)
			: RecreateZ(engineGraphicsMaterial.RecreateNormalZ)
		{
			memcpy(&Properties[0], &engineGraphicsMaterial, sizeof(SRuntimeGraphicsMaterialProperty) * 11);
		}

		SRuntimeGraphicsMaterialProperty Properties[11];

		bool RecreateZ = true;
		bool Padding[15] = {};
	};
	HV_ASSERT_BUFFER(SMaterialBufferData)

	struct SBoneBufferData
	{
		SMatrix Bones[64];
	};
	HV_ASSERT_BUFFER(SBoneBufferData)

	struct SDecalBufferData
	{
		SMatrix ToWorld;
		SMatrix ToObjectSpace;
	} DecalBufferData;
	HV_ASSERT_BUFFER(SDecalBufferData)

	struct SEditorGeometryRenderData
	{
		SRenderResource* EntityBuffer = nullptr;
	};

	struct SStaticMeshRenderData
	{
		SRenderResource* MaterialBuffer = nullptr;
		SRenderResource* VertexBuffer = nullptr;
		SRenderResource* IndexBuffer = nullptr;
		SRenderResource* TransformBuffer = nullptr;
		std::vector<SRenderResource*> MaterialTextures;
		U32 IndexCount = 0;
		U32 InstanceCount = 0;
	};

	struct SSkeletalMeshRenderData
	{
		SRenderResource* MaterialBuffer = nullptr;
		SRenderResource* VertexBuffer = nullptr;
		SRenderResource* IndexBuffer = nullptr;
		SRenderResource* BoneBuffer = nullptr;
		SRenderResource* TransformBuffer = nullptr;
		std::vector<SRenderResource*> MaterialTextures;
		U32 IndexCount = 0;
		U32 InstanceCount = 0;
	};

	struct SDecalRenderData
	{
		SRenderResource* DecalBuffer = nullptr;
		SRenderResource* OptionalAlbedoTexture = nullptr; // TODO.NW: Change this to material
		SRenderResource* OptionalMaterialTexture = nullptr;
		SRenderResource* OptionalNormalTexture = nullptr;
	};

	struct SSkyboxRenderData
	{
		SRenderResource* LitSceneTextureWithDepth = nullptr;
		SRenderResource* CubemapTexture = nullptr;
	};

	// TODO.NW: Rename to scene renderer?
	class CGeometryRenderer
	{
	public:
		CGeometryRenderer(CRenderStateManager* stateManager);
		~CGeometryRenderer() = default;

		void RenderStaticMesh(const SStaticMeshRenderData& passData);
		void RenderEditorStaticMesh(const SStaticMeshRenderData& passData, const SEditorGeometryRenderData& editorPassData);
		void RenderSkeletalMesh(const SSkeletalMeshRenderData& passData);
		void RenderEditorSkeletalMesh(const SSkeletalMeshRenderData& passData, const SEditorGeometryRenderData& editorPassData);
		void RenderDecal(const SDecalRenderData& passData);
		void RenderSkybox(const SSkyboxRenderData& passData);

	private:
		U16 StaticMeshGBufferPassPSOIndex = 0;
		U16 StaticMeshEditorGBufferPassPSOIndex = 0;
		U16 SkeletalMeshGBufferPassPSOIndex = 0;
		U16 SkeletalMeshEditorGBufferPassPSOIndex = 0;
		U16 DecalGBufferPassPSOIndex = 0;
		U16 SkyboxPassPSOIndex = 0;

		CRenderStateManager* RenderStateManager = nullptr;
	};
}
