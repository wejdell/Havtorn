// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"
#include "GraphicsFramework.h"
#include "Renderers/FullscreenRenderer.h"
#include "FullscreenTexture.h"
#include "FullscreenTextureFactory.h"
#include "RenderStateManager.h"
#include "GBuffer.h"
#include "GraphicsEnums.h"
#include <queue>

#include "Core/RuntimeAssetDeclarations.h"

namespace Havtorn
{
	enum class EShaderType
	{
		Vertex,
		Compute,
		Geometry,
		Pixel
	};

	enum class ESamplerType
	{
		Border,
		Clamp,
		Mirror,
		Wrap
	};

	enum class EInputLayoutType
	{
		Pos3Nor3Tan3Bit3UV2,
		Pos3Nor3Tan3Bit3UV2Trans,
		Pos4
	};

	enum class EVertexShaders
	{
		Fullscreen = 0,
		StaticMesh = 1,
		StaticMeshInstanced = 2,
		Decal = 3,
		PointAndSpotLight = 4,
		EditorPreview = 5,
	};

	enum class EPixelShaders
	{
		GBuffer = 0,
		DecalAlbedo = 1,
		DecalMaterial = 2,
		DecalNormal = 3,
		DeferredDirectional = 4,
		DeferredPoint = 5,
		DeferredSpot = 6,
		VolumetricDirectional = 7,
		VolumetricPoint = 8,
		VolumetricSpot = 9,
		EditorPreview = 10,
	};

	enum class ESamplers
	{
		DefaultWrap = 0,
		DefaultBorder = 1,
	};

	enum class ETopologies
	{
		TriangleList = 0,
	};

	class CGraphicsFramework;
	class CWindowHandler;
	struct SRenderCommand;
	struct SStaticMeshComponent;
	struct SMaterialComponent;
	struct SDecalComponent;
	struct SEnvironmentLightComponent;

	struct SRenderCommandComparer
	{
		bool operator()(const SRenderCommand& a, const SRenderCommand& b) const;
	};

	using CRenderCommandHeap = std::priority_queue<SRenderCommand, std::vector<SRenderCommand>, SRenderCommandComparer>;

	class CRenderManager
	{
	public:
		CRenderManager();
		~CRenderManager();
		bool Init(CGraphicsFramework* framework, CWindowHandler* windowHandler);
		bool ReInit(CGraphicsFramework* framework, CWindowHandler* windowHandler);
		void Render();

		void Release();

		HAVTORN_API void ConvertToHVA(const std::string& filePath, EAssetType assetType);
		HAVTORN_API void LoadStaticMeshComponent(const std::string& filePath, SStaticMeshComponent* outStaticMeshComponent);
		HAVTORN_API void LoadMaterialComponent(const std::vector<std::string>& materialNames, SMaterialComponent* outMaterialComponent);
		// NR: Note that we use the file *name* instead of the full path here, we assume that it already exists in the registry.
		HAVTORN_API bool TryLoadStaticMeshComponent(const std::string& fileName, SStaticMeshComponent* outStaticMeshComponent) const;

		HAVTORN_API EMaterialConfiguration GetMaterialConfiguration() const;
		HAVTORN_API SVector2<F32> GetShadowAtlasResolution() const;
		
		void LoadDecalComponent(const std::vector<std::string>& textureNames, SDecalComponent* outDecalComponent);
		void LoadEnvironmentLightComponent(const std::string& ambientCubemapTextureName, SEnvironmentLightComponent* outEnvironmentLightComponent);

		HAVTORN_API void* RenderStaticMeshAssetTexture(const std::string& fileName);
		HAVTORN_API void* GetTextureAssetTexture(const std::string& fileName);

		bool IsStaticMeshInInstancedRenderList(const std::string& meshName);
		void AddStaticMeshToInstancedRenderList(const std::string& meshName, const SMatrix& transformMatrix);
		void SwapStaticMeshInstancedRenderLists();
		void ClearSystemStaticMeshInstanceTransforms();

	public:
		[[nodiscard]] HAVTORN_API const CFullscreenTexture& GetRenderedSceneTexture() const;
		void PushRenderCommand(SRenderCommand& command);
		void SwapRenderCommandBuffers();

	public:
		HAVTORN_API static U32 NumberOfDrawCallsThisFrame;

	private:
		void Clear(SVector4 clearColor);
		void InitRenderTextures(CWindowHandler* windowHandler);
		void InitShadowmapAtlas(SVector2<F32> atlasResolution);
		void InitShadowmapLOD(SVector2<F32> topLeftCoordinate, const SVector2<F32>& widthAndHeight, const SVector2<F32>& depth, const SVector2<F32>& atlasResolution, U16 mapsInLod, U16 startIndex);
		void InitDecalResources();
		void InitPointLightResources();
		void InitSpotLightResources();
		void InitEditorResources();
		void LoadDemoSceneResources();

	private:
		void ToggleRenderPass(bool shouldToggleForwards = true);

		void VolumetricBlur();
		void RenderBloom();
		void DebugShadowAtlas();

	private:
		template<typename T>
		U16 AddVertexBuffer(const std::vector<T>& vertices);
		U16 AddIndexBuffer(const std::vector<U32>& indices);
		U16 AddMeshVertexStride(U32 stride);
		U16 AddMeshVertexOffset(U32 offset);

		std::string AddShader(const std::string& fileName, EShaderType shaderType);
		void AddInputLayout(const std::string& vsData, EInputLayoutType layoutType);
		void AddSampler(ESamplerType samplerType);
		void AddTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
		void AddViewport(SVector2<F32> topLeftCoordinate, SVector2<F32> widthAndHeight, SVector2<F32> depth);

		std::vector<U16> AddMaterial(const std::string& materialName, EMaterialConfiguration configuration);

	private:
		template<class T>
		void BindBuffer(ID3D11Buffer* buffer, T& bufferData, const std::string& bufferType)
		{
			D3D11_MAPPED_SUBRESOURCE localBufferData;
			ZeroMemory(&localBufferData, sizeof(D3D11_MAPPED_SUBRESOURCE));
			const std::string errorMessage = bufferType + " could not be bound.";
			ENGINE_HR_MESSAGE(Context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &localBufferData), errorMessage.c_str());

			memcpy(localBufferData.pData, &bufferData, sizeof(T));
			Context->Unmap(buffer, 0);
		}

		template<class T>
		void BindBuffer(ID3D11Buffer* buffer, const std::vector<T>& bufferData, const std::string& bufferType)
		{
			D3D11_MAPPED_SUBRESOURCE localBufferData;
			ZeroMemory(&localBufferData, sizeof(D3D11_MAPPED_SUBRESOURCE));
			const std::string errorMessage = bufferType + " could not be bound.";
			ENGINE_HR_MESSAGE(Context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &localBufferData), errorMessage.c_str());

			memcpy(localBufferData.pData, &bufferData[0], sizeof(T) * bufferData.size());
			Context->Unmap(buffer, 0);
		}

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

		struct SDecalBufferData
		{
			SMatrix ToWorld;
			SMatrix ToObjectSpace;
		} DecalBufferData;
		HV_ASSERT_BUFFER(SDecalBufferData)

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

	private:
		CGraphicsFramework* Framework;
		ID3D11DeviceContext* Context;
		ID3D11Buffer* FrameBuffer;
		ID3D11Buffer* ObjectBuffer;
		ID3D11Buffer* DecalBuffer;
		ID3D11Buffer* DirectionalLightBuffer;
		ID3D11Buffer* PointLightBuffer;
		ID3D11Buffer* SpotLightBuffer;
		ID3D11Buffer* ShadowmapBuffer;
		ID3D11Buffer* VolumetricLightBuffer;
		ID3D11Buffer* EmissiveBuffer;
		CRenderStateManager RenderStateManager;
		CFullscreenRenderer FullscreenRenderer;

		CFullscreenTextureFactory FullscreenTextureFactory;
		CFullscreenTexture RenderedScene;
		CFullscreenTexture Backbuffer;
		CFullscreenTexture IntermediateTexture;
		CFullscreenTexture IntermediateDepth;
		CFullscreenTexture ShadowAtlasDepth;
		CFullscreenTexture DepthCopy;

		CFullscreenTexture HalfSizeTexture;
		CFullscreenTexture QuarterSizeTexture;
		CFullscreenTexture BlurTexture1;
		CFullscreenTexture BlurTexture2;
		CFullscreenTexture VignetteTexture;

		CFullscreenTexture VolumetricAccumulationBuffer;
		CFullscreenTexture VolumetricBlurTexture;
		CFullscreenTexture SSAOBuffer;
		CFullscreenTexture SSAOBlurTexture;
		CFullscreenTexture DownsampledDepth;
		CFullscreenTexture TonemappedTexture;
		CFullscreenTexture AntiAliasedTexture;
		CGBuffer GBuffer;

		CRenderCommandHeap RenderCommandsA;
		CRenderCommandHeap RenderCommandsB;

		CRenderCommandHeap* PushToCommands;
		CRenderCommandHeap* PopFromCommands;

		SVector4 ClearColor;

		I8 RenderPassIndex;
		// Effectively used to toggle renderpasses and bloom. True == enable bloom, full render. False == disable bloom, isolated render pass
		bool DoFullRender;
		bool UseAntiAliasing;
		bool UseBrokenScreenPass;
		bool ShouldBlurVolumetricBuffer = false;

		std::vector<ID3D11VertexShader*> VertexShaders;
		std::vector<ID3D11PixelShader*> PixelShaders;
		std::vector<ID3D11SamplerState*> Samplers;
		std::vector<ID3D11Buffer*> VertexBuffers;
		std::vector<ID3D11Buffer*> IndexBuffers;
		std::vector<ID3D11InputLayout*> InputLayouts;
		std::vector<D3D11_PRIMITIVE_TOPOLOGY> Topologies;
		std::vector<D3D11_VIEWPORT> Viewports;
		std::vector<U32> MeshVertexStrides;
		std::vector<U32> MeshVertexOffsets;
		ID3D11Buffer* InstancedTransformBuffer;

		// TODO.NR: Add GUIDs to things like this
		std::unordered_map<std::string, SStaticMeshAsset> LoadedStaticMeshes;
		// NR: These are used as a way of cross-thread resource management
		std::unordered_map<std::string, std::vector<SMatrix>> SystemStaticMeshInstanceTransforms;
		std::unordered_map<std::string, std::vector<SMatrix>> RendererStaticMeshInstanceTransforms;

		ID3D11ShaderResourceView* DefaultAlbedoTexture = nullptr;
		ID3D11ShaderResourceView* DefaultNormalTexture = nullptr;
		ID3D11ShaderResourceView* DefaultMaterialTexture = nullptr;
		
		EMaterialConfiguration MaterialConfiguration = EMaterialConfiguration::AlbedoMaterialNormal_Packed;
		U8 TexturesPerMaterial = 3;

		SVector2<F32> ShadowAtlasResolution = SVector2<F32>::Zero;

		U16 InstancedMeshNumberLimit = 200;
	};

	template <typename T>
	U16 CRenderManager::AddVertexBuffer(const std::vector<T>& vertices)
	{
		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(T) * static_cast<U32>(vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA subVertexResourceData = { nullptr };
		subVertexResourceData.pSysMem = vertices.data();

		ID3D11Buffer* vertexBuffer;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&vertexBufferDesc, &subVertexResourceData, &vertexBuffer), "Vertex Buffer could not be created.");
		VertexBuffers.emplace_back(vertexBuffer);

		return static_cast<U16>(VertexBuffers.size() - 1);
	}
}
