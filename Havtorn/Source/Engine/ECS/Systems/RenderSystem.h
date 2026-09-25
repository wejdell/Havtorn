// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/System.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	class CRenderManager;
	class CWorld;
	struct SEntity;
	struct SComponent;
	struct SSphere;
	struct SFrustum;

	class CRenderSystem final : public ISystem
	{
	public:
		CRenderSystem(CRenderManager* renderManager, CWorld* world);
		~CRenderSystem() override = default;

		void Update(std::vector<Ptr<CScene>>& scenes) override;
		
		// Camera Entity may be null
		ENGINE_API void PushCommandsForScene(CScene* scene, const U64& renderViewID, const SEntity& cameraEntity, const bool runEditorDataPasses, const bool doCulling) const;
		ENGINE_API void PushUniqueCommands(const U64& renderViewID) const;

		// Light Culling

		void AddStaticMeshShadowmapPass(const std::vector<SDrawCallData>& staticMeshData, const std::vector<SMatrix>& transforms, const std::vector<SShadowmapViewData>& shadowmapData);
		void AddStaticMeshPass(const std::vector<SDrawCallData>& staticMeshData, const std::vector<SGraphicsMaterialAsset*>& materials, const std::vector<SMatrix>& transforms, const std::vector<SEntity>& entities, const bool runEditorDataPasses);
		void AddSkeletalMeshPass(const std::vector<SDrawCallData>& skeletalMeshData, const std::vector<SGraphicsMaterialAsset*>& materials, const std::vector<SMatrix>& transforms, const std::vector<SMatrix>& posedBones, const std::vector<SEntity>& entities, const bool runEditorDataPasses);
		void AddBillboardPass(STextureAsset* textureAsset, const std::vector<SMatrix>& transforms, const std::vector<SVector4>& uvRects, const std::vector<SVector4>& colors, const std::vector<SEntity>& entities, const bool runEditorDataPasses);
		void AddDecalPass(const SMatrix& transformMatrix, const std::vector<STextureAsset*>& textureAssets);
		void AddDirectionalLightPass(SDirectionalLightComponent* component, SVolumetricLightComponent* volumetricLightComp, CStaticRenderTexture* cubemapTexture, const SShadowmapViewData& shadowmapViewData); // Should include calls to volumetrics passes, can they be sorted properly in the function scope?
		void AddPointLightPass(SPointLightComponent* component, SVolumetricLightComponent* volumetricLightComp, const SMatrix& transform, const std::vector<SShadowmapViewData>& shadowmapViewData);
		void AddSpotlightPass(SSpotLightComponent* component, SVolumetricLightComponent* volumetricLightComp, const SMatrix& transform, const SShadowmapViewData& shadowmapViewData);
		void AddSkyboxPass(STextureCubeAsset* cubemapTextureAsset);
		// TODO.NW: Technically missing non-ui screen space sprite here, before tonemapping. Haven't really used that but supported it before the rework
		void AddEditorWidgetPass(STextureAsset* textureAsset, const std::vector<SMatrix>& transforms, const std::vector<SVector4>& uvRects, const std::vector<SVector4>& colors, const std::vector<SEntity>& entities);
		void AddUIPass(STextureAsset* textureAsset, const std::vector<SMatrix>& transforms, const std::vector<SVector4>& uvRects, const std::vector<SVector4>& colors);

		bool IsCulled(CScene* scene, const SSphere& boundingSphere, const SFrustum& cameraFrustum) const;

	private:
		CRenderManager* RenderManager = nullptr;
		CWorld* World = nullptr;
		DelegateHandle Handle = {};
	};
}
