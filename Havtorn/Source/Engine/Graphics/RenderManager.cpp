// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderManager.h"
#include "GraphicsUtilities.h"
#include "Core/GeneralUtilities.h"

#include "Engine.h"
#include "Input/InputMapper.h"
#include "Scene/World.h"

#include "ECS/ECSInclude.h"
#include "GraphicsStructs.h"
#include "GeometryPrimitives.h"
#include "FileSystem/FileHeaderDeclarations.h"

#include <algorithm>
#include <future>

#include "FileSystem/FileSystem.h"
#include "Threading/ThreadManager.h"
#include "TextureBank.h"

#include "ModelImporter.h"

#include <DirectXTex/DirectXTex.h>
#include <set>

namespace Havtorn
{
	U32 CRenderManager::NumberOfDrawCallsThisFrame = 0;

	CRenderManager::CRenderManager()
		: Framework(nullptr)
	    , Context(nullptr)
	    , FrameBuffer(nullptr)
	    , ObjectBuffer(nullptr)
		, DecalBuffer(nullptr)
	    , DirectionalLightBuffer(nullptr)
		, PointLightBuffer(nullptr)
		, SpotLightBuffer(nullptr)
		, ShadowmapBuffer(nullptr)
		, DebugShapeObjectBuffer(nullptr)
		, EmissiveBuffer(nullptr)
		, MaterialBuffer(nullptr)
		, InstancedTransformBuffer(nullptr)
		, VolumetricLightBuffer(nullptr)
	    , PushToCommands(&RenderCommandsA)
	    , PopFromCommands(&RenderCommandsB)
	    , ClearColor(0.5f, 0.5f, 0.5f, 1.0f)
	{
	}

	CRenderManager::~CRenderManager()
	{
		Release();
	}

	bool CRenderManager::Init(CGraphicsFramework* framework, CWindowHandler* windowHandler)
	{
		Framework = framework;
		Context = Framework->GetContext();

		ENGINE_ERROR_BOOL_MESSAGE(FullscreenRenderer.Init(framework), "Failed to Init Fullscreen Renderer.");
		ENGINE_ERROR_BOOL_MESSAGE(FullscreenTextureFactory.Init(framework), "Failed to Init Fullscreen Texture Factory.");
		ENGINE_ERROR_BOOL_MESSAGE(RenderStateManager.Init(framework), "Failed to Init Render State Manager.");

		ID3D11Texture2D* backbufferTexture = framework->GetBackbufferTexture();
		ENGINE_ERROR_BOOL_MESSAGE(backbufferTexture, "Backbuffer Texture is null.");

		Backbuffer = FullscreenTextureFactory.CreateTexture(backbufferTexture);
		InitRenderTextures(windowHandler);

		InitDataBuffers();

		// Load default resources
		InitVertexShadersAndInputLayouts();
		InitPixelShaders();
		InitGeometryShaders();
		InitSamplers();
		InitVertexBuffers();
		InitIndexBuffers();
		InitTopologies();
		InitMeshVertexStrides();
		InitMeshVertexOffset();

		InitEditorResources();
		LoadDemoSceneResources();

		RenderFunctions[ERenderCommandType::ShadowAtlasPrePassDirectional] = std::bind(&CRenderManager::ShadowAtlasPrePassDirectional, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::ShadowAtlasPrePassPoint] = std::bind(&CRenderManager::ShadowAtlasPrePassPoint, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::ShadowAtlasPrePassSpot] = std::bind(&CRenderManager::ShadowAtlasPrePassSpot, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::CameraDataStorage] = std::bind(&CRenderManager::CameraDataStorage, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GBufferDataInstanced] = std::bind(&CRenderManager::GBufferDataInstanced, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GBufferSpriteInstanced] = std::bind(&CRenderManager::GBufferSpriteInstanced, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DecalDepthCopy] = std::bind(&CRenderManager::DecalDepthCopy, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DeferredDecal] = std::bind(&CRenderManager::DeferredDecal, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PreLightingPass] = std::bind(&CRenderManager::PreLightingPass, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DeferredLightingDirectional] = std::bind(&CRenderManager::DeferredLightingDirectional, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DeferredLightingPoint] = std::bind(&CRenderManager::DeferredLightingPoint, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DeferredLightingSpot] = std::bind(&CRenderManager::DeferredLightingSpot, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PostBaseLightingPass] = std::bind(&CRenderManager::PostBaseLightingPass, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::VolumetricLightingDirectional] = std::bind(&CRenderManager::VolumetricLightingDirectional, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::VolumetricLightingPoint] = std::bind(&CRenderManager::VolumetricLightingPoint, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::VolumetricLightingSpot] = std::bind(&CRenderManager::VolumetricLightingSpot, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::VolumetricBufferBlurPass] = std::bind(&CRenderManager::VolumetricBlur, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::ForwardTransparency] = std::bind(&CRenderManager::ForwardTransparency, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::ScreenSpaceSprite] = std::bind(&CRenderManager::ScreenSpaceSprite, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::Bloom] = std::bind(&CRenderManager::RenderBloom, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::Tonemapping] = std::bind(&CRenderManager::Tonemapping, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PreDebugShape] = std::bind(&CRenderManager::PreDebugShapes, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PostToneMappingUseDepth] = std::bind(&CRenderManager::PostTonemappingUseDepth, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PostToneMappingIgnoreDepth] = std::bind(&CRenderManager::PostTonemappingIgnoreDepth, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DebugShapeUseDepth] = std::bind(&CRenderManager::DebugShapes, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DebugShapeIgnoreDepth] = std::bind(&CRenderManager::DebugShapes, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::AntiAliasing] = std::bind(&CRenderManager::AntiAliasing, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GammaCorrection] = std::bind(&CRenderManager::GammaCorrection, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::RendererDebug] = std::bind(&CRenderManager::RendererDebug, this, std::placeholders::_1);

		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassForward).AddMember(this, &CRenderManager::CycleRenderPass);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassBackward).AddMember(this, &CRenderManager::CycleRenderPass);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassReset).AddMember(this, &CRenderManager::CycleRenderPass);

		return true;
	}

	bool CRenderManager::ReInit(CGraphicsFramework* framework, CWindowHandler* windowHandler)
	{
		ENGINE_ERROR_BOOL_MESSAGE(RenderStateManager.Init(framework), "Failed to Init Render State Manager.");

		ID3D11Texture2D* backbufferTexture = framework->GetBackbufferTexture();
		ENGINE_ERROR_BOOL_MESSAGE(backbufferTexture, "Backbuffer Texture is null.");

		Backbuffer = FullscreenTextureFactory.CreateTexture(backbufferTexture);
		InitRenderTextures(windowHandler);

		return true;
	}

	void CRenderManager::InitRenderTextures(CWindowHandler* windowHandler)
	{
		RenderedScene = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		LitScene = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		IntermediateDepth = FullscreenTextureFactory.CreateDepth(windowHandler->GetResolution(), DXGI_FORMAT_R24G8_TYPELESS);

		ShadowAtlasResolution = {8192.0f, 8192.0f};
		InitShadowmapAtlas(ShadowAtlasResolution);

		DepthCopy = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R32_FLOAT);
		DownsampledDepth = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R32_FLOAT);

		IntermediateTexture = FullscreenTextureFactory.CreateTexture(ShadowAtlasResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);

		HalfSizeTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);
		QuarterSizeTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 4.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);
		BlurTexture1 = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		BlurTexture2 = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		VignetteTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);

		VolumetricAccumulationBuffer = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);
		VolumetricBlurTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);

		SSAOBuffer = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);
		SSAOBlurTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);

		TonemappedTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		AntiAliasedTexture = FullscreenTextureFactory.CreateTexture(windowHandler->GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT);
		GBuffer = FullscreenTextureFactory.CreateGBuffer(windowHandler->GetResolution());
	}

	void CRenderManager::InitShadowmapAtlas(SVector2<F32> atlasResolution)
	{
		ShadowAtlasDepth = FullscreenTextureFactory.CreateDepth(atlasResolution, DXGI_FORMAT_R32_TYPELESS);

		// LOD 1
		U16 mapsInLod = 8;
		SVector2<F32> topLeftCoordinate = SVector2<F32>::Zero;
		SVector2<F32> widthAndHeight = atlasResolution * (1.0f / (mapsInLod / 2));
		const SVector2<F32> depth = { 0.0f, 1.0f };
		InitShadowmapLOD(topLeftCoordinate, widthAndHeight, depth, atlasResolution, mapsInLod, 0);

		// LOD 2
		mapsInLod = 16;
		topLeftCoordinate = { 0.0f, atlasResolution.Y * 0.5f };
		widthAndHeight = atlasResolution * (1.0f / (mapsInLod / 2));
		InitShadowmapLOD(topLeftCoordinate, widthAndHeight, depth, atlasResolution, mapsInLod, 8);

		// LOD 3
		mapsInLod = 32;
		topLeftCoordinate = {0.0f, atlasResolution.Y * 0.75f };
		widthAndHeight = atlasResolution * (1.0f / (mapsInLod / 2));
		InitShadowmapLOD(topLeftCoordinate, widthAndHeight, depth, atlasResolution, mapsInLod, 24);

		// LOD 4
		mapsInLod = 128;
		topLeftCoordinate = { 0.0f, atlasResolution.Y * 0.875f };
		widthAndHeight = atlasResolution * (1.0f / (mapsInLod / 4));
		InitShadowmapLOD(topLeftCoordinate, widthAndHeight, depth, atlasResolution, mapsInLod, 56);
	}

	void CRenderManager::InitShadowmapLOD(SVector2<F32> topLeftCoordinate, const SVector2<F32>& widthAndHeight, const SVector2<F32>& depth, const SVector2<F32>& atlasResolution, U16 mapsInLod, U16 startIndex)
	{
		const float startingYCoordinate = topLeftCoordinate.Y;
		const U16 mapsPerRow = static_cast<U16>(atlasResolution.X / widthAndHeight.X);
		for (U16 i = startIndex; i < startIndex + mapsInLod; i++)
		{
			const U16 relativeIndex = i - startIndex;
			topLeftCoordinate.X = static_cast<U16>(relativeIndex % mapsPerRow) * widthAndHeight.X;
			topLeftCoordinate.Y = startingYCoordinate + static_cast<U16>(relativeIndex / mapsPerRow) * widthAndHeight.Y;
			AddViewport(topLeftCoordinate, widthAndHeight, depth);
		}
	}

	void CRenderManager::InitVertexShadersAndInputLayouts()
	{
		AddShader("Shaders/FullscreenVertexShader_VS.cso", EShaderType::Vertex);
		
		std::string vsData = AddShader("Shaders/DeferredStaticMesh_VS.cso", EShaderType::Vertex);
		AddInputLayout(vsData, EInputLayoutType::Pos3Nor3Tan3Bit3UV2);
		
		vsData = AddShader("Shaders/DeferredInstancedMesh_VS.cso", EShaderType::Vertex);
		AddInputLayout(vsData, EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans);
		
		AddShader("Shaders/Decal_VS.cso", EShaderType::Vertex);

		vsData = AddShader("Shaders/PointLight_VS.cso", EShaderType::Vertex);
		AddInputLayout(vsData, EInputLayoutType::Pos4);

		AddShader("Shaders/EditorPreview_VS.cso", EShaderType::Vertex);
		
		AddShader("Shaders/Line_VS.cso", EShaderType::Vertex);

		AddShader("Shaders/SpriteScreenSpace_VS.cso", EShaderType::Vertex);
		
		vsData = AddShader("Shaders/SpriteWorldSpace_VS.cso", EShaderType::Vertex);
		AddInputLayout(vsData, EInputLayoutType::TransUVRectColor);
	}

	void CRenderManager::InitPixelShaders()
	{
		AddShader("Shaders/GBuffer_PS.cso", EShaderType::Pixel);

		AddShader("Shaders/Decal_Albedo_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/Decal_Material_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/Decal_Normal_PS.cso", EShaderType::Pixel);
		
		AddShader("Shaders/DeferredLightDirectionalAndEnvironment_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/DeferredLightPoint_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/DeferredLightSpot_PS.cso", EShaderType::Pixel);
		
		AddShader("Shaders/DeferredLightDirectionalVolumetric_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/DeferredLightPointVolumetric_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/DeferredLightSpotVolumetric_PS.cso", EShaderType::Pixel);
		
		AddShader("Shaders/EditorPreview_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/Line_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/SpriteScreenSpace_PS.cso", EShaderType::Pixel);
		AddShader("Shaders/SpriteWorldSpace_PS.cso", EShaderType::Pixel);
	}

	void CRenderManager::InitGeometryShaders()
	{
		AddShader("Shaders/Line_GS.cso", EShaderType::Geometry);
		AddShader("Shaders/SpriteScreenSpace_GS.cso", EShaderType::Geometry);
		AddShader("Shaders/SpriteWorldSpace_GS.cso", EShaderType::Geometry);
	}

	void CRenderManager::InitSamplers()
	{
		AddSampler(ESamplerType::Wrap);
		AddSampler(ESamplerType::Border);
	}

	void CRenderManager::InitVertexBuffers()
	{
		AddVertexBuffer(GeometryPrimitives::DecalProjector);
		AddVertexBuffer(GeometryPrimitives::PointLightCube);
		AddVertexBuffer(GeometryPrimitives::Icosphere.Vertices);
		AddVertexBuffer(GeometryPrimitives::Line.Vertices);
		AddVertexBuffer(GeometryPrimitives::Pyramid.Vertices);
		AddVertexBuffer(GeometryPrimitives::BoundingBox.Vertices);
		AddVertexBuffer(GeometryPrimitives::Camera.Vertices);
		AddVertexBuffer(GeometryPrimitives::Circle8.Vertices);
		AddVertexBuffer(GeometryPrimitives::Circle16.Vertices);
		AddVertexBuffer(GeometryPrimitives::Circle32.Vertices);
		AddVertexBuffer(GeometryPrimitives::HalfCircle16.Vertices);
		AddVertexBuffer(GeometryPrimitives::Grid.Vertices);
		AddVertexBuffer(GeometryPrimitives::Axis.Vertices);
		AddVertexBuffer(GeometryPrimitives::Octahedron.Vertices);
		AddVertexBuffer(GeometryPrimitives::Square.Vertices);
		AddVertexBuffer(GeometryPrimitives::UVSphere.Vertices);
	}

	void CRenderManager::InitIndexBuffers()
	{
		AddIndexBuffer(GeometryPrimitives::DecalProjectorIndices);
		AddIndexBuffer(GeometryPrimitives::PointLightCubeIndices);
		AddIndexBuffer(GeometryPrimitives::Icosphere.Indices);
		AddIndexBuffer(GeometryPrimitives::Line.Indices);
		AddIndexBuffer(GeometryPrimitives::Pyramid.Indices);
		AddIndexBuffer(GeometryPrimitives::BoundingBox.Indices);
		AddIndexBuffer(GeometryPrimitives::Camera.Indices);
		AddIndexBuffer(GeometryPrimitives::Circle8.Indices);
		AddIndexBuffer(GeometryPrimitives::Circle16.Indices);
		AddIndexBuffer(GeometryPrimitives::Circle32.Indices);
		AddIndexBuffer(GeometryPrimitives::HalfCircle16.Indices);
		AddIndexBuffer(GeometryPrimitives::Grid.Indices);
		AddIndexBuffer(GeometryPrimitives::Axis.Indices);
		AddIndexBuffer(GeometryPrimitives::Octahedron.Indices);
		AddIndexBuffer(GeometryPrimitives::Square.Indices);
		AddIndexBuffer(GeometryPrimitives::UVSphere.Indices);
	}

	void CRenderManager::InitTopologies()
	{
		AddTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		AddTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		AddTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	}

	void CRenderManager::InitMeshVertexStrides()
	{
		AddMeshVertexStride(sizeof(SStaticMeshVertex));
		AddMeshVertexStride(sizeof(SPositionVertex));
	}

	void CRenderManager::InitMeshVertexOffset()
	{
		AddMeshVertexOffset(0);
	}

	void CRenderManager::InitEditorResources()
	{}

	void CRenderManager::LoadDemoSceneResources()
	{
	}

	void CRenderManager::Render()
	{
		while (CThreadManager::RunRenderThread)
		{
			std::unique_lock<std::mutex> uniqueLock(CThreadManager::RenderMutex);
			CThreadManager::RenderCondition.wait(uniqueLock, [] 
				{ return CThreadManager::RenderThreadStatus == ERenderThreadStatus::ReadyToRender; });

			GTime::BeginTracking(ETimerCategory::GPU);

			ShouldBlurVolumetricBuffer = false;
			CRenderManager::NumberOfDrawCallsThisFrame = 0;
			RenderStateManager.SetAllDefault();

			Backbuffer.ClearTexture();
			ShadowAtlasDepth.ClearDepth();
			SSAOBuffer.ClearTexture();

			RenderedScene.ClearTexture();
			LitScene.ClearTexture();
			IntermediateTexture.ClearTexture();
			IntermediateDepth.ClearDepth();
			VolumetricAccumulationBuffer.ClearTexture();
			GBuffer.ClearTextures(ClearColor);

			ShadowAtlasDepth.SetAsDepthTarget(&IntermediateTexture);

			const U16 commandsInHeap = static_cast<U16>(PopFromCommands->size());
			for (U16 i = 0; i < commandsInHeap; ++i)
			{
				SRenderCommand currentCommand = PopFromCommands->top();
				RenderFunctions[currentCommand.Type](currentCommand);
				PopFromCommands->pop();
			}

			CheckIsolatedRenderPass();

			// RenderedScene should be complete as that is the texture we send to the viewport
			Backbuffer.SetAsActiveTarget();
			RenderedScene.SetAsResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

			GTime::EndTracking(ETimerCategory::GPU);

			CThreadManager::RenderThreadStatus = ERenderThreadStatus::PostRender;
			uniqueLock.unlock();
			CThreadManager::RenderCondition.notify_one();
		}
	}

	void CRenderManager::Release()
	{
		Clear(ClearColor);
		GEngine::Instance->Framework->GetContext()->OMSetRenderTargets(0, 0, 0);
		GEngine::Instance->Framework->GetContext()->OMGetDepthStencilState(0, 0);
		GEngine::Instance->Framework->GetContext()->ClearState();

		//Backbuffer.ReleaseTexture();
		//myIntermediateTexture.ReleaseTexture();
		//myIntermediateDepth.ReleaseDepth();
		//myLuminanceTexture.ReleaseTexture();
		//myHalfSizeTexture.ReleaseTexture();
		//myQuarterSizeTexture.ReleaseTexture();
		//myBlurTexture1.ReleaseTexture();
		//myBlurTexture2.ReleaseTexture();
		//myVignetteTexture.ReleaseTexture();
		//myVignetteOverlayTexture.ReleaseTexture();
		//myDeferredLightingTexture.ReleaseTexture();

		//myEnvironmentShadowDepth.ReleaseDepth();
		//myBoxLightShadowDepth.ReleaseDepth();
		//myDepthCopy.ReleaseDepth();
		//myDownsampledDepth.ReleaseDepth();

		//myVolumetricAccumulationBuffer.ReleaseTexture();
		//myVolumetricBlurTexture.ReleaseTexture();
		//myTonemappedTexture.ReleaseTexture();
		//myAntiAliasedTexture.ReleaseTexture();

		//myGBuffer.ReleaseResources();
		//myGBufferCopy.ReleaseResources();
	}

	void CRenderManager::LoadStaticMeshComponent(const std::string& filePath, SStaticMeshComponent* outStaticMeshComponent)
	{
		SStaticMeshAsset asset;
		if (!LoadedStaticMeshes.contains(filePath))
		{
			// Asset Loading
			const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
			char* data = new char[fileSize];

			GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));

			SStaticModelFileHeader assetFile;
			assetFile.Deserialize(data);
			asset = SStaticMeshAsset(assetFile);

			for (U16 i = 0; i < assetFile.NumberOfMeshes; i++)
			{
				// TODO.NR: Check for existing buffers
				asset.DrawCallData[i].VertexBufferIndex = AddVertexBuffer(assetFile.Meshes[i].Vertices);
				asset.DrawCallData[i].IndexBufferIndex = AddIndexBuffer(assetFile.Meshes[i].Indices);
				asset.DrawCallData[i].VertexStrideIndex = /*AddMeshVertexStride(static_cast<U32>(sizeof(SStaticMeshVertex)))*/0;
				asset.DrawCallData[i].VertexOffsetIndex = /*AddMeshVertexOffset(0)*/0;
			}

			// NR: Mesh name will be much easier to handle
			LoadedStaticMeshes.emplace(UGeneralUtils::ExtractFileNameFromPath(filePath), asset);
			delete[] data;
		}
		else
		{
			asset = LoadedStaticMeshes.at(filePath);
		}

		// NR: Components initialized by AssetRegistry and Rendermanager have dynamically sized sized, need to serialize and deserialize them in another way
		outStaticMeshComponent->Name = UGeneralUtils::ExtractFileNameFromPath(filePath);
		outStaticMeshComponent->NumberOfMaterials = asset.NumberOfMaterials;

		// Geometry
		outStaticMeshComponent->VertexShaderIndex = static_cast<U8>(EVertexShaders::StaticMesh);
		outStaticMeshComponent->PixelShaderIndex = static_cast<U8>(EPixelShaders::GBuffer);
		outStaticMeshComponent->InputLayoutIndex = static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2);
		outStaticMeshComponent->SamplerIndex = static_cast<U8>(ESamplers::DefaultWrap);
		outStaticMeshComponent->TopologyIndex = static_cast<U8>(ETopologies::TriangleList);
		outStaticMeshComponent->DrawCallData = asset.DrawCallData;
	}

	void CRenderManager::LoadMaterialComponent(const std::vector<std::string>& materialPaths, SMaterialComponent* outMaterialComponent)
	{
		SGraphicsMaterialAsset asset;
		for (const std::string& materialPath : materialPaths)
		{
			const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(materialPath);
			char* data = new char[fileSize];

			GEngine::GetFileSystem()->Deserialize(materialPath, data, static_cast<U32>(fileSize));

			SMaterialAssetFileHeader assetFile;
			assetFile.Deserialize(data);
			asset = SGraphicsMaterialAsset(assetFile);
			
			SEngineGraphicsMaterial& material = outMaterialComponent->Materials.emplace_back();
			material = asset.Material;
		}
	}

	void CRenderManager::LoadDecalComponent(const std::vector<std::string>& texturePaths, SDecalComponent* outDecalComponent)
	{
		outDecalComponent->TextureReferences.clear();
		auto textureBank = GEngine::GetTextureBank();

		for (const std::string& texturePath: texturePaths)
		{
			outDecalComponent->TextureReferences.emplace_back(static_cast<U16>(textureBank->GetTextureIndex(texturePath)));
		}
	}

	void CRenderManager::LoadEnvironmentLightComponent(const std::string& ambientCubemapTexturePath, SEnvironmentLightComponent* outEnvironmentLightComponent)
	{
		auto textureBank = GEngine::GetTextureBank();
		outEnvironmentLightComponent->AmbientCubemapReference = static_cast<U16>(textureBank->GetTextureIndex(ambientCubemapTexturePath));
	}

	void CRenderManager::LoadSpriteComponent(const std::string& filePath, SSpriteComponent* outSpriteComponent)
	{
		auto textureBank = GEngine::GetTextureBank();
		outSpriteComponent->TextureIndex = textureBank->GetTextureIndex(filePath);
	}

	SVector2<F32> CRenderManager::GetShadowAtlasResolution() const
	{
		return ShadowAtlasResolution;
	}

	bool CRenderManager::TryLoadStaticMeshComponent(const std::string& fileName, SStaticMeshComponent* outStaticMeshComponent) const
	{
		SStaticMeshAsset asset;

		if (!LoadedStaticMeshes.contains(fileName))
			return false;
		else
			asset = LoadedStaticMeshes.at(fileName);

		outStaticMeshComponent->Name = fileName;
		outStaticMeshComponent->NumberOfMaterials = asset.NumberOfMaterials;

		// Geometry
		outStaticMeshComponent->VertexShaderIndex = static_cast<U8>(EVertexShaders::StaticMesh);
		outStaticMeshComponent->PixelShaderIndex = static_cast<U8>(EPixelShaders::GBuffer);
		outStaticMeshComponent->InputLayoutIndex = static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2);
		outStaticMeshComponent->SamplerIndex = static_cast<U8>(ESamplers::DefaultWrap);
		outStaticMeshComponent->TopologyIndex = static_cast<U8>(ETopologies::TriangleList);
		outStaticMeshComponent->DrawCallData = asset.DrawCallData;

		return true;
	}

	bool CRenderManager::TryReplaceMaterialOnComponent(const std::string& filePath, U8 materialIndex, SMaterialComponent* outMaterialComponent) const
	{
		if (!GEngine::GetFileSystem()->DoesFileExist(filePath))
		{
			HV_LOG_ERROR("File not found when trying to replace material: %s", filePath.c_str());
			return false;
		}

		if (materialIndex >= static_cast<U8>(outMaterialComponent->Materials.size()))
		{
			HV_LOG_ERROR("Material index out of bounds when trying to replace material: %s", filePath.c_str());
			return false;
		}

		const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
		char* data = new char[fileSize];

		GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));

		SMaterialAssetFileHeader assetFile;
		assetFile.Deserialize(data);
		SGraphicsMaterialAsset asset(assetFile);

		outMaterialComponent->Materials[materialIndex] = asset.Material;
		return true;
	}

	void* CRenderManager::RenderStaticMeshAssetTexture(const std::string& filePath)
	{
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = static_cast<U16>(256.0f);
		textureDesc.Height = static_cast<U16>(256.0f);
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		D3D11_TEXTURE2D_DESC depthStencilDesc = { 0 };
		depthStencilDesc.Width = static_cast<U16>(256.0f);
		depthStencilDesc.Height = static_cast<U16>(256.0f);
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		depthStencilViewDesc.Flags = 0;

		ID3D11Texture2D* depthStencilBuffer;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer), "Texture could not be created.");
		
		ID3D11DepthStencilView* depthStencilView;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView), "Depth could not be created.");

		ID3D11Texture2D* texture;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &texture), "Could not create Fullscreen Texture2D");

		ID3D11ShaderResourceView* shaderResource;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateShaderResourceView(texture, nullptr, &shaderResource), "Could not create Fullscreen Shader Resource View.");

		ID3D11RenderTargetView* renderTarget;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateRenderTargetView(texture, nullptr, &renderTarget), "Could not create Fullcreen Render Target View.");

		D3D11_VIEWPORT* viewport;
		D3D11_TEXTURE2D_DESC textureDescription;
		texture->GetDesc(&textureDescription);
		viewport = new D3D11_VIEWPORT({ 0.0f, 0.0f, static_cast<F32>(textureDescription.Width), static_cast<F32>(textureDescription.Height), 0.0f, 1.0f });

		// TODO.NR: Figure out why the depth doesn't work
		Context->OMSetRenderTargets(1, &renderTarget, nullptr);
		Context->RSSetViewports(1, viewport);

		STransform camTransform;
		camTransform.Translate(SVector4::Right * 1.5f);
		camTransform.Translate(SVector4::Backward * 2.5f);
		camTransform.Translate(SVector4::Up * 2.0f);
		camTransform.Rotate({UMath::DegToRad(-30.0f), UMath::DegToRad(30.0f), 0.0f});
		SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), (16.0f / 9.0f), 0.1f, 10.0f);

		FrameBufferData.ToCameraFromWorld = camTransform.GetMatrix().FastInverse();
		FrameBufferData.ToWorldFromCamera = camTransform.GetMatrix();
		FrameBufferData.ToProjectionFromCamera = camProjection;
		FrameBufferData.ToCameraFromProjection = camProjection.Inverse();
		FrameBufferData.CameraPosition = camTransform.GetMatrix().GetTranslation4();
		BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");

		Context->VSSetConstantBuffers(0, 1, &FrameBuffer);
		Context->PSSetConstantBuffers(0, 1, &FrameBuffer);

		ObjectBufferData.ToWorldFromObject = SMatrix();
		BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

		Havtorn::CScene tempScene;
		auto entity = tempScene.GetNewEntity();
		auto& staticMeshComp = tempScene.GetStaticMeshComponents()[tempScene.GetSceneIndex(*entity)];
		LoadStaticMeshComponent(filePath, &staticMeshComp);

		Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
		Context->IASetPrimitiveTopology(Topologies[staticMeshComp.TopologyIndex]);
		Context->IASetInputLayout(InputLayouts[staticMeshComp.InputLayoutIndex]);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::EditorPreview)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::EditorPreview)], nullptr, 0);

		ID3D11SamplerState* sampler = Samplers[staticMeshComp.SamplerIndex];
		Context->PSSetSamplers(0, 1, &sampler);

		for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp.DrawCallData.size()); drawCallIndex++)
		{
			const SDrawCallData& drawData = staticMeshComp.DrawCallData[drawCallIndex];
			ID3D11Buffer* vertexBuffer = VertexBuffers[drawData.VertexBufferIndex];
			Context->IASetVertexBuffers(0, 1, &vertexBuffer, &MeshVertexStrides[drawData.VertexStrideIndex], &MeshVertexOffsets[drawData.VertexStrideIndex]);
			Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
			Context->DrawIndexed(drawData.IndexCount, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		tempScene.TryRemoveEntity(*entity);
		delete viewport;
		renderTarget->Release();
		texture->Release();
		depthStencilView->Release();
		depthStencilBuffer->Release();

		return std::move((void*)shaderResource);
	}

	void* CRenderManager::GetTextureAssetTexture(const std::string& filePath)
	{
		// Asset Loading
		const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
		char* data = new char[fileSize];

		GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));

		STextureFileHeader assetFile;
		assetFile.Deserialize(data);
		STextureAsset asset = STextureAsset(assetFile, Framework->GetDevice());

		return asset.ShaderResourceView;
	}

	void* CRenderManager::RenderMaterialAssetTexture(const std::string& filePath)
	{
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = static_cast<U16>(256.0f);
		textureDesc.Height = static_cast<U16>(256.0f);
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		D3D11_TEXTURE2D_DESC depthStencilDesc = { 0 };
		depthStencilDesc.Width = static_cast<U16>(256.0f);
		depthStencilDesc.Height = static_cast<U16>(256.0f);
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		depthStencilViewDesc.Flags = 0;

		ID3D11Texture2D* depthStencilBuffer;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer), "Texture could not be created.");

		ID3D11DepthStencilView* depthStencilView;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView), "Depth could not be created.");

		ID3D11Texture2D* texture;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &texture), "Could not create Fullscreen Texture2D");

		ID3D11ShaderResourceView* shaderResource;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateShaderResourceView(texture, nullptr, &shaderResource), "Could not create Fullscreen Shader Resource View.");

		ID3D11RenderTargetView* renderTarget;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateRenderTargetView(texture, nullptr, &renderTarget), "Could not create Fullcreen Render Target View.");

		D3D11_VIEWPORT* viewport;
		D3D11_TEXTURE2D_DESC textureDescription;
		texture->GetDesc(&textureDescription);
		viewport = new D3D11_VIEWPORT({ 0.0f, 0.0f, static_cast<F32>(textureDescription.Width), static_cast<F32>(textureDescription.Height), 0.0f, 1.0f });

		CGBuffer gBuffer = FullscreenTextureFactory.CreateGBuffer({ static_cast<F32>(textureDescription.Width), static_cast<F32>(textureDescription.Height) });
		gBuffer.SetAsActiveTarget();
		Context->RSSetViewports(1, viewport);

		STransform camTransform;
		constexpr F32 zoomMultiplier = 0.72f;
		camTransform.Translate(SVector4::Backward * 1.8f * zoomMultiplier);
		camTransform.Translate(SVector4::Right * 1.08f * zoomMultiplier);
		camTransform.Translate(SVector4::Up * 1.2f * zoomMultiplier);
		camTransform.Rotate({ UMath::DegToRad(-30.0f), UMath::DegToRad(30.0f), 0.0f });
		SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), 1.0f, 0.01f, 10.0f);

		FrameBufferData.ToCameraFromWorld = camTransform.GetMatrix().FastInverse();
		FrameBufferData.ToWorldFromCamera = camTransform.GetMatrix();
		FrameBufferData.ToProjectionFromCamera = camProjection;
		FrameBufferData.ToCameraFromProjection = camProjection.Inverse();
		FrameBufferData.CameraPosition = camTransform.GetMatrix().GetTranslation4();
		BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");

		Context->VSSetConstantBuffers(0, 1, &FrameBuffer);
		Context->PSSetConstantBuffers(0, 1, &FrameBuffer);

		ObjectBufferData.ToWorldFromObject = SMatrix();
		BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

		CScene tempScene;
		auto entity = tempScene.GetNewEntity();
		auto& materialComp = tempScene.GetMaterialComponents()[tempScene.GetSceneIndex(*entity)];
		LoadMaterialComponent({ filePath }, &materialComp);

		Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
		Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2)]);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::StaticMesh)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::GBuffer)], nullptr, 0);

		ID3D11SamplerState* sampler = Samplers[static_cast<U8>(ESamplers::DefaultWrap)];
		Context->PSSetSamplers(0, 1, &sampler);

		ID3D11Buffer* vertexBuffer = VertexBuffers[static_cast<U8>(EVertexBufferPrimitives::Icosphere)];
		Context->IASetVertexBuffers(0, 1, &vertexBuffer, &MeshVertexStrides[0], &MeshVertexOffsets[0]);
		Context->IASetIndexBuffer(IndexBuffers[static_cast<U8>(EIndexBufferPrimitives::Icosphere)], DXGI_FORMAT_R32_UINT, 0);
			
		auto textureBank = GEngine::GetTextureBank();
		std::vector<ID3D11ShaderResourceView*> resourceViewPointers;

		std::map<F32, F32> textureIndices;
		auto findTextureByIndex = [&](SRuntimeGraphicsMaterialProperty& bufferProperty)
		{
			if (bufferProperty.TextureChannelIndex > -1.0f)
			{
				if (!textureIndices.contains(bufferProperty.TextureIndex))
				{
					resourceViewPointers.emplace_back(textureBank->GetTexture(static_cast<U32>(bufferProperty.TextureIndex)));
					textureIndices.emplace(bufferProperty.TextureIndex, static_cast<F32>(resourceViewPointers.size() - 1));
				}

				bufferProperty.TextureIndex = textureIndices[bufferProperty.TextureIndex];
			}
		};

		MaterialBufferData = SMaterialBufferData(materialComp.Materials[0]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::AlbedoR)]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::AlbedoG)]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::AlbedoB)]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::AlbedoA)]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::NormalX)]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::NormalY)]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::NormalZ)]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::AmbientOcclusion)]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::Metalness)]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::Roughness)]);
		findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::Emissive)]);

		BindBuffer(MaterialBuffer, MaterialBufferData, "Material Buffer");

		Context->PSSetShaderResources(5, static_cast<U32>(resourceViewPointers.size()), resourceViewPointers.data());
		Context->PSSetConstantBuffers(8, 1, &MaterialBuffer);
		
		Context->DrawIndexed(static_cast<U32>(GeometryPrimitives::Icosphere.Indices.size()), 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;

		// ======== Lighting =========
		Context->OMSetRenderTargets(1, &renderTarget, nullptr);

		auto& environmentLightComp = tempScene.GetEnvironmentLightComponents()[tempScene.GetSceneIndex(*entity)];
		auto& directionalLightComp = tempScene.GetDirectionalLightComponents()[tempScene.GetSceneIndex(*entity)];

		LoadEnvironmentLightComponent("Assets/Textures/Cubemaps/Skybox.hva", &environmentLightComp);

		directionalLightComp.Direction = { -1.0f, 0.0f, 0.0f, 0.0f };
		directionalLightComp.Color = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 0.25f };
		directionalLightComp.ShadowmapView.ShadowmapViewportIndex = 0;
		directionalLightComp.ShadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(directionalLightComp.ShadowViewSize.X, directionalLightComp.ShadowViewSize.Y, directionalLightComp.ShadowNearAndFarPlane.X, directionalLightComp.ShadowNearAndFarPlane.Y);	

		gBuffer.SetAllAsResources(1);
		IntermediateDepth.SetAsResourceOnSlot(21);
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);

		// add Alpha blend PS shader

		ShadowAtlasDepth.SetAsResourceOnSlot(22);
		SSAOBlurTexture.SetAsResourceOnSlot(23);

		auto cubemapTexture = GEngine::GetTextureBank()->GetTexture(environmentLightComp.AmbientCubemapReference);
		Context->PSSetShaderResources(0, 1, &cubemapTexture);

		// Update lightbufferdata and fill lightbuffer
		DirectionalLightBufferData.DirectionalLightDirection = directionalLightComp.Direction;
		DirectionalLightBufferData.DirectionalLightColor = directionalLightComp.Color;
		BindBuffer(DirectionalLightBuffer, DirectionalLightBufferData, "Light Buffer");
		Context->PSSetConstantBuffers(2, 1, &DirectionalLightBuffer);

		ShadowmapBufferData.ToShadowmapView = directionalLightComp.ShadowmapView.ShadowViewMatrix;
		ShadowmapBufferData.ToShadowmapProjection = directionalLightComp.ShadowmapView.ShadowProjectionMatrix;
		ShadowmapBufferData.ShadowmapPosition = directionalLightComp.ShadowmapView.ShadowPosition;

		const auto& shadowmapViewport = Viewports[directionalLightComp.ShadowmapView.ShadowmapViewportIndex];
		ShadowmapBufferData.ShadowmapResolution = { shadowmapViewport.Width, shadowmapViewport.Height };
		ShadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
		ShadowmapBufferData.ShadowmapStartingUV = { shadowmapViewport.TopLeftX / ShadowAtlasResolution.X, shadowmapViewport.TopLeftY / ShadowAtlasResolution.Y };
		ShadowmapBufferData.ShadowTestTolerance = 0.001f;

		BindBuffer(ShadowmapBuffer, ShadowmapBufferData, "Shadowmap Buffer");
		Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

		// Emissive Post Processing 
		EmissiveBufferData.EmissiveStrength = FullscreenRenderer.PostProcessingBufferData.EmissiveStrength;
		BindBuffer(EmissiveBuffer, EmissiveBufferData, "Emissive Buffer");
		Context->PSSetConstantBuffers(7, 1, &EmissiveBuffer);

		Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Context->IASetInputLayout(nullptr);
		Context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::Fullscreen)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DeferredDirectional)], nullptr, 0);

		Context->Draw(3, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;

		// TODO.NR: Make temp scene outside, in EditorResourceManager and send it in to these functions. Need default lighting
		tempScene.TryRemoveEntity(*entity);
		delete viewport;
		renderTarget->Release();
		texture->Release();
		depthStencilView->Release();
		depthStencilBuffer->Release();

		ID3D11ShaderResourceView* nullView = NULL;
		Context->PSSetShaderResources(21, 1, &nullView);
		Context->PSSetShaderResources(22, 1, &nullView);
		Context->PSSetShaderResources(23, 1, &nullView);

		return std::move((void*)shaderResource);
	}

	bool CRenderManager::IsStaticMeshInInstancedRenderList(const std::string& meshName)
	{
		return SystemStaticMeshInstanceTransforms.contains(meshName);
	}

	void CRenderManager::AddStaticMeshToInstancedRenderList(const std::string& meshName, const SMatrix& transformMatrix)
	{
		if (!SystemStaticMeshInstanceTransforms.contains(meshName))
			SystemStaticMeshInstanceTransforms.emplace(meshName, std::vector<SMatrix>());

		SystemStaticMeshInstanceTransforms[meshName].emplace_back(transformMatrix);
	}

	void CRenderManager::SwapStaticMeshInstancedRenderLists()
	{
		std::swap(SystemStaticMeshInstanceTransforms, RendererStaticMeshInstanceTransforms);
	}

	void CRenderManager::ClearSystemStaticMeshInstanceTransforms()
	{
		return SystemStaticMeshInstanceTransforms.clear();
	}

	bool CRenderManager::IsSpriteInInstancedTransformRenderList(const U32 textureBankIndex)
	{
		return SystemSpriteInstanceTransforms.contains(textureBankIndex);
	}

	void CRenderManager::AddSpriteToInstancedTransformRenderList(const U32 textureBankIndex, const SMatrix& transformMatrix)
	{
		if (!SystemSpriteInstanceTransforms.contains(textureBankIndex))
			SystemSpriteInstanceTransforms.emplace(textureBankIndex, std::vector<SMatrix>());

		SystemSpriteInstanceTransforms[textureBankIndex].emplace_back(transformMatrix);
	}

	void CRenderManager::SwapSpriteInstancedTransformRenderLists()
	{
		std::swap(SystemSpriteInstanceTransforms, RendererSpriteInstanceTransforms);
	}

	void CRenderManager::ClearSpriteInstanceTransforms()
	{
		SystemSpriteInstanceTransforms.clear();
	}

	bool CRenderManager::IsSpriteInInstancedUVRectRenderList(const U32 textureBankIndex)
	{
		return SystemSpriteInstanceUVRects.contains(textureBankIndex);
	}

	void CRenderManager::AddSpriteToInstancedUVRectRenderList(const U32 textureBankIndex, const SVector4& uvRect)
	{
		if (!SystemSpriteInstanceUVRects.contains(textureBankIndex))
			SystemSpriteInstanceUVRects.emplace(textureBankIndex, std::vector<SVector4>());

		SystemSpriteInstanceUVRects[textureBankIndex].emplace_back(uvRect);
	}

	void CRenderManager::SwapSpriteInstancedUVRectRenderLists()
	{
		std::swap(SystemSpriteInstanceUVRects, RendererSpriteInstanceUVRects);
	}

	void CRenderManager::ClearSpriteInstanceUVRects()
	{
		SystemSpriteInstanceUVRects.clear();
	}

	bool CRenderManager::IsSpriteInInstancedColorRenderList(const U32 textureBankIndex)
	{
		return SystemSpriteInstanceColors.contains(textureBankIndex);
	}

	void CRenderManager::AddSpriteToInstancedColorRenderList(const U32 textureBankIndex, const SVector4& color)
	{
		if (!SystemSpriteInstanceColors.contains(textureBankIndex))
			SystemSpriteInstanceColors.emplace(textureBankIndex, std::vector<SVector4>());

		SystemSpriteInstanceColors[textureBankIndex].emplace_back(color);
	}

	void CRenderManager::SwapSpriteInstancedColorRenderLists()
	{
		std::swap(SystemSpriteInstanceColors, RendererSpriteInstanceColors);
	}

	void CRenderManager::ClearSpriteInstanceColors()
	{
		SystemSpriteInstanceColors.clear();
	}

	const CFullscreenTexture& CRenderManager::GetRenderedSceneTexture() const
	{
		return RenderedScene;
	}

	void CRenderManager::PushRenderCommand(SRenderCommand command)
	{
		PushToCommands->push(command);
	}

	void CRenderManager::SwapRenderCommandBuffers()
	{
		std::swap(PushToCommands, PopFromCommands);
	}

	void CRenderManager::Clear(SVector4 /*clearColor*/)
	{
		//Backbuffer.ClearTexture(clearColor);
		//myIntermediateDepth.ClearDepth();
	}

	void CRenderManager::InitDataBuffers()
	{
		D3D11_BUFFER_DESC bufferDescription = { 0 };
		bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		bufferDescription.ByteWidth = sizeof(SFrameBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &FrameBuffer), "Frame Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SObjectBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &ObjectBuffer), "Object Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SMaterialBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &MaterialBuffer), "Material Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SDebugShapeObjectBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &DebugShapeObjectBuffer), "Debug Shape Object Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SDecalBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &DecalBuffer), "Decal Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SSpriteBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &SpriteBuffer), "Sprite Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SDirectionalLightBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &DirectionalLightBuffer), "Directional Light Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SPointLightBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &PointLightBuffer), "Point Light Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SSpotLightBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &SpotLightBuffer), "Spot Light Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SShadowmapBufferData) * 6;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &ShadowmapBuffer), "Shadowmap Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SVolumetricLightBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &VolumetricLightBuffer), "Volumetric Light Buffer could not be created.");

		bufferDescription.ByteWidth = sizeof(SEmissiveBufferData);
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &EmissiveBuffer), "Emissive Buffer could not be created.");

		//Instance Transform Buffer
		D3D11_BUFFER_DESC instanceBufferDesc;
		instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		instanceBufferDesc.ByteWidth = sizeof(SMatrix) * InstancedDrawInstanceLimit;
		instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		instanceBufferDesc.MiscFlags = 0;
		instanceBufferDesc.StructureByteStride = 0;

		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&instanceBufferDesc, nullptr, &InstancedTransformBuffer), "Instanced Transform Buffer could not be created.");

		//Instance UV Rect Buffer
		instanceBufferDesc.ByteWidth = sizeof(SVector4) * InstancedDrawInstanceLimit;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&instanceBufferDesc, nullptr, &InstancedUVRectBuffer), "Instanced UV Rect Buffer could not be created.");

		//Instance Color Buffer
		instanceBufferDesc.ByteWidth = sizeof(SVector4) * InstancedDrawInstanceLimit;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&instanceBufferDesc, nullptr, &InstancedColorBuffer), "Instanced Color Buffer could not be created.");
	}

	U16 CRenderManager::AddIndexBuffer(const std::vector<U32>& indices)
	{
		D3D11_BUFFER_DESC indexBufferDesc = { 0 };
		indexBufferDesc.ByteWidth = sizeof(U32) * static_cast<U32>(indices.size());
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubresourceData = { 0 };
		indexSubresourceData.pSysMem = indices.data();

		ID3D11Buffer* indexBuffer;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer), "Index Buffer could not be created.");
		IndexBuffers.emplace_back(indexBuffer);

		return static_cast<U16>(IndexBuffers.size() - 1);
	}

	U16 CRenderManager::AddMeshVertexOffset(U32 offset)
	{
		MeshVertexOffsets.emplace_back(offset);
		return static_cast<U16>(MeshVertexOffsets.size() - 1);
	}

	U16 CRenderManager::AddMeshVertexStride(U32 stride)
	{
		MeshVertexStrides.emplace_back(stride);
		return static_cast<U16>(MeshVertexStrides.size() - 1);
	}

	std::string CRenderManager::AddShader(const std::string& fileName, const EShaderType shaderType)
	{
		std::string outShaderData;

		switch (shaderType)
		{
		case EShaderType::Vertex:
		{
			ID3D11VertexShader* vertexShader;
			UGraphicsUtils::CreateVertexShader(fileName, Framework, &vertexShader, outShaderData);
			VertexShaders.emplace_back(vertexShader);
		}
		break;
		case EShaderType::Compute:
		case EShaderType::Geometry:
		{
			ID3D11GeometryShader* geometryShader;
			UGraphicsUtils::CreateGeometryShader(fileName, Framework, &geometryShader);
			GeometryShaders.emplace_back(geometryShader);
		}
		break;
		case EShaderType::Pixel:
		{
			ID3D11PixelShader* pixelShader;
			UGraphicsUtils::CreatePixelShader(fileName, Framework, &pixelShader);
			PixelShaders.emplace_back(pixelShader);
		}
		break;
		}

		return outShaderData;
	}

	void CRenderManager::AddInputLayout(const std::string& vsData, EInputLayoutType layoutType)
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
		switch (layoutType)
		{
		case EInputLayoutType::Pos3Nor3Tan3Bit3UV2:
			layout =
			{
				{"POSITION"	,	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL"   ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TANGENT"  ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"BINORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"UV"		,   0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};
			break;

		case EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans:
			layout =
			{
				{"POSITION"	,	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL"   ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TANGENT"  ,   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"BINORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"UV"		,   0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"INSTANCETRANSFORM",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"INSTANCETRANSFORM",	1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"INSTANCETRANSFORM",	2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"INSTANCETRANSFORM",	3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
			};
			break;

		case EInputLayoutType::Pos4:
			layout =
			{
				{"POSITION"	,	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};
			break;

		case EInputLayoutType::TransUVRectColor:
			layout =
			{
				{"INSTANCETRANSFORM",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"INSTANCETRANSFORM",	1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"INSTANCETRANSFORM",	2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"INSTANCETRANSFORM",	3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"INSTANCEUVRECT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"INSTANCECOLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
			};
			break;
		}
		ID3D11InputLayout* inputLayout;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateInputLayout(layout.data(), static_cast<U32>(layout.size()), vsData.data(), vsData.size(), &inputLayout), "Input Layout could not be created.")
			InputLayouts.emplace_back(inputLayout);
	}

	void CRenderManager::AddSampler(ESamplerType samplerType)
	{
		// TODO.NR: Extend to different LOD levels and filters
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = 10;

		switch (samplerType)
		{
		case ESamplerType::Border:
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			break;
		case ESamplerType::Clamp:
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case ESamplerType::Mirror:
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
			break;
		case ESamplerType::Wrap:
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		}

		ID3D11SamplerState* samplerState;
		ENGINE_HR_MESSAGE(Framework->GetDevice()->CreateSamplerState(&samplerDesc, &samplerState), "Sampler could not be created.");
		Samplers.emplace_back(samplerState);
	}

	void CRenderManager::AddTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		Topologies.emplace_back(topology);
	}

	void CRenderManager::AddViewport(SVector2<F32> topLeftCoordinate, SVector2<F32> widthAndHeight, SVector2<F32> depth)
	{
		Viewports.emplace_back(D3D11_VIEWPORT(topLeftCoordinate.X, topLeftCoordinate.Y, widthAndHeight.X, widthAndHeight.Y, depth.X, depth.Y));
	}

	std::vector<U16> CRenderManager::AddMaterial(const std::string& materialName, EMaterialConfiguration configuration)
	{
		std::vector<U16> references;
		
		switch (configuration)
		{
		case EMaterialConfiguration::AlbedoMaterialNormal_Packed:
		{
			const std::string texturesFolder = "Assets/Textures/";
			auto textureBank = GEngine::GetTextureBank();

			references.emplace_back(static_cast<U16>(textureBank->GetTextureIndex(texturesFolder + materialName + "_c.hva")));
			references.emplace_back(static_cast<U16>(textureBank->GetTextureIndex(texturesFolder + materialName + "_m.hva")));
			references.emplace_back(static_cast<U16>(textureBank->GetTextureIndex(texturesFolder + materialName + "_n.hva")));
		}
			break;
		}

		return references;
	}

	void CRenderManager::ShadowAtlasPrePassDirectional(const SRenderCommand& command)
	{
		const auto& transformComp = command.GetComponent(TransformComponent);
		const auto& staticMeshComp = command.GetComponent(StaticMeshComponent);
		const auto& directionalLightComp = command.GetComponent(DirectionalLightComponent);

		FrameBufferData.ToCameraFromWorld = directionalLightComp.ShadowmapView.ShadowViewMatrix;
		FrameBufferData.ToWorldFromCamera = directionalLightComp.ShadowmapView.ShadowViewMatrix.FastInverse();
		FrameBufferData.ToProjectionFromCamera = directionalLightComp.ShadowmapView.ShadowProjectionMatrix;
		FrameBufferData.ToCameraFromProjection = directionalLightComp.ShadowmapView.ShadowProjectionMatrix.Inverse();
		FrameBufferData.CameraPosition = directionalLightComp.ShadowmapView.ShadowPosition;
		BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");
		Context->RSSetViewports(1, &Viewports[directionalLightComp.ShadowmapView.ShadowmapViewportIndex]);

		// =============

		ObjectBufferData.ToWorldFromObject = transformComp.Transform.GetMatrix();
		BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

		const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceTransforms[staticMeshComp.Name.AsString()];
		BindBuffer(InstancedTransformBuffer, matrices, "Instanced Transform Buffer");

		Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
		Context->IASetPrimitiveTopology(Topologies[staticMeshComp.TopologyIndex]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)]);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::StaticMeshInstanced)], nullptr, 0);
		Context->PSSetShader(nullptr, nullptr, 0);

		for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp.DrawCallData.size()); drawCallIndex++)
		{
			const SDrawCallData& drawData = staticMeshComp.DrawCallData[drawCallIndex];
			ID3D11Buffer* bufferPointers[2] = { VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
			const U32 strides[2] = { MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
			const U32 offsets[2] = { MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
			Context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
			Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
			Context->DrawIndexedInstanced(drawData.IndexCount, static_cast<U32>(matrices.size()), 0, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::ShadowAtlasPrePassPoint(const SRenderCommand& command)
	{
		const auto& transformComp = command.GetComponent(TransformComponent);
		const auto& staticMeshComp = command.GetComponent(StaticMeshComponent);
		const auto& pointLightComp = command.GetComponent(PointLightComponent);

		// TODO.NR: Not needed for instanced rendering?
		ObjectBufferData.ToWorldFromObject = transformComp.Transform.GetMatrix();
		BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

		const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceTransforms[staticMeshComp.Name.AsString()];
		BindBuffer(InstancedTransformBuffer, matrices, "Instanced Transform Buffer");

		Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
		Context->IASetPrimitiveTopology(Topologies[staticMeshComp.TopologyIndex]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)]);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::StaticMeshInstanced)], nullptr, 0);
		Context->PSSetShader(nullptr, nullptr, 0);

		for (const auto& shadowmapView : pointLightComp.ShadowmapViews)
		{
			FrameBufferData.ToCameraFromWorld = shadowmapView.ShadowViewMatrix;
			FrameBufferData.ToWorldFromCamera = shadowmapView.ShadowViewMatrix.FastInverse();
			FrameBufferData.ToProjectionFromCamera = shadowmapView.ShadowProjectionMatrix;
			FrameBufferData.ToCameraFromProjection = shadowmapView.ShadowProjectionMatrix.Inverse();
			FrameBufferData.CameraPosition = shadowmapView.ShadowPosition;
			BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");

			Context->VSSetConstantBuffers(0, 1, &FrameBuffer);

			Context->RSSetViewports(1, &Viewports[shadowmapView.ShadowmapViewportIndex]);

			// =============

			for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp.DrawCallData.size()); drawCallIndex++)
			{
				const SDrawCallData& drawData = staticMeshComp.DrawCallData[drawCallIndex];
				ID3D11Buffer* bufferPointers[2] = { VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
				const U32 strides[2] = { MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
				const U32 offsets[2] = { MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
				Context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
				Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
				Context->DrawIndexedInstanced(drawData.IndexCount, static_cast<U32>(matrices.size()), 0, 0, 0);
				CRenderManager::NumberOfDrawCallsThisFrame++;
			}
		}
	}

	void CRenderManager::ShadowAtlasPrePassSpot(const SRenderCommand& command)
	{
		const auto& transformComp = command.GetComponent(TransformComponent);
		const auto& staticMeshComp = command.GetComponent(StaticMeshComponent);
		const auto& spotLightComp = command.GetComponent(SpotLightComponent);

		FrameBufferData.ToCameraFromWorld = spotLightComp.ShadowmapView.ShadowViewMatrix;
		FrameBufferData.ToWorldFromCamera = spotLightComp.ShadowmapView.ShadowViewMatrix.FastInverse();
		FrameBufferData.ToProjectionFromCamera = spotLightComp.ShadowmapView.ShadowProjectionMatrix;
		FrameBufferData.ToCameraFromProjection = spotLightComp.ShadowmapView.ShadowProjectionMatrix.Inverse();
		FrameBufferData.CameraPosition = spotLightComp.ShadowmapView.ShadowPosition;
		BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");

		ObjectBufferData.ToWorldFromObject = transformComp.Transform.GetMatrix();
		BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

		Context->VSSetConstantBuffers(0, 1, &FrameBuffer);
		Context->RSSetViewports(1, &Viewports[spotLightComp.ShadowmapView.ShadowmapViewportIndex]);

		// =============

		const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceTransforms[staticMeshComp.Name.AsString()];
		BindBuffer(InstancedTransformBuffer, matrices, "Instanced Transform Buffer");

		Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
		Context->IASetPrimitiveTopology(Topologies[staticMeshComp.TopologyIndex]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)]);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::StaticMeshInstanced)], nullptr, 0);
		Context->PSSetShader(nullptr, nullptr, 0);

		for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp.DrawCallData.size()); drawCallIndex++)
		{
			const SDrawCallData& drawData = staticMeshComp.DrawCallData[drawCallIndex];
			ID3D11Buffer* bufferPointers[2] = { VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
			const U32 strides[2] = { MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
			const U32 offsets[2] = { MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
			Context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
			Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
			Context->DrawIndexedInstanced(drawData.IndexCount, static_cast<U32>(matrices.size()), 0, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::CameraDataStorage(const SRenderCommand& command)
	{
		GBuffer.SetAsActiveTarget(&IntermediateDepth);

		const auto& transformComp = command.GetComponent(TransformComponent);
		const auto& cameraComp = command.GetComponent(CameraComponent);

		FrameBufferData.ToCameraFromWorld = transformComp.Transform.GetMatrix().FastInverse();
		FrameBufferData.ToWorldFromCamera = transformComp.Transform.GetMatrix();
		FrameBufferData.ToProjectionFromCamera = cameraComp.ProjectionMatrix;
		FrameBufferData.ToCameraFromProjection = cameraComp.ProjectionMatrix.Inverse();
		FrameBufferData.CameraPosition = transformComp.Transform.GetMatrix().GetTranslation4();
		BindBuffer(FrameBuffer, FrameBufferData, "Frame Buffer");

		Context->VSSetConstantBuffers(0, 1, &FrameBuffer);
		Context->PSSetConstantBuffers(0, 1, &FrameBuffer);
		Context->GSSetConstantBuffers(0, 1, &FrameBuffer);
	}

	void CRenderManager::GBufferDataInstanced(const SRenderCommand& command)
	{
		const auto& transformComp = command.GetComponent(TransformComponent);
		const auto& staticMeshComp = command.GetComponent(StaticMeshComponent);
		const auto& materialComp = command.GetComponent(MaterialComponent);

		ObjectBufferData.ToWorldFromObject = transformComp.Transform.GetMatrix();
		BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

		const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceTransforms[staticMeshComp.Name.AsString()];
		BindBuffer(InstancedTransformBuffer, matrices, "Instanced Transform Buffer");

		Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
		Context->IASetPrimitiveTopology(Topologies[staticMeshComp.TopologyIndex]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans)]);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::StaticMeshInstanced)], nullptr, 0);
		Context->PSSetShader(PixelShaders[staticMeshComp.PixelShaderIndex], nullptr, 0);

		ID3D11SamplerState* sampler = Samplers[staticMeshComp.SamplerIndex];
		Context->PSSetSamplers(0, 1, &sampler);

		auto textureBank = GEngine::GetTextureBank();
		for (U8 drawCallIndex = 0; drawCallIndex < static_cast<U8>(staticMeshComp.DrawCallData.size()); drawCallIndex++)
		{
			std::vector<ID3D11ShaderResourceView*> resourceViewPointers;
			
			std::map<F32, F32> textureIndices;
			auto findTextureByIndex = [&](SRuntimeGraphicsMaterialProperty& bufferProperty)
			{
				if (bufferProperty.TextureChannelIndex > -1.0f)
				{
					if (!textureIndices.contains(bufferProperty.TextureIndex))
					{
						resourceViewPointers.emplace_back(textureBank->GetTexture(static_cast<U32>(bufferProperty.TextureIndex)));
						textureIndices.emplace(bufferProperty.TextureIndex, static_cast<F32>(resourceViewPointers.size() - 1));
					}
					
					bufferProperty.TextureIndex = textureIndices[bufferProperty.TextureIndex];
				}
			};

			MaterialBufferData = SMaterialBufferData(materialComp.Materials[drawCallIndex]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::AlbedoR)]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::AlbedoG)]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::AlbedoB)]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::AlbedoA)]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::NormalX)]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::NormalY)]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::NormalZ)]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::AmbientOcclusion)]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::Metalness)]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::Roughness)]);
			findTextureByIndex(MaterialBufferData.Properties[static_cast<U8>(EMaterialProperty::Emissive)]);

			BindBuffer(MaterialBuffer, MaterialBufferData, "Material Buffer");

			Context->PSSetShaderResources(5, static_cast<U32>(resourceViewPointers.size()), resourceViewPointers.data());
			Context->PSSetConstantBuffers(8, 1, &MaterialBuffer);

			const SDrawCallData& drawData = staticMeshComp.DrawCallData[drawCallIndex];
			ID3D11Buffer* bufferPointers[2] = { VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
			const U32 strides[2] = { MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
			const U32 offsets[2] = { MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
			Context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
			Context->IASetIndexBuffer(IndexBuffers[drawData.IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);
			Context->DrawIndexedInstanced(drawData.IndexCount, static_cast<U32>(matrices.size()), 0, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::GBufferSpriteInstanced(const SRenderCommand& command)
	{
		// TODO.NR: Fix transparency
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::GBufferAlphaBlend);

		//const STransformComponent& transformComp = command.GetComponent(TransformComponent);
		const SSpriteComponent& spriteComp = command.GetComponent(SpriteComponent); 

		//ObjectBufferData.ToWorldFromObject = transformComp.Transform.GetMatrix();
		//BindBuffer(ObjectBuffer, ObjectBufferData, "Object Buffer");

		const std::vector<SMatrix>& matrices = RendererSpriteInstanceTransforms[spriteComp.TextureIndex];
		BindBuffer(InstancedTransformBuffer, matrices, "Instanced Transform Buffer");

		const std::vector<SVector4>& uvRects = RendererSpriteInstanceUVRects[spriteComp.TextureIndex];
		BindBuffer(InstancedUVRectBuffer, uvRects, "Instanced UV Rect Buffer");

		const std::vector<SVector4>& colors = RendererSpriteInstanceColors[spriteComp.TextureIndex];
		BindBuffer(InstancedColorBuffer, colors, "Instanced Color Buffer");

		//Context->VSSetConstantBuffers(1, 1, &ObjectBuffer);
		Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::PointList)]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::TransUVRectColor)]);

		// Make new VS, GS, PS, need to write to GBuffer
		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::SpriteWorldSpace)], nullptr, 0);
		Context->GSSetShader(GeometryShaders[static_cast<U8>(EGeometryShaders::SpriteWorldSpace)], nullptr, 0);
		//Context->PSSetConstantBuffers(0, 1, &SpriteBuffer);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::SpriteWorldSpace)], nullptr, 0);

		ID3D11SamplerState* sampler = Samplers[static_cast<U8>(ESamplers::DefaultWrap)];
		Context->PSSetSamplers(0, 1, &sampler);

		ID3D11ShaderResourceView* spriteTexture = GEngine::GetTextureBank()->GetTexture(spriteComp.TextureIndex);
		Context->PSSetShaderResources(0, 1, &spriteTexture);

		ID3D11Buffer* bufferPointers[3] = { InstancedTransformBuffer, InstancedUVRectBuffer, InstancedColorBuffer };
		const U32 strides[3] = { sizeof(SMatrix), sizeof(SVector4), sizeof(SVector4) };
		const U32 offsets[3] = { 0, 0, 0 };
		Context->IASetVertexBuffers(0, 3, bufferPointers, strides, offsets);
		Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		Context->DrawInstanced(1, static_cast<U32>(matrices.size()), 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CRenderManager::DecalDepthCopy(const SRenderCommand& /*command*/)
	{
		DepthCopy.SetAsActiveTarget();
		IntermediateDepth.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::CopyDepth);
	}

	void CRenderManager::DeferredDecal(const SRenderCommand& command)
	{
		RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::OnlyRead);
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);
		GBuffer.SetAsActiveTarget(&IntermediateDepth);
		DepthCopy.SetAsResourceOnSlot(21);

		const auto& transformComp = command.GetComponent(TransformComponent);
		const auto& decalComp = command.GetComponent(DecalComponent);

		DecalBufferData.ToWorld = transformComp.Transform.GetMatrix();
		DecalBufferData.ToObjectSpace = transformComp.Transform.GetMatrix().Inverse();

		BindBuffer(DecalBuffer, DecalBufferData, "Decal Buffer");

		Context->VSSetConstantBuffers(1, 1, &DecalBuffer);
		Context->PSSetConstantBuffers(1, 1, &DecalBuffer);

		Context->IASetPrimitiveTopology(Topologies[0]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos3Nor3Tan3Bit3UV2)]);
		Context->IASetVertexBuffers(0, 1, &VertexBuffers[0], &MeshVertexStrides[0], &MeshVertexOffsets[0]);
		Context->IASetIndexBuffer(IndexBuffers[static_cast<U8>(EDefaultIndexBuffers::DecalProjector)], DXGI_FORMAT_R32_UINT, 0);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::Decal)], nullptr, 0);

		auto sampler = Samplers[static_cast<U8>(ESamplers::DefaultWrap)];
		Context->PSSetSamplers(0, 1, &sampler);

		auto textureBank = GEngine::GetTextureBank();
		if (decalComp.ShouldRenderAlbedo)
		{
			auto shaderResource = textureBank->GetTexture(decalComp.TextureReferences[0]);
			Context->PSSetShaderResources(5, 1, &shaderResource);
			Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DecalAlbedo)], nullptr, 0);
			Context->DrawIndexed(36, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		if (decalComp.ShouldRenderMaterial)
		{
			auto shaderResource = textureBank->GetTexture(decalComp.TextureReferences[1]);
			Context->PSSetShaderResources(6, 1, &shaderResource);
			Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DecalMaterial)], nullptr, 0);
			Context->DrawIndexed(36, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		if (decalComp.ShouldRenderNormal)
		{
			auto shaderResource = textureBank->GetTexture(decalComp.TextureReferences[2]);
			Context->PSSetShaderResources(7, 1, &shaderResource);
			Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DecalNormal)], nullptr, 0);
			Context->DrawIndexed(36, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::PreLightingPass(const SRenderCommand& /*command*/)
	{
		// === SSAO ===
		SSAOBuffer.SetAsActiveTarget();
		GBuffer.SetAsResourceOnSlot(CGBuffer::EGBufferTextures::Normal, 2);
		IntermediateDepth.SetAsResourceOnSlot(21);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::SSAO);
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::Disable);

		SSAOBlurTexture.SetAsActiveTarget();
		SSAOBuffer.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::SSAOBlur);
		// === !SSAO ===

		Context->PSSetSamplers(0, 1, &Samplers[static_cast<U8>(ESamplers::DefaultWrap)]);
		Context->PSSetSamplers(1, 1, &Samplers[static_cast<U8>(ESamplers::DefaultBorder)]);

		Context->GSSetShader(nullptr, nullptr, 0);

		LitScene.SetAsActiveTarget();
		GBuffer.SetAllAsResources(1);
		IntermediateDepth.SetAsResourceOnSlot(21);
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
	}

	void CRenderManager::DeferredLightingDirectional(const SRenderCommand& command)
	{
		// add Alpha blend PS shader

		ShadowAtlasDepth.SetAsResourceOnSlot(22);
		SSAOBlurTexture.SetAsResourceOnSlot(23);

		const auto& environmentLightComp = command.GetComponent(EnvironmentLightComponent);
		const auto& directionalLightComp = command.GetComponent(DirectionalLightComponent);

		auto cubemapTexture = GEngine::GetTextureBank()->GetTexture(environmentLightComp.AmbientCubemapReference);
		Context->PSSetShaderResources(0, 1, &cubemapTexture);

		// Update lightbufferdata and fill lightbuffer
		DirectionalLightBufferData.DirectionalLightDirection = directionalLightComp.Direction;
		DirectionalLightBufferData.DirectionalLightColor = directionalLightComp.Color;
		BindBuffer(DirectionalLightBuffer, DirectionalLightBufferData, "Light Buffer");
		Context->PSSetConstantBuffers(2, 1, &DirectionalLightBuffer);

		ShadowmapBufferData.ToShadowmapView = directionalLightComp.ShadowmapView.ShadowViewMatrix;
		ShadowmapBufferData.ToShadowmapProjection = directionalLightComp.ShadowmapView.ShadowProjectionMatrix;
		ShadowmapBufferData.ShadowmapPosition = directionalLightComp.ShadowmapView.ShadowPosition;

		const auto& viewport = Viewports[directionalLightComp.ShadowmapView.ShadowmapViewportIndex];
		ShadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
		ShadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
		ShadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
		ShadowmapBufferData.ShadowTestTolerance = 0.001f;

		BindBuffer(ShadowmapBuffer, ShadowmapBufferData, "Shadowmap Buffer");
		Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

		// Emissive Post Processing 
		EmissiveBufferData.EmissiveStrength = FullscreenRenderer.PostProcessingBufferData.EmissiveStrength;
		BindBuffer(EmissiveBuffer, EmissiveBufferData, "Emissive Buffer");
		Context->PSSetConstantBuffers(7, 1, &EmissiveBuffer);

		Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Context->IASetInputLayout(nullptr);
		Context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::Fullscreen)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DeferredDirectional)], nullptr, 0);

		Context->Draw(3, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CRenderManager::DeferredLightingPoint(const SRenderCommand& command)
	{
		ShadowAtlasDepth.SetAsResourceOnSlot(22);
		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);

		// Update lightbufferdata and fill lightbuffer
		const auto& pointLightComp = command.GetComponent(PointLightComponent);
		const auto& transformComponent = command.GetComponent(TransformComponent);
		SVector position = transformComponent.Transform.GetMatrix().GetTranslation();
		PointLightBufferData.ToWorldFromObject = transformComponent.Transform.GetMatrix();
		PointLightBufferData.ColorAndIntensity = pointLightComp.ColorAndIntensity;
		PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, pointLightComp.Range };
		BindBuffer(PointLightBuffer, PointLightBufferData, "Point Light Buffer");
		Context->VSSetConstantBuffers(3, 1, &PointLightBuffer);
		Context->PSSetConstantBuffers(3, 1, &PointLightBuffer);

		SShadowmapBufferData shadowmapBufferData[6];
		for (U8 shadowmapViewIndex = 0; shadowmapViewIndex < 6; shadowmapViewIndex++)
		{
			shadowmapBufferData[shadowmapViewIndex].ToShadowmapView = pointLightComp.ShadowmapViews[shadowmapViewIndex].ShadowViewMatrix;
			shadowmapBufferData[shadowmapViewIndex].ToShadowmapProjection = pointLightComp.ShadowmapViews[shadowmapViewIndex].ShadowProjectionMatrix;
			shadowmapBufferData[shadowmapViewIndex].ShadowmapPosition = pointLightComp.ShadowmapViews[shadowmapViewIndex].ShadowPosition;

			const auto& viewport = Viewports[pointLightComp.ShadowmapViews[shadowmapViewIndex].ShadowmapViewportIndex];
			shadowmapBufferData[shadowmapViewIndex].ShadowmapResolution = { viewport.Width, viewport.Height };
			shadowmapBufferData[shadowmapViewIndex].ShadowAtlasResolution = ShadowAtlasResolution;
			shadowmapBufferData[shadowmapViewIndex].ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
			shadowmapBufferData[shadowmapViewIndex].ShadowTestTolerance = 0.00001f;
		}

		BindBuffer(ShadowmapBuffer, shadowmapBufferData, "Shadowmap Buffer");
		Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

		Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos4)]);
		Context->IASetVertexBuffers(0, 1, &VertexBuffers[1], &MeshVertexStrides[1], &MeshVertexOffsets[0]);
		Context->IASetIndexBuffer(IndexBuffers[static_cast<U8>(EDefaultIndexBuffers::PointLightCube)], DXGI_FORMAT_R32_UINT, 0);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::PointAndSpotLight)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DeferredPoint)], nullptr, 0);

		Context->DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
	}

	void CRenderManager::DeferredLightingSpot(const SRenderCommand& command)
	{
		ShadowAtlasDepth.SetAsResourceOnSlot(22);
		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);

		// Update lightbufferdata and fill lightbuffer
		const auto& spotLightComp = command.GetComponent(SpotLightComponent);
		const auto& transformComponent = command.GetComponent(TransformComponent);
		SVector position = transformComponent.Transform.GetMatrix().GetTranslation();

		PointLightBufferData.ToWorldFromObject = transformComponent.Transform.GetMatrix();
		PointLightBufferData.ColorAndIntensity = spotLightComp.ColorAndIntensity;
		PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, spotLightComp.Range };

		BindBuffer(PointLightBuffer, PointLightBufferData, "Spotlight Vertex Shader Buffer");
		Context->VSSetConstantBuffers(3, 1, &PointLightBuffer);

		SpotLightBufferData.ColorAndIntensity = spotLightComp.ColorAndIntensity;
		SpotLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, spotLightComp.Range };
		SpotLightBufferData.Direction = spotLightComp.Direction;
		SpotLightBufferData.DirectionNormal1 = spotLightComp.DirectionNormal1;
		SpotLightBufferData.DirectionNormal2 = spotLightComp.DirectionNormal2;
		SpotLightBufferData.OuterAngle = spotLightComp.OuterAngle;
		SpotLightBufferData.InnerAngle = spotLightComp.InnerAngle;

		BindBuffer(SpotLightBuffer, SpotLightBufferData, "Spotlight Pixel Shader Buffer");
		Context->PSSetConstantBuffers(3, 1, &SpotLightBuffer);

		SShadowmapBufferData shadowmapBufferData;
		shadowmapBufferData.ToShadowmapView = spotLightComp.ShadowmapView.ShadowViewMatrix;
		shadowmapBufferData.ToShadowmapProjection = spotLightComp.ShadowmapView.ShadowProjectionMatrix;
		shadowmapBufferData.ShadowmapPosition = spotLightComp.ShadowmapView.ShadowPosition;

		const auto& viewport = Viewports[spotLightComp.ShadowmapView.ShadowmapViewportIndex];
		shadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
		shadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
		shadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
		shadowmapBufferData.ShadowTestTolerance = 0.00001f;

		BindBuffer(ShadowmapBuffer, shadowmapBufferData, "Shadowmap Buffer");
		Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

		Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos4)]);
		Context->IASetVertexBuffers(0, 1, &VertexBuffers[1], &MeshVertexStrides[1], &MeshVertexOffsets[0]);
		Context->IASetIndexBuffer(IndexBuffers[static_cast<U8>(EDefaultIndexBuffers::PointLightCube)], DXGI_FORMAT_R32_UINT, 0);

		// Use Point Light Vertex Shader
		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::PointAndSpotLight)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::DeferredSpot)], nullptr, 0);

		Context->DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
	}

	void CRenderManager::PostBaseLightingPass(const SRenderCommand& /*command*/)
	{
		RenderedScene.SetAsActiveTarget();
		LitScene.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
	}

	void CRenderManager::VolumetricLightingDirectional(const SRenderCommand& command)
	{
		VolumetricAccumulationBuffer.SetAsActiveTarget();
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
		IntermediateDepth.SetAsResourceOnSlot(21);
		ShadowAtlasDepth.SetAsResourceOnSlot(22);

		const auto& directionalLightComp = command.GetComponent(DirectionalLightComponent);
		const auto& volumetricLightComp = command.GetComponent(VolumetricLightComponent);

		// Lightbuffer
		DirectionalLightBufferData.DirectionalLightDirection = directionalLightComp.Direction;
		DirectionalLightBufferData.DirectionalLightColor = directionalLightComp.Color;
		BindBuffer(DirectionalLightBuffer, DirectionalLightBufferData, "Light Buffer");
		Context->PSSetConstantBuffers(1, 1, &DirectionalLightBuffer);

		// Volumetric buffer
		VolumetricLightBufferData.NumberOfSamplesReciprocal = (1.0f / volumetricLightComp.NumberOfSamples);
		VolumetricLightBufferData.LightPower = volumetricLightComp.LightPower;
		VolumetricLightBufferData.ScatteringProbability = volumetricLightComp.ScatteringProbability;
		VolumetricLightBufferData.HenyeyGreensteinGValue = volumetricLightComp.HenyeyGreensteinGValue;

		BindBuffer(VolumetricLightBuffer, VolumetricLightBufferData, "Volumetric Light Buffer");
		Context->PSSetConstantBuffers(4, 1, &VolumetricLightBuffer);

		// Shadowbuffer
		ShadowmapBufferData.ToShadowmapView = directionalLightComp.ShadowmapView.ShadowViewMatrix;
		ShadowmapBufferData.ToShadowmapProjection = directionalLightComp.ShadowmapView.ShadowProjectionMatrix;
		ShadowmapBufferData.ShadowmapPosition = directionalLightComp.ShadowmapView.ShadowPosition;

		const auto& viewport = Viewports[directionalLightComp.ShadowmapView.ShadowmapViewportIndex];
		ShadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
		ShadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
		ShadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
		ShadowmapBufferData.ShadowTestTolerance = 0.001f;

		BindBuffer(ShadowmapBuffer, ShadowmapBufferData, "Shadowmap Buffer");
		Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

		Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
		Context->IASetInputLayout(nullptr);
		Context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

		Context->VSSetShader(VertexShaders[static_cast<U16>(EVertexShaders::Fullscreen)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U16>(EPixelShaders::VolumetricDirectional)], nullptr, 0);

		Context->Draw(3, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;

		ShouldBlurVolumetricBuffer = true;
	}

	void CRenderManager::VolumetricLightingPoint(const SRenderCommand& command)
	{
		VolumetricAccumulationBuffer.SetAsActiveTarget();
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);
		IntermediateDepth.SetAsResourceOnSlot(21);
		ShadowAtlasDepth.SetAsResourceOnSlot(22);

		const auto& transformComp = command.GetComponent(TransformComponent);
		const auto& pointLightComp = command.GetComponent(PointLightComponent);
		const auto& volumetricLightComp = command.GetComponent(VolumetricLightComponent);

		// Light Buffer
		SVector position = transformComp.Transform.GetMatrix().GetTranslation();
		PointLightBufferData.ToWorldFromObject = transformComp.Transform.GetMatrix();
		PointLightBufferData.ColorAndIntensity = pointLightComp.ColorAndIntensity;
		PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, pointLightComp.Range };
		BindBuffer(PointLightBuffer, PointLightBufferData, "Point Light Buffer");
		Context->VSSetConstantBuffers(3, 1, &PointLightBuffer);
		Context->PSSetConstantBuffers(3, 1, &PointLightBuffer);

		// Volumetric buffer
		VolumetricLightBufferData.NumberOfSamplesReciprocal = (1.0f / volumetricLightComp.NumberOfSamples);
		VolumetricLightBufferData.LightPower = volumetricLightComp.LightPower;
		VolumetricLightBufferData.ScatteringProbability = volumetricLightComp.ScatteringProbability;
		VolumetricLightBufferData.HenyeyGreensteinGValue = volumetricLightComp.HenyeyGreensteinGValue;

		BindBuffer(VolumetricLightBuffer, VolumetricLightBufferData, "Volumetric Light Buffer");
		Context->PSSetConstantBuffers(4, 1, &VolumetricLightBuffer);

		// Shadow Buffer
		SShadowmapBufferData shadowmapBufferData[6];
		for (U8 shadowmapViewIndex = 0; shadowmapViewIndex < 6; shadowmapViewIndex++)
		{
			shadowmapBufferData[shadowmapViewIndex].ToShadowmapView = pointLightComp.ShadowmapViews[shadowmapViewIndex].ShadowViewMatrix;
			shadowmapBufferData[shadowmapViewIndex].ToShadowmapProjection = pointLightComp.ShadowmapViews[shadowmapViewIndex].ShadowProjectionMatrix;
			shadowmapBufferData[shadowmapViewIndex].ShadowmapPosition = pointLightComp.ShadowmapViews[shadowmapViewIndex].ShadowPosition;

			const auto& viewport = Viewports[pointLightComp.ShadowmapViews[shadowmapViewIndex].ShadowmapViewportIndex];
			shadowmapBufferData[shadowmapViewIndex].ShadowmapResolution = { viewport.Width, viewport.Height };
			shadowmapBufferData[shadowmapViewIndex].ShadowAtlasResolution = ShadowAtlasResolution;
			shadowmapBufferData[shadowmapViewIndex].ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
			shadowmapBufferData[shadowmapViewIndex].ShadowTestTolerance = 0.00001f;
		}

		BindBuffer(ShadowmapBuffer, shadowmapBufferData, "Shadowmap Buffer");
		Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

		Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos4)]);
		Context->IASetVertexBuffers(0, 1, &VertexBuffers[1], &MeshVertexStrides[1], &MeshVertexOffsets[0]);
		Context->IASetIndexBuffer(IndexBuffers[static_cast<U8>(EDefaultIndexBuffers::PointLightCube)], DXGI_FORMAT_R32_UINT, 0);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::PointAndSpotLight)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::VolumetricPoint)], nullptr, 0);

		Context->DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);

		ShouldBlurVolumetricBuffer = true;
	}

	void CRenderManager::VolumetricLightingSpot(const SRenderCommand& command)
	{
		VolumetricAccumulationBuffer.SetAsActiveTarget();
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);
		IntermediateDepth.SetAsResourceOnSlot(21);
		ShadowAtlasDepth.SetAsResourceOnSlot(22);

		const auto& transformComp = command.GetComponent(TransformComponent);
		const auto& spotLightComp = command.GetComponent(SpotLightComponent);
		const auto& volumetricLightComp = command.GetComponent(VolumetricLightComponent);

		// Light Buffer
		SVector position = transformComp.Transform.GetMatrix().GetTranslation();
		PointLightBufferData.ToWorldFromObject = transformComp.Transform.GetMatrix();
		PointLightBufferData.ColorAndIntensity = spotLightComp.ColorAndIntensity;
		PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, spotLightComp.Range };

		BindBuffer(PointLightBuffer, PointLightBufferData, "Spotlight Vertex Shader Buffer");
		Context->VSSetConstantBuffers(3, 1, &PointLightBuffer);

		SpotLightBufferData.ColorAndIntensity = spotLightComp.ColorAndIntensity;
		SpotLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, spotLightComp.Range };
		SpotLightBufferData.Direction = spotLightComp.Direction;
		SpotLightBufferData.DirectionNormal1 = spotLightComp.DirectionNormal1;
		SpotLightBufferData.DirectionNormal2 = spotLightComp.DirectionNormal2;
		SpotLightBufferData.OuterAngle = spotLightComp.OuterAngle;
		SpotLightBufferData.InnerAngle = spotLightComp.InnerAngle;

		BindBuffer(SpotLightBuffer, SpotLightBufferData, "Spotlight Pixel Shader Buffer");
		Context->PSSetConstantBuffers(3, 1, &SpotLightBuffer);

		// Volumetric buffer
		VolumetricLightBufferData.NumberOfSamplesReciprocal = (1.0f / volumetricLightComp.NumberOfSamples);
		VolumetricLightBufferData.LightPower = volumetricLightComp.LightPower;
		VolumetricLightBufferData.ScatteringProbability = volumetricLightComp.ScatteringProbability;
		VolumetricLightBufferData.HenyeyGreensteinGValue = volumetricLightComp.HenyeyGreensteinGValue;

		BindBuffer(VolumetricLightBuffer, VolumetricLightBufferData, "Volumetric Light Buffer");
		Context->PSSetConstantBuffers(4, 1, &VolumetricLightBuffer);

		// Shadow Buffer
		SShadowmapBufferData shadowmapBufferData;
		shadowmapBufferData.ToShadowmapView = spotLightComp.ShadowmapView.ShadowViewMatrix;
		shadowmapBufferData.ToShadowmapProjection = spotLightComp.ShadowmapView.ShadowProjectionMatrix;
		shadowmapBufferData.ShadowmapPosition = spotLightComp.ShadowmapView.ShadowPosition;

		const auto& viewport = Viewports[spotLightComp.ShadowmapView.ShadowmapViewportIndex];
		shadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
		shadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
		shadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
		shadowmapBufferData.ShadowTestTolerance = 0.00001f;

		BindBuffer(ShadowmapBuffer, shadowmapBufferData, "Shadowmap Buffer");
		Context->PSSetConstantBuffers(5, 1, &ShadowmapBuffer);

		Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::TriangleList)]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos4)]);
		Context->IASetVertexBuffers(0, 1, &VertexBuffers[1], &MeshVertexStrides[1], &MeshVertexOffsets[0]);
		Context->IASetIndexBuffer(IndexBuffers[static_cast<U8>(EDefaultIndexBuffers::PointLightCube)], DXGI_FORMAT_R32_UINT, 0);

		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::PointAndSpotLight)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::VolumetricSpot)], nullptr, 0);

		Context->DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
		RenderStateManager.SetRasterizerState(CRenderStateManager::ERasterizerStates::Default);

		ShouldBlurVolumetricBuffer = true;
	}

	void CRenderManager::VolumetricBlur(const SRenderCommand& /*command*/)
	{
		if (!ShouldBlurVolumetricBuffer)
			return;

		// Downsampling and Blur
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::Disable);
		DownsampledDepth.SetAsActiveTarget();
		IntermediateDepth.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::DownsampleDepth);

		// Blur
		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		// Upsampling
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		RenderedScene.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
		DownsampledDepth.SetAsResourceOnSlot(1);
		IntermediateDepth.SetAsResourceOnSlot(2);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::DepthAwareUpsampling);
	}

	inline void CRenderManager::ForwardTransparency(const SRenderCommand& /*command*/)
	{
		//RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);
	}

	void CRenderManager::ScreenSpaceSprite(const SRenderCommand& command)
	{
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);

		const STransform2DComponent& transform2DComp = command.GetComponent(Transform2DComponent);
		const SSpriteComponent& spriteComponent = command.GetComponent(SpriteComponent);

		SpriteBufferData.Color = spriteComponent.Color.AsVector4();
		SpriteBufferData.UVRect = spriteComponent.UVRect;
		SpriteBufferData.Position = transform2DComp.Position;
		SpriteBufferData.Size = transform2DComp.Scale;
		SpriteBufferData.Rotation = UMath::DegToRad(transform2DComp.DegreesRoll);

		BindBuffer(SpriteBuffer, SpriteBufferData, "Sprite Buffer");

		Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::PointList)]);
		Context->IASetInputLayout(nullptr);
		Context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

		Context->VSSetConstantBuffers(0, 1, &SpriteBuffer);
		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::SpriteScreenSpace)], nullptr, 0);

		Context->GSSetShader(GeometryShaders[static_cast<U8>(EGeometryShaders::SpriteScreenSpace)], nullptr, 0);

		Context->PSSetConstantBuffers(0, 1, &SpriteBuffer);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::SpriteScreenSpace)], nullptr, 0);

		ID3D11ShaderResourceView* spriteTexture = GEngine::GetTextureBank()->GetTexture(spriteComponent.TextureIndex);
		Context->PSSetShaderResources(0, 1, &spriteTexture);

		Context->Draw(1, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;

		ID3D11Buffer* nullBuffer = NULL;
		Context->VSSetConstantBuffers(0, 1, &nullBuffer);
	}

	void CRenderManager::RenderBloom(const SRenderCommand& /*command*/)
	{
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::Disable);
		RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);

		HalfSizeTexture.SetAsActiveTarget();
		RenderedScene.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		QuarterSizeTexture.SetAsActiveTarget();
		HalfSizeTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		BlurTexture1.SetAsActiveTarget();
		QuarterSizeTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		BlurTexture2.SetAsActiveTarget();
		BlurTexture1.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianHorizontal);

		BlurTexture1.SetAsActiveTarget();
		BlurTexture2.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianVertical);

		BlurTexture2.SetAsActiveTarget();
		BlurTexture1.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianHorizontal);

		BlurTexture1.SetAsActiveTarget();
		BlurTexture2.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianVertical);

		QuarterSizeTexture.SetAsActiveTarget();
		BlurTexture1.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		HalfSizeTexture.SetAsActiveTarget();
		QuarterSizeTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		VignetteTexture.SetAsActiveTarget();
		RenderedScene.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		RenderedScene.SetAsActiveTarget();
		VignetteTexture.SetAsResourceOnSlot(0);
		HalfSizeTexture.SetAsResourceOnSlot(1);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Bloom);
	}

	inline void CRenderManager::Tonemapping(const SRenderCommand& /*command*/)
	{
		TonemappedTexture.SetAsActiveTarget();
		RenderedScene.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Tonemap);
	}

	inline void CRenderManager::AntiAliasing(const SRenderCommand& /*command*/)
	{
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::Disable);
		RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);
		
		AntiAliasedTexture.SetAsActiveTarget();
		TonemappedTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::FXAA);
	}

	inline void CRenderManager::GammaCorrection(const SRenderCommand& /*command*/)
	{
		RenderedScene.SetAsActiveTarget();
		AntiAliasedTexture.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GammaCorrection);
	}

	inline void CRenderManager::RendererDebug(const SRenderCommand& /*command*/)
	{
		//DebugShadowAtlas();
	}

	inline void CRenderManager::PreDebugShapes(const SRenderCommand& /*command*/)
	{
		RenderStateManager.SetDepthStencilState(CRenderStateManager::EDepthStencilStates::OnlyRead);
		RenderStateManager.SetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);

		Context->IASetPrimitiveTopology(Topologies[static_cast<U8>(ETopologies::LineList)]);
		Context->IASetInputLayout(InputLayouts[static_cast<U8>(EInputLayoutType::Pos4)]);

		Context->GSSetShader(GeometryShaders[static_cast<U8>(EGeometryShaders::Line)], nullptr, 0);
		Context->VSSetShader(VertexShaders[static_cast<U8>(EVertexShaders::Line)], nullptr, 0);
		Context->PSSetShader(PixelShaders[static_cast<U8>(EPixelShaders::Line)], nullptr, 0);
	}

	inline void CRenderManager::PostTonemappingUseDepth(const SRenderCommand& /*command*/)
	{
		TonemappedTexture.SetAsActiveTarget(&IntermediateDepth);
	}

	inline void CRenderManager::PostTonemappingIgnoreDepth(const SRenderCommand& /*command*/)
	{
		TonemappedTexture.SetAsActiveTarget();
	}

	inline void CRenderManager::DebugShapes(const SRenderCommand& /*command*/)
	{
		//const SDebugShapeComponent& shape = command.GetComponent(DebugShapeComponent);
		//const STransformComponent& transform = command.GetComponent(TransformComponent);

		//DebugShapeObjectBufferData.ToWorldFromObject = transform->Transform.GetMatrix();
		//DebugShapeObjectBufferData.Color = shape->Color.AsVector4();
		//DebugShapeObjectBufferData.HalfThickness = shape->Thickness;

		//BindBuffer(DebugShapeObjectBuffer, DebugShapeObjectBufferData, "Object Buffer");

		//Context->IASetVertexBuffers(0, 1, &VertexBuffers[shape->VertexBufferIndex], &MeshVertexStrides[1], &MeshVertexOffsets[0]);
		//Context->IASetIndexBuffer(IndexBuffers[shape->IndexBufferIndex], DXGI_FORMAT_R32_UINT, 0);

		//Context->GSSetConstantBuffers(1, 1, &DebugShapeObjectBuffer);

		//Context->VSSetConstantBuffers(1, 1, &DebugShapeObjectBuffer);
		//Context->DrawIndexed(shape->IndexCount, 0, 0);
		//NumberOfDrawCallsThisFrame++;
	}

	void CRenderManager::DebugShadowAtlas()
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = 256.0f;
		viewport.Height = 256.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		Context->RSSetViewports(1, &viewport);
		ShadowAtlasDepth.SetAsResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::CopyDepth);
	}

	void CRenderManager::CheckIsolatedRenderPass()
	{
		switch (CurrentRunningRenderPass)
		{
		case Havtorn::ERenderPass::All:
			break;
		case Havtorn::ERenderPass::Depth:
		{
			RenderedScene.SetAsActiveTarget();
			DepthCopy.SetAsResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::CopyDepth);
		}
		break;
		case Havtorn::ERenderPass::GBufferAlbedo:
		{
			RenderedScene.SetAsActiveTarget();
			GBuffer.SetAsResourceOnSlot(CGBuffer::EGBufferTextures::Albedo, 0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::GBufferNormals:
		{
			RenderedScene.SetAsActiveTarget();
			GBuffer.SetAsResourceOnSlot(CGBuffer::EGBufferTextures::Normal, 0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::GBufferMaterials:
		{
			RenderedScene.SetAsActiveTarget();
			GBuffer.SetAsResourceOnSlot(CGBuffer::EGBufferTextures::Material, 0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::SSAO:
		{
			RenderedScene.SetAsActiveTarget();
			SSAOBlurTexture.SetAsResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::DeferredLighting:
		{
			RenderedScene.SetAsActiveTarget();
			LitScene.SetAsResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::VolumetricLighting:
		{
			RenderedScene.SetAsActiveTarget();
			VolumetricAccumulationBuffer.SetAsResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::Bloom:
		{
			RenderedScene.SetAsActiveTarget();
			VignetteTexture.SetAsResourceOnSlot(0);
			HalfSizeTexture.SetAsResourceOnSlot(1);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Difference);
		}
		break;
		case Havtorn::ERenderPass::Tonemapping:
		{
			RenderedScene.SetAsActiveTarget();
			TonemappedTexture.SetAsResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::Antialiasing:
		{
			RenderedScene.SetAsActiveTarget();
			AntiAliasedTexture.SetAsResourceOnSlot(0);
			TonemappedTexture.SetAsResourceOnSlot(1);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Difference);
		}
		break;
		case Havtorn::ERenderPass::Count:
			break;
		default:
			break;
		}
	}

	void CRenderManager::CycleRenderPass(const SInputActionPayload payload)
	{
		if (!payload.IsPressed)
			return;

		U8 currentRunningRenderPassIndex = static_cast<U8>(CurrentRunningRenderPass);
		switch (payload.Event)
		{
		case EInputActionEvent::CycleRenderPassForward:
			currentRunningRenderPassIndex = (currentRunningRenderPassIndex + 1) % static_cast<U8>(ERenderPass::Count);
			break;

		case EInputActionEvent::CycleRenderPassBackward:
		{
			U8 maxIndex = static_cast<U8>(ERenderPass::Count) - 1;
			currentRunningRenderPassIndex = UMath::Min(--currentRunningRenderPassIndex, maxIndex);
		}
			break;

		case EInputActionEvent::CycleRenderPassReset:
			currentRunningRenderPassIndex = static_cast<U8>(ERenderPass::All);
			break;

		default:
			break;
		}

		HV_LOG_TRACE("Render Pass Index: %i", currentRunningRenderPassIndex);
		CurrentRunningRenderPass = static_cast<ERenderPass>(currentRunningRenderPassIndex);
	}

	bool SRenderCommandComparer::operator()(const SRenderCommand& a, const SRenderCommand& b) const
	{
		return 	static_cast<U16>(a.Type) > static_cast<U16>(b.Type);
	}
}
