// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"
#include "GraphicsFramework.h"
#include "Renderers/FullscreenRenderer.h"
#include "RenderTextureFactory.h"
#include "RenderStateManager.h"
#include "GraphicsEnums.h"
#include "GraphicsMaterial.h"
#include "RenderCommand.h"
#include "Scene/World.h"

#include "RenderingPrimitives/DataBuffer.h"
#include "RenderingPrimitives/RenderTexture.h"
#include "RenderingPrimitives/GBuffer.h"

#include <queue>

#include "Core/RuntimeAssetDeclarations.h"
#include "Input/InputTypes.h"

namespace Havtorn
{
	class CGraphicsFramework;
	class CPlatformManager;
	struct SRenderCommand;
	struct SStaticMeshComponent;
	struct SSkeletalMeshComponent;
	struct SMaterialComponent;
	struct SDecalComponent;
	struct SEnvironmentLightComponent;
	struct SSpriteComponent;

	enum class ERenderPass
	{
		All,
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
		Count
	};

	struct SRenderCommandComparer
	{
		bool operator()(const SRenderCommand& a, const SRenderCommand& b) const;
	};

	using CRenderCommandHeap = std::priority_queue<SRenderCommand, std::vector<SRenderCommand>, SRenderCommandComparer>;

	class CRenderManager
	{
		friend CAssetRegistry;

	public:
		CRenderManager() = default;
		~CRenderManager();
		bool Init(CGraphicsFramework* framework, CPlatformManager* windowHandler);
		bool ReInit(CGraphicsFramework* framework, SVector2<U16> newResolution);
		void Render();

		void Release(SVector2<U16> newResolution);

		ENGINE_API void LoadStaticMeshComponent(const std::string& filePath, SStaticMeshComponent* outStaticMeshComponent, CScene* scene = nullptr);
		ENGINE_API void LoadSkeletalMeshComponent(const std::string& filePath, SSkeletalMeshComponent* outSkeletalMeshComponent);
		ENGINE_API void LoadMaterialComponent(const std::vector<std::string>& materialPaths, SMaterialComponent* outMaterialComponent);
		// NR: Note that we use the file *name* instead of the full path here, we assume that it already exists in the registry.
		ENGINE_API bool TryLoadStaticMeshComponent(const std::string& fileName, SStaticMeshComponent* outStaticMeshComponent) const;
		ENGINE_API bool TryReplaceMaterialOnComponent(const std::string& filePath, U8 materialIndex, SMaterialComponent* outMaterialComponent) const;

		ENGINE_API SVector2<F32> GetShadowAtlasResolution() const;
		
		ENGINE_API void LoadDecalComponent(const std::vector<std::string>& texturePaths, SDecalComponent* outDecalComponent);
		ENGINE_API void LoadEnvironmentLightComponent(const std::string& ambientCubemapTexturePath, SEnvironmentLightComponent* outEnvironmentLightComponent);
		ENGINE_API void LoadSpriteComponent(const std::string& filePath, SSpriteComponent* outSpriteComponent);
		ENGINE_API void LoadSkeletalAnimationComponent(const std::vector<std::string>& filePaths, SSkeletalAnimationComponent* outSkeletalAnimationComponent);

		ENGINE_API CRenderTexture RenderStaticMeshAssetTexture(const std::string& filePath);
		ENGINE_API CRenderTexture RenderSkeletalMeshAssetTexture(const std::string& filePath);
		ENGINE_API CRenderTexture RenderSkeletalAnimationAssetTexture(const std::string& filePath, const std::vector<SMatrix>& boneTransforms = {});
		ENGINE_API CRenderTexture RenderMaterialAssetTexture(const std::string& filePath);
		ENGINE_API void RenderMaterialTexture(CRenderTexture& textureTarget, const SEngineGraphicsMaterial& material, const SVector& objectRotationEuler = SVector(-25.0f, 30.0f), const F32 zoomMultiplier = 1.0f);

		ENGINE_API void RenderSkeletalAnimationAssetTexture(CRenderTexture& assetTexture, const std::string& filePath, const std::vector<SMatrix>& boneTransforms = {});

		ENGINE_API CRenderTexture CreateRenderTextureFromSource(const std::string& filePath);
		ENGINE_API CRenderTexture CreateRenderTextureFromAsset(const std::string& filePath);

		ENGINE_API U64 GetEntityGUIDFromData(U64 dataIndex) const;

		U32 WriteToAnimationDataTexture(const std::string& animationName);

		bool IsStaticMeshInInstancedRenderList(const std::string& meshName);
		void AddStaticMeshToInstancedRenderList(const std::string& meshName, const STransformComponent* component);
		void SwapStaticMeshInstancedRenderLists();
		void ClearSystemStaticMeshInstanceData();

		bool IsSkeletalMeshInInstancedRenderList(const std::string& meshName);
		void AddSkeletalMeshToInstancedRenderList(const std::string& meshName, const STransformComponent* transformComponent, const SSkeletalAnimationComponent* animationComponent);
		void SwapSkeletalMeshInstancedRenderLists();
		void ClearSystemSkeletalMeshInstanceData();

		bool IsSpriteInWorldSpaceInstancedRenderList(const U32 textureBankIndex);
		void AddSpriteToWorldSpaceInstancedRenderList(const U32 textureBankIndex, const STransformComponent* worldSpaceTransform, const SSpriteComponent* spriteComponent);
		void AddSpriteToWorldSpaceInstancedRenderList(const U32 textureBankIndex, const STransformComponent* worldSpaceTransform, const STransformComponent* cameraTransform);
		void SwapSpriteWorldInstancedRenderLists();
		void ClearSystemWorldSpaceSpriteInstanceData();

		bool IsSpriteInScreenSpaceInstancedRenderList(const U32 textureBankIndex);
		void AddSpriteToScreenSpaceInstancedRenderList(const U32 textureBankIndex, const STransform2DComponent* screenSpaceTransform, const SSpriteComponent* spriteComponent);
		void SwapSpriteScreenInstancedRenderLists();
		void ClearSystemScreenSpaceSpriteInstanceData();

	public:
		void SyncCrossThreadResources(const CWorld* world);
		void SetWorldPlayState(EWorldPlayState playState);
		[[nodiscard]] ENGINE_API const CRenderTexture& GetRenderedSceneTexture() const;
		void PushRenderCommand(SRenderCommand command);
		void SwapRenderCommandBuffers();

		const SVector2<U16>& GetCurrentWindowResolution() const;

	public:
		ENGINE_API static U32 NumberOfDrawCallsThisFrame;

	private:
		void Clear(SVector4 clearColor);
		void InitRenderTextures(SVector2<U16> windowResolution);
		void InitShadowmapAtlas(SVector2<F32> atlasResolution);
		void InitShadowmapLOD(SVector2<F32> topLeftCoordinate, const SVector2<F32>& widthAndHeight, const SVector2<F32>& depth, const SVector2<F32>& atlasResolution, U16 mapsInLod, U16 startIndex);
		
		void InitDataBuffers();
		
		void BindRenderFunctions();

	private:
		std::vector<U16> AddMaterial(const std::string& materialName, EMaterialConfiguration configuration);

		inline void ShadowAtlasPrePassDirectional(const SRenderCommand& command);
		inline void ShadowAtlasPrePassPoint(const SRenderCommand& command);
		inline void ShadowAtlasPrePassSpot(const SRenderCommand& command);
		inline void CameraDataStorage(const SRenderCommand& command);
		inline void GBufferDataInstanced(const SRenderCommand& command);
		inline void GBufferDataInstancedEditor(const SRenderCommand& command);
		inline void GBufferSkeletalInstanced(const SRenderCommand& command);
		inline void GBufferSkeletalInstancedEditor(const SRenderCommand& command);
		inline void GBufferSpriteInstanced(const SRenderCommand& command);
		inline void GBufferSpriteInstancedEditor(const SRenderCommand& command);
		inline void DecalDepthCopy(const SRenderCommand& command);
		inline void DeferredDecal(const SRenderCommand& command);
		inline void PreLightingPass(const SRenderCommand& command);
		inline void DeferredLightingDirectional(const SRenderCommand& command);
		inline void DeferredLightingPoint(const SRenderCommand& command);
		inline void DeferredLightingSpot(const SRenderCommand& command);
		inline void Skybox(const SRenderCommand& command);
		inline void PostBaseLightingPass(const SRenderCommand& command);
		inline void VolumetricLightingDirectional(const SRenderCommand& command);
		inline void VolumetricLightingPoint(const SRenderCommand& command);
		inline void VolumetricLightingSpot(const SRenderCommand& command);
		inline void VolumetricBlur(const SRenderCommand& command);
		inline void ForwardTransparency(const SRenderCommand& command);
		inline void ScreenSpaceSprite(const SRenderCommand& command);
		inline void WorldSpaceSpriteEditorWidget(const SRenderCommand& command);
		inline void RenderBloom(const SRenderCommand& command);
		inline void Tonemapping(const SRenderCommand& command);
		inline void AntiAliasing(const SRenderCommand& command);
		inline void GammaCorrection(const SRenderCommand& command);
		inline void RendererDebug(const SRenderCommand& command);
		inline void PreDebugShapes(const SRenderCommand& command);
		inline void PostTonemappingUseDepth(const SRenderCommand& command);
		inline void PostTonemappingIgnoreDepth(const SRenderCommand& command);
		inline void DebugShapes(const SRenderCommand& command);

		inline void DebugShadowAtlas();

		void CheckIsolatedRenderPass();
		void CycleRenderPass(const SInputActionPayload payload);

	private:
		struct SFrameBufferData
		{
			SMatrix ToCameraFromWorld;
			SMatrix ToWorldFromCamera;
			SMatrix ToProjectionFromCamera;
			SMatrix ToCameraFromProjection;
			SVector4 CameraPosition;
		} FrameBufferData;
		HV_ASSERT_BUFFER(SFrameBufferData)

		struct SObjectBufferData
		{
			SMatrix ToWorldFromObject;
		} ObjectBufferData;
		HV_ASSERT_BUFFER(SObjectBufferData)

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

		} MaterialBufferData;
		HV_ASSERT_BUFFER(SMaterialBufferData)

		struct SDebugShapeObjectBufferData
		{
			SMatrix ToWorldFromObject;
			SVector4 Color;
			F32 HalfThickness = 0.5f;
			F32 Padding[3] = {};
		} DebugShapeObjectBufferData;
		HV_ASSERT_BUFFER(SDebugShapeObjectBufferData)

		struct SDecalBufferData
		{
			SMatrix ToWorld;
			SMatrix ToObjectSpace;
		} DecalBufferData;
		HV_ASSERT_BUFFER(SDecalBufferData)

		struct SSpriteBufferData
		{
			SVector4 Color;
			SVector4 UVRect;
			SVector2<F32> Position;
			SVector2<F32> Size;
			F32 Rotation;
			SVector Padding;
		} SpriteBufferData;
		HV_ASSERT_BUFFER(SSpriteBufferData)

		struct SDirectionalLightBufferData
		{
			SVector4 DirectionalLightDirection;
			SVector4 DirectionalLightColor;
		} DirectionalLightBufferData;
		HV_ASSERT_BUFFER(SDirectionalLightBufferData)

		struct SPointLightBufferData
		{
			SMatrix ToWorldFromObject;
			SVector4 ColorAndIntensity;
			SVector4 PositionAndRange;
		} PointLightBufferData;
		HV_ASSERT_BUFFER(SPointLightBufferData)

		struct SSpotLightBufferData
		{
			SVector4 ColorAndIntensity;
			SVector4 PositionAndRange;
			SVector4 Direction;
			SVector4 DirectionNormal1;
			SVector4 DirectionNormal2;
			F32 OuterAngle = 0.0f;
			F32 InnerAngle = 0.0f;
			SVector2<F32> Padding;
		} SpotLightBufferData;
		HV_ASSERT_BUFFER(SSpotLightBufferData)

		struct SShadowmapBufferData
		{
			SMatrix ToShadowmapView;
			SMatrix ToShadowmapProjection;
			SVector4 ShadowmapPosition;
			SVector2<F32> ShadowmapResolution;
			SVector2<F32> ShadowAtlasResolution;
			SVector2<F32> ShadowmapStartingUV;
			F32 ShadowTestTolerance = 0.0f;
			F32 Padding = -1.0f;
		} ShadowmapBufferData;
		HV_ASSERT_BUFFER(SShadowmapBufferData)

		struct SVolumetricLightBufferData
		{
			F32 NumberOfSamplesReciprocal = (1.0f / 16.0f);
			F32 LightPower = 500000.0f;
			F32 ScatteringProbability = 0.0001f;
			F32 HenyeyGreensteinGValue = 0.0f;
		} VolumetricLightBufferData;
		HV_ASSERT_BUFFER(SVolumetricLightBufferData)

		struct SEmissiveBufferData
		{
			F32 EmissiveStrength = 1.0f;
			SVector Padding;
		} EmissiveBufferData;
		HV_ASSERT_BUFFER(SEmissiveBufferData)

		struct SBoneBufferData
		{
			SMatrix Bones[64];
		} BoneBufferData;
		HV_ASSERT_BUFFER(SBoneBufferData)

	private:
		CGraphicsFramework* Framework = nullptr;
		CDataBuffer FrameBuffer;
		CDataBuffer ObjectBuffer;
		CDataBuffer MaterialBuffer;
		CDataBuffer DebugShapeObjectBuffer;
		CDataBuffer DecalBuffer;
		CDataBuffer SpriteBuffer;
		CDataBuffer DirectionalLightBuffer;
		CDataBuffer PointLightBuffer;
		CDataBuffer SpotLightBuffer;
		CDataBuffer ShadowmapBuffer;
		CDataBuffer VolumetricLightBuffer;
		CDataBuffer EmissiveBuffer;
		CDataBuffer BoneBuffer;
		CRenderStateManager RenderStateManager;
		CFullscreenRenderer FullscreenRenderer;

		CRenderTextureFactory RenderTextureFactory;
		CRenderTexture RenderedScene;
		CRenderTexture Backbuffer;
		CRenderTexture IntermediateTexture;
		CRenderTexture IntermediateDepth;
		CRenderTexture EditorWidgetDepth;
		CRenderTexture ShadowAtlasDepth;
		CRenderTexture DepthCopy;

		CRenderTexture HalfSizeTexture;
		CRenderTexture QuarterSizeTexture;
		CRenderTexture BlurTexture1;
		CRenderTexture BlurTexture2;
		CRenderTexture VignetteTexture;

		CRenderTexture LitScene;
		CRenderTexture VolumetricAccumulationBuffer;
		CRenderTexture VolumetricBlurTexture;
		CRenderTexture SSAOBuffer;
		CRenderTexture SSAOBlurTexture;
		CRenderTexture DownsampledDepth;
		CRenderTexture TonemappedTexture;
		CRenderTexture AntiAliasedTexture;
		CRenderTexture EditorDataTexture;
		CRenderTexture SkeletalAnimationDataTextureCPU;
		CRenderTexture SkeletalAnimationDataTextureGPU;
		CGBuffer GBuffer;

		CRenderCommandHeap RenderCommandsA;
		CRenderCommandHeap RenderCommandsB;

		CRenderCommandHeap* PushToCommands = &RenderCommandsA;
		CRenderCommandHeap* PopFromCommands = &RenderCommandsB;

		SVector4 ClearColor = SVector4(0.5f, 0.5f, 0.5f, 1.0f);

		std::map<ERenderCommandType, std::function<void(const SRenderCommand& command)>> RenderFunctions;
		ERenderPass CurrentRunningRenderPass = ERenderPass::All;
		bool ShouldBlurVolumetricBuffer = false;
		
		CDataBuffer InstancedTransformBuffer;
		CDataBuffer InstancedEntityIDBuffer;
		CDataBuffer InstancedAnimationDataBuffer;

		// NR: Used together with the InstancedTransformBuffer to batch World Space Sprites as well as Screen Space Sprites
		CDataBuffer InstancedUVRectBuffer;
		CDataBuffer InstancedColorBuffer;

		struct SStaticMeshInstanceData
		{
			std::vector<SMatrix> Transforms{};
			std::vector<SEntity> Entities{};
		};

		struct SSkeletalMeshInstanceData
		{
			std::vector<SMatrix> Transforms{};
			std::vector<SMatrix> Bones{};
			std::vector<SEntity> Entities{};
		};

		struct SSpriteInstanceData
		{
			std::vector<SMatrix> Transforms{};
			std::vector<SVector4> UVRects{};
			std::vector<SVector4> Colors{};
			std::vector<SEntity> Entities{};
		};

		// TODO.NR: Add GUIDs to things like this
		std::unordered_map<std::string, SStaticMeshAsset> LoadedStaticMeshes;
		std::unordered_map<std::string, SSkeletalMeshAsset> LoadedSkeletalMeshes;
		std::unordered_map<std::string, SSkeletalAnimationAsset> LoadedSkeletalAnims;
		// NR: These are used as a way of cross-thread resource management
		std::unordered_map<std::string, SStaticMeshInstanceData> SystemStaticMeshInstanceData;
		std::unordered_map<std::string, SStaticMeshInstanceData> RendererStaticMeshInstanceData;

		std::unordered_map<std::string, SSkeletalMeshInstanceData> SystemSkeletalMeshInstanceData;
		std::unordered_map<std::string, SSkeletalMeshInstanceData> RendererSkeletalMeshInstanceData;

		// TODO.NR: Maybe generalize GPU Instancing resources that are kept duplicate like this, combining
		// the 4 function calls as well somehow. Maybe templated? Could use two template arguments, one for key and 
		// one for the value

		/*
			NR: It's annoying having a separate map for the SMatrix collections, but since sprites
			reference their texture using an index directly into the TextureBank, it makes sense 
			to key into these collections using that. We could get the corresponding filepath from
			the TextureBank in a roundabout way, to use as the key into the above collection, but
			this seems better for now
		*/
		std::unordered_map<U32, SSpriteInstanceData> SystemWorldSpaceSpriteInstanceData;
		std::unordered_map<U32, SSpriteInstanceData> RendererWorldSpaceSpriteInstanceData;

		std::unordered_map<U32, SSpriteInstanceData> SystemScreenSpaceSpriteInstanceData;
		std::unordered_map<U32, SSpriteInstanceData> RendererScreenSpaceSpriteInstanceData;

		SVector2<F32> ShadowAtlasResolution = SVector2<F32>::Zero;
		SVector2<U16> CurrentWindowResolution = SVector2<U16>::Zero;

		// NR: Keep our own property here for use on render thread
		EWorldPlayState WorldPlayState = EWorldPlayState::Stopped;

		void* EntityPerPixelData = nullptr;
		U64 EntityPerPixelDataSize = 0;

		void* SystemSkeletalAnimationBoneData = nullptr;
		void* RendererSkeletalAnimationBoneData = nullptr;
		U64 SkeletalAnimationBoneDataSize = 0;
		
		const U16 InstancedDrawInstanceLimit = 65535;
	};
}
