// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/System.h"
#include "Graphics/GraphicsStructs.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/RenderResourceRegistry.h"
#include "Graphics/Renderers/GeometryRenderer.h"
#include "Graphics/Renderers/LightRenderer.h"
#include "MathTypes/Sphere.h"

namespace Havtorn
{
	class CAssetRegistry;
	class CRenderManager;
	class CWorld;
	class CScene;
	struct SEntity;
	struct SComponent;
	struct SFrustum;
	struct SRenderView;
	struct SStaticMeshDrawData;
	struct SDirectionalLightComponent;
	struct SPointLightComponent;
	struct SSpotLightComponent;
	struct SVolumetricLightComponent;
	struct SStaticMeshComponent;
	struct SSkeletalMeshComponent;
	struct SDecalComponent;
	struct SSpriteComponent;
	struct SUICanvasComponent;
	struct SGraphicsMaterialAsset;

	// TODO.NW: Find the right place for the pass data. Common shared file representing default game render passes or something?
	struct STonemappingPassData : public SRenderPassParams
	{
		SRenderResourceHandle RenderTargetTextureHandle;
		SRenderResourceHandle TonemappingTargetHandle;
	};

	struct SViewData
	{
		SMatrix ViewMatrix = SMatrix::Identity;
		SMatrix ProjectionMatrix = SMatrix::Identity;
	};

	// Remainder after culling
	struct SSceneLightCollection
	{
		std::vector<SDirectionalLightComponent*> DirectionalLights;
		std::vector<SPointLightComponent*> PointLights;
		std::vector<SSpotLightComponent*> Spotlights;
		
		// If lights were culled, these are 1 to 1 with their corresponding component vector
		std::vector<SFrustum> DirectionalLightCullGeometry; 
		std::vector<SSphere> PointLightCullGeometry; 
		std::vector<SFrustum> SpotlightCullGeometry; 
	};

	using SShadowmapProcessingResult = std::tuple<std::vector<SShadowmapBufferData>, std::vector<U16>>;
	using SMaterialProcessingResult = std::tuple<std::vector<SRenderResourceHandle>, SMaterialBufferData>;

	class CRenderSystem final : public ISystem
	{
	public:
		CRenderSystem(CRenderManager* renderManager, CWorld* world);
		~CRenderSystem() override = default;

		void Update(std::vector<Ptr<CScene>>& scenes) override;
		
		SSceneLightCollection GatherActiveLights(CScene* scene, const bool doCulling, const SFrustum& viewFrustum) const;
		ENGINE_API void ProcessRenderView(SRenderView* renderView, CScene* scene, const SViewData& viewData, const bool doCulling) const;

		void ProcessDirectionalLight(SRenderView* renderView, CScene* scene, const SDirectionalLightComponent* lightComponent, const SVector2<F32>& shadowAtlasResolution, const F32 shadowTestTolerance) const;
		void ProcessPointLight(SRenderView* renderView, CScene* scene, const SPointLightComponent* lightComponent, const SVector2<F32>& shadowAtlasResolution, const F32 shadowTestTolerance) const;
		void ProcessSpotlight(SRenderView* renderView, CScene* scene, const SSpotLightComponent* lightComponent, const SVector2<F32>& shadowAtlasResolution, const F32 shadowTestTolerance) const;
		void ProcessStaticMesh(SRenderView* renderView, CScene* scene, const SStaticMeshComponent* meshComponent, const bool doCulling, const SFrustum& viewFrustum, const SSceneLightCollection& sceneLights, const SEntity& editorRenderExemptEntity) const;
		void ProcessSkeletalMesh(SRenderView* renderView, CScene* scene, const SSkeletalMeshComponent* meshComponent, const bool doCulling, const SFrustum& viewFrustum, const SSceneLightCollection& sceneLights, const SEntity& editorRenderExemptEntity) const;
		void ProcessDecal(SRenderView* renderView, CScene* scene, const SDecalComponent* component, const bool doCulling, const SFrustum& viewFrustum) const;
		void ProcessSprite(SRenderView* renderView, CScene* scene, const SSpriteComponent* component, const bool doCulling, const SFrustum& viewFrustum) const;
		void ProcessUICanvas(SRenderView* renderView, CScene* scene, const SUICanvasComponent* component) const;

		ENGINE_API SVolumetricLightBufferData ProcessVolumetricLightData(const SVolumetricLightComponent* component) const;
		ENGINE_API SShadowmapProcessingResult ProcessShadowmapBufferData(const std::vector<SShadowmapViewData>& viewData, const SVector2<F32>& shadowAtlasResolution, const F32 shadowTestTolerance) const;
		ENGINE_API SStaticMeshDrawData* ProcessStaticGeometry(SRenderView* renderView, const std::vector<SDrawCallData>& drawCallData, const std::vector<SGraphicsMaterialAsset*>& materialAssets, const SSceneLightCollection& sceneLights, const bool doCulling, const SSphere& boundingSphere, const SFrustum& viewFrustum, const SMatrix& meshTransformMatrix, const U32 drawID, const U32 meshAssetID, const U64 assetRequester) const;
		ENGINE_API SMaterialProcessingResult ProcessMaterialData(SRenderView* renderView, const SEngineGraphicsMaterial& material, const U64 assetRequester) const;

		// TODO.NW: Technically missing non-ui screen space sprite here, before tonemapping. Haven't really used that but supported it before the rework
		ENGINE_API void AddDefaultPasses(SRenderView* renderView, const bool runEditorDataPasses) const;
		
	private:
		CRenderManager* RenderManager = nullptr;
		CRenderStateManager* StateManager = nullptr;
		CAssetRegistry* AssetRegistry = nullptr;
		CWorld* World = nullptr;
		DelegateHandle Handle = {};
	};
}
