// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"
#include "Renderers/CustomRenderer.h"
#include "Renderers/DebugRenderer.h"
#include "Renderers/FullscreenRenderer.h"
#include "Renderers/GeometryRenderer.h"
#include "Renderers/LightRenderer.h"
#include "Renderers/SpriteRenderer.h"
#include "Renderers/ThumbnailRenderer.h"
#include "RenderTextureFactory.h"
#include "RenderStateManager.h"
#include "GraphicsEnums.h"
#include "GraphicsMaterial.h"
#include "RenderGraph.h"
#include "Scene/World.h"

#include <RHI/RenderingPrimitives/DataBuffer.h>
#include <RHI/RenderingPrimitives/RenderTexture.h>
#include <RHI/RenderingPrimitives/GBuffer.h>

#include <queue>

#include "Input/InputTypes.h"

namespace Havtorn
{
	class CPlatformManager;
	class CRHI;
	struct SRenderCommand;
	struct SStaticMeshComponent;
	struct SSkeletalMeshComponent;
	struct SMaterialComponent;
	struct SDecalComponent;
	struct SEnvironmentLightComponent;
	struct SSpriteComponent;
	struct SGraphicsMaterialAsset;

	enum class ERenderViewMode
	{
		All,
		Game,
		Depth,
		GBufferAlbedo,
		GBufferNormals,
		GBufferMaterials,
		SSAO,
		DeferredLighting,
		VolumetricLighting,
		Bloom,
		Tonemapping,
		Antialiasing,
		EditorData,
		WorldPosition,
		Count
	};

	struct SPrimitiveData
	{
		SRenderResourceHandle VertexBufferHandle;
		SRenderResourceHandle IndexBufferHandle;
		U32 IndexCount = 0;
	};

	// Replaces DrawCallData
	struct SMeshDrawData
	{
		// Data that comes from external sources that should be declared (with external memory) in setup
		std::vector<SRenderResourceHandle> MaterialTextureHandles;
		// Vertex Buffers should be owned by Render Thread
		SPrimitiveData PrimitiveData;

		// Buffer data that should be bound at execution time - can we store this in a way that makes sense? Can we make all buffers immutable or dynamic?
		// Many instances here, would like these holders to just point to one entry in the resource registry. But that means storing many buffers in the registry
		SMaterialBufferData MaterialBufferData;
	};

	struct SStaticShadowCasterData
	{
		std::vector<SPrimitiveData> PrimitiveData;
		std::vector<SMatrix> Transforms;
	};

	struct SStaticMeshDrawData
	{
		std::vector<SMeshDrawData> DrawData; // one draw data per submesh

		// Instance Data
		std::vector<SMatrix> Transforms;
		std::vector<SEntity> Entities;
	};

	struct SSkeletalMeshDrawData
	{
		std::vector<SMeshDrawData> DrawData;

		std::vector<SMatrix> Transforms;
		std::vector<SEntity> Entities;
		std::vector<SMatrix> Bones; // Still need a way to split these out so each animated mesh can use its own bone buffer (but preferrably be instanced still)
	};

	struct SSpriteDrawData
	{
		SRenderResourceHandle SpriteTextureHandle;
		std::vector<SMatrix> Transforms{};
		std::vector<SVector4> UVRects{};
		std::vector<SVector4> Colors{};
		std::vector<SEntity> Entities{};
	};

	struct SDecalDrawData
	{
		std::vector<SRenderResourceHandle> MaterialTextureHandles;
		SMaterialBufferData MaterialBufferData;

		std::vector<SMatrix> Transforms;
	};

	struct SDirectionalLightDrawData
	{
		// Associated by mesh ID rather than drawID, for more effective instancing
		std::unordered_map<U32, SStaticShadowCasterData> ShadowCasterData;
		SDirectionalLightBufferData LightBufferData;
		SShadowmapBufferData ShadowmapBufferData;
		SVolumetricLightBufferData VolumetricBufferData;
		SRenderResourceHandle CubemapTextureHandle;
		U16 ShadowmapViewportIndex = 0;
		bool RenderVolumetrics = false;
	};

	struct SPointLightDrawData
	{
		// TODO.NW: Right now these are associated by light instance, maybe it would be more beneficial
		// to associate by shadowmap view? Potentially a higher cost to check meshes against all sides of the 
		// point light cubes individually though.
		std::unordered_map<U32, SStaticShadowCasterData> ShadowCasterData;
		SPointLightBufferData LightBufferData;
		std::array<SShadowmapBufferData, 6> ShadowmapBufferData;
		SVolumetricLightBufferData VolumetricBufferData;
		U16 ShadowmapViewportIndex = 0;
		bool RenderVolumetrics = false;
	};

	struct SSpotlightDrawData
	{
		std::unordered_map<U32, SStaticShadowCasterData> ShadowCasterData;
		SPointLightBufferData PointLightBufferData;
		SSpotLightBufferData SpotlightBufferData;
		SShadowmapBufferData ShadowmapBufferData;
		SVolumetricLightBufferData VolumetricBufferData;
		U16 ShadowmapViewportIndex = 0;
		bool RenderVolumetrics = false;
	};

	struct SDebugObjectDrawData
	{
		SMatrix ToWorldFromObject;
		SVector4 Color;
		F32 HalfThickness = 0.5f;
		SPrimitiveData PrimitiveData;
	};

	struct SRenderViewDrawData
	{
		// Associates mesh+material asset combinations (drawID) to instanced draw data. Each view handles its own culling so 
		// we expect different sets of instances of maybe the same meshes for different views.
		std::unordered_map<U32, SStaticMeshDrawData> StaticMeshDrawData;
		std::unordered_map<U32, SSkeletalMeshDrawData> SkeletalMeshDrawData;
		std::unordered_map<U32, SSpriteDrawData> WorldSpaceSpriteDrawData;
		std::unordered_map<U32, SSpriteDrawData> ScreenSpaceSpriteDrawData;
		std::unordered_map<U32, SDecalDrawData> DecalDrawData;
		std::unordered_map<U32, SSpriteDrawData> WidgetSpriteDrawData;
		std::unordered_map<U32, SSpriteDrawData> UISpriteDrawData;

		std::vector<SDirectionalLightDrawData> DirectionalLightDrawData;
		std::vector<SPointLightDrawData> PointLightDrawData;
		std::vector<SSpotlightDrawData> SpotlightDrawData;

		std::vector<SDebugObjectDrawData> DebugObjectsDepth3D;
		std::vector<SDebugObjectDrawData> DebugObjectsNoDepth3D;
		std::vector<SDebugObjectDrawData> DebugObjectsDepth2D;
		std::vector<SDebugObjectDrawData> DebugObjectsNoDepth2D;
	};

	struct SRenderView
	{
		SRenderView(CRenderStateManager* stateManager, CAssetRegistry* assetRegistry, CRenderResourceRegistry* renderResourceRegistry);
		~SRenderView();
		void ReleaseViewResources();

		// TODO.NW: Do we wanna store these as references instead?
		CRenderStateManager* StateManager = nullptr;
		CAssetRegistry* AssetRegistry = nullptr;
		CRenderResourceRegistry* RenderResourceRegistry = nullptr;

		SVector2<U16> ViewSize = SVector2<U16>::Zero;
		CRenderTexture RenderTarget;
		CRenderTexture EditorDataTexture;
		CRenderTexture WorldPositionTexture;
		CRenderGraph RenderGraph;

		// Render view may request assets and unrequest them when finished drawing

		SRenderViewDrawData DrawData;

		SMatrix ViewMatrix = SMatrix::Identity;
		SMatrix ProjectionMatrix = SMatrix::Identity;

		SRenderResourceHandle SkyboxCubemapTextureHandle;

		void* EntityPerPixelData = nullptr;
		U64 EntityPerPixelDataSize = 0;
		
		void* WorldPositionPerPixelData = nullptr;
		U64 WorldPositionPerPixelDataSize = 0;

		// This signature is provided like this because the culling preprocessing leaves us with this most cache-friendly data
		// This is expected to overwrite any previous instancing data for a particular drawID.
		SMeshDrawData ConstructMeshDrawData(const SDrawCallData& assetDrawCallData, const char* renderResourceSuffix, std::vector<SGraphicsMaterialAsset*>& materialAssets, const U32 renderViewID);
		void AddStaticMeshDrawData(const U32 renderViewID, const U32 meshAssetID, const std::vector<U32>& materialAssetIDs, const std::vector<SMatrix>& transforms, const std::vector<SEntity>& entities, const bool deferCommand = false);
		void AddSkeletalMeshDrawData(const U32 renderViewID, const U32 meshAssetID, const std::vector<U32>& materialAssetIDs, const std::vector<SMatrix>& transforms, const std::vector<SEntity>& entities, const std::vector<SMatrix>& bones, const bool deferCommand = false);
		void ExtractEditorData();
	};

	class CRenderManager
	{
		friend CAssetRegistry;
		friend CRenderGraph;
		friend CRenderResourceRegistry;
		friend CFullscreenRenderer;

	public:
		CRenderManager() = default;
		~CRenderManager();
		bool Init(CRHI* rhi, CPlatformManager* windowHandler);
		void Render();

		void UpdateResolution(SVector2<U16> newResolution);

		ENGINE_API CRenderTexture CreateRenderTextureFromSource(const std::string& filePath);
		ENGINE_API CRenderTexture CreateRenderTextureFromAsset(const std::string& filePath, const EAssetType assetType);
		ENGINE_API CRenderTexture CreateRenderTextureFromData(const SVector2<U16> size, const DXGI_FORMAT format, void* data, const U64 elementSize);

		ENGINE_API U64 GetEntityGUIDFromData(const U64 renderViewID, const U64 dataIndex) const;
		ENGINE_API SVector4 GetWorldPositionFromData(const U64 renderViewID, const U64 dataIndex) const;

		U32 WriteToAnimationDataTexture(const std::string& animationName);

		ENGINE_API SRenderView* GetRenderView(const U64 id);
		ENGINE_API bool HasRenderView(const U64 id);
		ENGINE_API std::map<U64, SRenderView>* GetRenderViews();

		ENGINE_API SPostProcessingBufferData GetPostProcessingBufferData() const;
		ENGINE_API void SetPostProcessingBufferData(const SPostProcessingBufferData& data);

	public:
		void SyncCrossThreadResources(const CWorld* world, const SVector2<U16>& windowResizeTarget);
		void SetWorldMainCameraEntity(const SEntity& entity);
		void SetWorldEditorRenderExemptEntity(const SEntity& entity);
		void SetWorldPlayState(EWorldPlayState playState);
		[[nodiscard]] ENGINE_API CRenderTexture* GetRenderTargetTexture(const U64 renderViewID) const;
		
		ENGINE_API CRenderResourceRegistry& GetRenderResourceRegistry();
		ENGINE_API CRenderTextureFactory& GetRenderTextureFactory();
		ENGINE_API CRenderStateManager& GetRenderStateManager();

		void SwapRenderViews(const SVector2<U16>& windowResizeTarget);

		// If a callback is provided, the request will be unrequested after executing the callback
		ENGINE_API void RequestRenderView(const U64& renderViewID, std::optional<std::function<void(CRenderTexture)>> callback = {});
		ENGINE_API void UnrequestRenderView(const U64& renderViewID);

		ENGINE_API const SVector2<U16>& GetCurrentWindowResolution() const;
		ENGINE_API const SVector2<F32>& GetShadowAtlasResolution() const;
		ENGINE_API F32 GetShadowTestTolerance() const;
		ENGINE_API U32 GetNumberOfRenderViews() const;

		ENGINE_API void SetRenderViewMode(const ERenderViewMode renderPass);
		ENGINE_API ERenderViewMode GetRenderViewMode() const;

		ENGINE_API void DebugLiveRenderObjects() const;

	public:
		ENGINE_API static U32 NumberOfDrawCallsThisFrame;
		
		// Render "Modules"
		CCustomRenderer CustomRenderer;
		CDebugRenderer DebugRenderer;
		CFullscreenRenderer FullscreenRenderer;
		CGeometryRenderer GeometryRenderer;
		CLightRenderer LightRenderer;
		CSpriteRenderer SpriteRenderer;
		CThumbnailRenderer ThumbnailRenderer;

	private:
		void InitShadowmapAtlas(SVector2<F32> atlasResolution);
		void InitShadowmapLOD(SVector2<F32> topLeftCoordinate, const SVector2<F32>& widthAndHeight, const SVector2<F32>& depth, const SVector2<F32>& atlasResolution, U16 mapsInLod, U16 startIndex);

		void CheckRenderViewMode(const U64 renderViewID);
		void CycleRenderViewMode(const SInputActionPayload payload);

	private:
		CRHI* RHI = nullptr;
		CRenderStateManager RenderStateManager;
		CRenderTextureFactory RenderTextureFactory;

		std::map<U64, SRenderView> RenderViewsA;
		std::map<U64, SRenderView> RenderViewsB;
		CRenderResourceRegistry ResourceRegistryA;
		CRenderResourceRegistry ResourceRegistryB;

		std::map<U64, SRenderView>* GameThreadRenderViews = &RenderViewsA;
		std::map<U64, SRenderView>* RenderThreadRenderViews = &RenderViewsB;
		CRenderResourceRegistry* GameThreadResourceRegistry = &ResourceRegistryA;
		CRenderResourceRegistry* RenderThreadResourceRegistry = &ResourceRegistryB;
		std::map<U64, std::function<void(CRenderTexture&)>> RenderViewCallbacks;

		CRenderTexture Backbuffer;
		SVector4 ClearColor = SVector4(0.5f, 0.5f, 0.5f, 1.0f);

		ERenderViewMode CurrentRenderViewMode = ERenderViewMode::All;

		SVector2<U16> CurrentWindowResolution = SVector2<U16>::Zero;
		
		SVector2<F32> ShadowAtlasResolution = SVector2<F32>{ 8192.0f, 8192.0f };
		F32 ShadowTestTolerance = 0.001f;

		// NW: Keep our own properties here for use on render thread
		SEntity WorldMainCameraEntity = SEntity::Null;
		SEntity WorldEditorRenderExemptEntity = SEntity::Null;
		EWorldPlayState WorldPlayState = EWorldPlayState::Stopped;

		void* SystemSkeletalAnimationBoneData = nullptr;
		void* RendererSkeletalAnimationBoneData = nullptr;
		U64 SkeletalAnimationBoneDataSize = 0;
	};
}
