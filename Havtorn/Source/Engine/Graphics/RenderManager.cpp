// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderManager.h"
#include "GraphicsUtilities.h"
#include <GeneralUtilities.h>
#include <MathTypes/MathUtilities.h>

#include "Engine.h"
#include "Input/InputMapper.h"
#include "Scene/World.h"
#include "Assets/AssetRegistry.h"

#include "Debug/DebugDrawUtility.h"

#include "ECS/ECSInclude.h"

#include "GraphicsStructs.h"
#include "GeometryPrimitives.h"
#include "Assets/FileHeaderDeclarations.h"

#include <algorithm>
#include <future>

#include <PlatformManager.h>

#include "Threading/ThreadManager.h"
#include "TextureBank.h"

#include <DirectXTex/DirectXTex.h>
#include <set>

namespace Havtorn
{
	U32 CRenderManager::NumberOfDrawCallsThisFrame = 0;

	CRenderManager::~CRenderManager()
	{
		Release(SVector2<U16>::Zero);
	}

	bool CRenderManager::Init(CGraphicsFramework* framework, CPlatformManager* platformManager)
	{
		Framework = framework;

		ENGINE_ERROR_BOOL_MESSAGE(FullscreenRenderer.Init(framework, this), "Failed to Init Fullscreen Renderer.");
		ENGINE_ERROR_BOOL_MESSAGE(RenderTextureFactory.Init(framework), "Failed to Init Fullscreen Texture Factory.");
		ENGINE_ERROR_BOOL_MESSAGE(RenderStateManager.Init(framework), "Failed to Init Render State Manager.");

		ID3D11Texture2D* backbufferTexture = framework->GetBackbufferTexture();
		ENGINE_ERROR_BOOL_MESSAGE(backbufferTexture, "Backbuffer Texture is null.");

		Backbuffer = RenderTextureFactory.CreateTexture(backbufferTexture);
		InitRenderTextures(platformManager->GetResolution());

		InitDataBuffers();

		BindRenderFunctions();

		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassForward).AddMember(this, &CRenderManager::CycleRenderPass);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassBackward).AddMember(this, &CRenderManager::CycleRenderPass);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassReset).AddMember(this, &CRenderManager::CycleRenderPass);
		// TODO.NW: Bind to on resolution changed?

		return true;
	}

	bool CRenderManager::ReInit(CGraphicsFramework* framework, SVector2<U16> newResolution)
	{
		ENGINE_ERROR_BOOL_MESSAGE(RenderStateManager.Init(framework), "Failed to Init Render State Manager.");

		ID3D11Texture2D* backbufferTexture = framework->GetBackbufferTexture();
		ENGINE_ERROR_BOOL_MESSAGE(backbufferTexture, "Backbuffer Texture is null.");

		Backbuffer = RenderTextureFactory.CreateTexture(backbufferTexture);
		InitRenderTextures(newResolution);

		return true;
	}

	void CRenderManager::InitRenderTextures(SVector2<U16> windowResolution)
	{
		CurrentWindowResolution = windowResolution;

		RenderedScene = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);
		LitScene = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);
		IntermediateDepth = RenderTextureFactory.CreateDepth(windowResolution, DXGI_FORMAT_R24G8_TYPELESS);
		EditorWidgetDepth = RenderTextureFactory.CreateDepth(windowResolution, DXGI_FORMAT_R24G8_TYPELESS);

		ShadowAtlasResolution = { 8192.0f, 8192.0f };
		InitShadowmapAtlas(ShadowAtlasResolution);

		DepthCopy = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R32_FLOAT);
		DownsampledDepth = RenderTextureFactory.CreateTexture(windowResolution / 2, DXGI_FORMAT_R32_FLOAT);
		
		IntermediateTexture = RenderTextureFactory.CreateTexture(SVector2<U16>(STATIC_U16(ShadowAtlasResolution.X), STATIC_U16(ShadowAtlasResolution.Y)), DXGI_FORMAT_R16G16B16A16_FLOAT);

		HalfSizeTexture = RenderTextureFactory.CreateTexture(windowResolution / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);
		QuarterSizeTexture = RenderTextureFactory.CreateTexture(windowResolution / 4, DXGI_FORMAT_R16G16B16A16_FLOAT);
		BlurTexture1 = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);
		BlurTexture2 = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);
		VignetteTexture = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);

		VolumetricAccumulationBuffer = RenderTextureFactory.CreateTexture(windowResolution / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);
		VolumetricBlurTexture = RenderTextureFactory.CreateTexture(windowResolution / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);

		SSAOBuffer = RenderTextureFactory.CreateTexture(windowResolution / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);
		SSAOBlurTexture = RenderTextureFactory.CreateTexture(windowResolution / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);

		TonemappedTexture = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);
		AntiAliasedTexture = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);
		EditorDataTexture = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R32G32_UINT, true);
		SkeletalAnimationDataTextureCPU = RenderTextureFactory.CreateTexture({ 256, 256 }, DXGI_FORMAT_R32G32B32A32_FLOAT, true);
		SkeletalAnimationDataTextureGPU = RenderTextureFactory.CreateTexture({ 256, 256 }, DXGI_FORMAT_R32G32B32A32_FLOAT);
		GBuffer = RenderTextureFactory.CreateGBuffer(windowResolution);
	}

	void CRenderManager::InitShadowmapAtlas(SVector2<F32> atlasResolution)
	{
		ShadowAtlasDepth = RenderTextureFactory.CreateDepth(SVector2<U16>(STATIC_U16(atlasResolution.X), STATIC_U16(atlasResolution.Y)), DXGI_FORMAT_R32_TYPELESS);

		auto initShadowmapLOD = [this, atlasResolution](U16 mapsInLod, U16 startIndex, const SVector2<F32>& topLeftCoordinate)
			{
				const SVector2<F32> depth = { 0.0f, 1.0f };
				const SVector2<F32> widthAndHeight = atlasResolution * (1.0f / (mapsInLod / 2));
				InitShadowmapLOD(topLeftCoordinate, widthAndHeight, depth, atlasResolution, mapsInLod, startIndex);
			};

		initShadowmapLOD(8, 0, SVector2<F32>::Zero);
		initShadowmapLOD(16, 8, { 0.0f, atlasResolution.Y * 0.5f });
		initShadowmapLOD(32, 24, { 0.0f, atlasResolution.Y * 0.75f });
		initShadowmapLOD(128, 56, { 0.0f, atlasResolution.Y * 0.875f });
	}

	void CRenderManager::InitShadowmapLOD(SVector2<F32> topLeftCoordinate, const SVector2<F32>& widthAndHeight, const SVector2<F32>& depth, const SVector2<F32>& atlasResolution, U16 mapsInLod, U16 startIndex)
	{
		const float startingYCoordinate = topLeftCoordinate.Y;
		const U16 mapsPerRow = STATIC_U16(atlasResolution.X / widthAndHeight.X);
		for (U16 i = startIndex; i < startIndex + mapsInLod; i++)
		{
			const U16 relativeIndex = i - startIndex;
			topLeftCoordinate.X = STATIC_U16(relativeIndex % mapsPerRow) * widthAndHeight.X;
			topLeftCoordinate.Y = startingYCoordinate + STATIC_U16(relativeIndex / mapsPerRow) * widthAndHeight.Y;
			RenderStateManager.AddViewport(topLeftCoordinate, widthAndHeight, depth);
		}
	}

	void CRenderManager::BindRenderFunctions()
	{
		RenderFunctions[ERenderCommandType::ShadowAtlasPrePassDirectional] =	std::bind(&CRenderManager::ShadowAtlasPrePassDirectional, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::ShadowAtlasPrePassPoint] =			std::bind(&CRenderManager::ShadowAtlasPrePassPoint, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::ShadowAtlasPrePassSpot] =			std::bind(&CRenderManager::ShadowAtlasPrePassSpot, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::CameraDataStorage] =				std::bind(&CRenderManager::CameraDataStorage, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GBufferDataInstanced] =				std::bind(&CRenderManager::GBufferDataInstanced, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GBufferSkeletalInstanced] =			std::bind(&CRenderManager::GBufferSkeletalInstanced, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GBufferDataInstancedEditor] =		std::bind(&CRenderManager::GBufferDataInstancedEditor, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GBufferSkeletalInstancedEditor] =	std::bind(&CRenderManager::GBufferSkeletalInstancedEditor, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GBufferSpriteInstanced] =			std::bind(&CRenderManager::GBufferSpriteInstanced, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GBufferSpriteInstancedEditor] =		std::bind(&CRenderManager::GBufferSpriteInstancedEditor, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DecalDepthCopy] =					std::bind(&CRenderManager::DecalDepthCopy, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DeferredDecal] =					std::bind(&CRenderManager::DeferredDecal, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PreLightingPass] =					std::bind(&CRenderManager::PreLightingPass, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DeferredLightingDirectional] =		std::bind(&CRenderManager::DeferredLightingDirectional, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DeferredLightingPoint] =			std::bind(&CRenderManager::DeferredLightingPoint, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DeferredLightingSpot] =				std::bind(&CRenderManager::DeferredLightingSpot, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::Skybox] =							std::bind(&CRenderManager::Skybox, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PostBaseLightingPass] =				std::bind(&CRenderManager::PostBaseLightingPass, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::VolumetricLightingDirectional] =	std::bind(&CRenderManager::VolumetricLightingDirectional, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::VolumetricLightingPoint] =			std::bind(&CRenderManager::VolumetricLightingPoint, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::VolumetricLightingSpot] =			std::bind(&CRenderManager::VolumetricLightingSpot, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::VolumetricBufferBlurPass] =			std::bind(&CRenderManager::VolumetricBlur, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::ForwardTransparency] =				std::bind(&CRenderManager::ForwardTransparency, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::ScreenSpaceSprite] =				std::bind(&CRenderManager::ScreenSpaceSprite, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::WorldSpaceSpriteEditorWidget] =		std::bind(&CRenderManager::WorldSpaceSpriteEditorWidget, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::Bloom] =							std::bind(&CRenderManager::RenderBloom, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::Tonemapping] =						std::bind(&CRenderManager::Tonemapping, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PreDebugShape] =					std::bind(&CRenderManager::PreDebugShapes, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PostToneMappingUseDepth] =			std::bind(&CRenderManager::PostTonemappingUseDepth, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PostToneMappingIgnoreDepth] =		std::bind(&CRenderManager::PostTonemappingIgnoreDepth, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DebugShapeUseDepth] =				std::bind(&CRenderManager::DebugShapes, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DebugShapeIgnoreDepth] =			std::bind(&CRenderManager::DebugShapes, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::AntiAliasing] =						std::bind(&CRenderManager::AntiAliasing, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GammaCorrection] =					std::bind(&CRenderManager::GammaCorrection, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::RendererDebug] =					std::bind(&CRenderManager::RendererDebug, this, std::placeholders::_1);
	}

	void CRenderManager::Render()
	{
		while (CThreadManager::RunRenderThread)
		{
			std::unique_lock<std::mutex> uniqueLock(CThreadManager::RenderMutex);
			CThreadManager::RenderCondition.wait(uniqueLock, [] 
				{ return CThreadManager::RenderThreadStatus == ERenderThreadStatus::ReadyToRender
				|| !CThreadManager::RunRenderThread; });

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
			EditorWidgetDepth.ClearDepth();
			VolumetricAccumulationBuffer.ClearTexture();

			if (WorldPlayState != EWorldPlayState::Playing)
			{
				U32 size = (CurrentWindowResolution.X * CurrentWindowResolution.Y);

				void* editorData = EditorDataTexture.MapToCPUFromGPUTexture(GBuffer.GetEditorDataTexture());
				if (editorData != nullptr)
				{
					EntityPerPixelData = std::move(editorData);
					EntityPerPixelDataSize = size;
				}

				EditorDataTexture.UnmapFromCPU();
			}

			if (RendererSkeletalAnimationBoneData != nullptr)
				SkeletalAnimationDataTextureCPU.WriteToCPUTexture(RendererSkeletalAnimationBoneData, SkeletalAnimationBoneDataSize);

			GBuffer.ClearTextures(ClearColor);

			ShadowAtlasDepth.SetAsDepthTarget(&IntermediateTexture);

			const U16 commandsInHeap = STATIC_U16(PopFromCommands->size());
			for (U16 i = 0; i < commandsInHeap; ++i)
			{
				SRenderCommand currentCommand = PopFromCommands->top();
				RenderFunctions[currentCommand.Type](currentCommand);
				PopFromCommands->pop();
			}

			CheckIsolatedRenderPass();

			// RenderedScene should be complete as that is the texture we send to the viewport
			Backbuffer.SetAsActiveTarget();
			RenderedScene.SetAsPSResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

			GTime::EndTracking(ETimerCategory::GPU);

			CThreadManager::RenderThreadStatus = ERenderThreadStatus::PostRender;
			uniqueLock.unlock();
			CThreadManager::RenderCondition.notify_one();
		}
	}

	void CRenderManager::Release(SVector2<U16> newResolution)
	{
		Clear(ClearColor);
		GEngine::Instance->Framework->GetContext()->OMSetRenderTargets(0, 0, 0);
		GEngine::Instance->Framework->GetContext()->OMGetDepthStencilState(0, 0);
		GEngine::Instance->Framework->GetContext()->ClearState();

		// TODO.NR: Implement this properly for window resizing

		Backbuffer.ReleaseTexture();
		//TODO.AS: Is this ultra deep call really neccesary to do here? Context: We need to specifically Resize the SwapChain Buffers Right after we Release
		//the Backbuffer texture. 
		// TODO.NW: Sounds like this should be done in the Resize context, not in this function
		GEngine::Instance->Framework->GetSwapChain()->ResizeBuffers(0, newResolution.X, newResolution.Y, DXGI_FORMAT_UNKNOWN, 0);
	}

	CRenderTexture CRenderManager::RenderStaticMeshAssetTexture(const std::string& filePath)
	{
		SStaticMeshAsset* meshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SStaticMeshAsset>(SAssetReference(filePath), CAssetRegistry::RenderManagerRequestID);

		F32 aspectRatio = 1.0f;
		F32 marginPercentage = 1.5f;
		SVector2<F32> fov = { aspectRatio * 70.0f, 70.0f };

		STransform camTransform;
		camTransform.Orbit(SVector4(), SMatrix::CreateRotationFromEuler(30.0f, 30.0f, 0.0f));
		camTransform.Translate(SVector(meshAsset->BoundsCenter.X, meshAsset->BoundsCenter.Y, -UMathUtilities::GetFocusDistanceForBounds(meshAsset->BoundsCenter, SVector::GetAbsMaxKeepValue(meshAsset->BoundsMax, meshAsset->BoundsMin), fov, marginPercentage)));
		SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(fov.Y), aspectRatio, 0.001f, 100.0f);

		CRenderTexture renderTexture = RenderTextureFactory.CreateTexture(SVector2<U16>(256), DXGI_FORMAT_R16G16B16A16_FLOAT);
		CRenderTexture renderDepth = RenderTextureFactory.CreateDepth(SVector2<U16>(256), DXGI_FORMAT_R24G8_TYPELESS);
		
		// TODO.NW: Figure out why the depth doesn't work
		ID3D11RenderTargetView* renderTargets[1] = { renderTexture.GetRenderTargetView() };
		RenderStateManager.OMSetRenderTargets(1, renderTargets, nullptr/*renderDepth.GetDepthStencilView()*/);
		RenderStateManager.RSSetViewports(1, renderDepth.GetViewport());

		FrameBufferData.ToCameraFromWorld = camTransform.GetMatrix().FastInverse();
		FrameBufferData.ToWorldFromCamera = camTransform.GetMatrix();
		FrameBufferData.ToProjectionFromCamera = camProjection;
		FrameBufferData.ToCameraFromProjection = camProjection.Inverse();
		FrameBufferData.CameraPosition = camTransform.GetMatrix().GetTranslation4();
		FrameBuffer.BindBuffer(FrameBufferData);

		RenderStateManager.VSSetConstantBuffer(0, FrameBuffer);
		RenderStateManager.PSSetConstantBuffer(0, FrameBuffer);

		ObjectBufferData.ToWorldFromObject = SMatrix();
		ObjectBuffer.BindBuffer(ObjectBufferData);

		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2);

		RenderStateManager.VSSetShader(EVertexShaders::EditorPreviewStaticMesh);
		RenderStateManager.PSSetShader(EPixelShaders::EditorPreview);
		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		for (U8 drawCallIndex = 0; drawCallIndex < STATIC_U8(meshAsset->DrawCallData.size()); drawCallIndex++)
		{
			const SDrawCallData& drawData = meshAsset->DrawCallData[drawCallIndex];
			RenderStateManager.IASetVertexBuffer(0, RenderStateManager.VertexBuffers[drawData.VertexBufferIndex], RenderStateManager.MeshVertexStrides[drawData.VertexStrideIndex], RenderStateManager.MeshVertexOffsets[drawData.VertexOffsetIndex]);
			RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[drawData.IndexBufferIndex]);
			RenderStateManager.DrawIndexed(drawData.IndexCount, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		RenderStateManager.ClearState();
		Backbuffer.SetAsActiveTarget();

		//void* resource = renderTexture.MoveShaderResourceView();
		//renderTexture.Release();
		renderDepth.Release();
		GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::RenderManagerRequestID);

		return std::move(renderTexture);
	}

	CRenderTexture CRenderManager::RenderSkeletalMeshAssetTexture(const std::string& filePath)
	{		
		SSkeletalMeshAsset* meshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalMeshAsset>(SAssetReference(filePath), CAssetRegistry::RenderManagerRequestID);

		F32 aspectRatio = 1.0f;
		F32 marginPercentage = 1.5f;
		SVector2<F32> fov = { aspectRatio * 70.0f, 70.0f };

		STransform camTransform;
		camTransform.Orbit(SVector4(), SMatrix::CreateRotationFromEuler(30.0f, 30.0f, 0.0f));
		camTransform.Translate(SVector(meshAsset->BoundsCenter.X, meshAsset->BoundsCenter.Y, -UMathUtilities::GetFocusDistanceForBounds(meshAsset->BoundsCenter, SVector::GetAbsMaxKeepValue(meshAsset->BoundsMax, meshAsset->BoundsMin), fov, marginPercentage)));
		SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(fov.Y), aspectRatio, 0.001f, 100.0f);

		CRenderTexture renderTexture = RenderTextureFactory.CreateTexture(SVector2<U16>(256), DXGI_FORMAT_R16G16B16A16_FLOAT);
		CRenderTexture renderDepth = RenderTextureFactory.CreateDepth(SVector2<U16>(256), DXGI_FORMAT_R24G8_TYPELESS);

		const std::vector<SMatrix>& matrices = { SMatrix::Identity };
		InstancedTransformBuffer.BindBuffer(matrices);

		// TODO.NW: Figure out why the depth doesn't work
		ID3D11RenderTargetView* renderTargets[1] = { renderTexture.GetRenderTargetView() };
		RenderStateManager.OMSetRenderTargets(1, renderTargets, nullptr/*renderDepth.GetDepthStencilView()*/);
		RenderStateManager.RSSetViewports(1, renderDepth.GetViewport());

		FrameBufferData.ToCameraFromWorld = camTransform.GetMatrix().FastInverse();
		FrameBufferData.ToWorldFromCamera = camTransform.GetMatrix();
		FrameBufferData.ToProjectionFromCamera = camProjection;
		FrameBufferData.ToCameraFromProjection = camProjection.Inverse();
		FrameBufferData.CameraPosition = camTransform.GetMatrix().GetTranslation4();
		FrameBuffer.BindBuffer(FrameBufferData);
		
		std::vector<SMatrix> boneTransforms;
		boneTransforms.resize(64, SMatrix::Identity);
		BoneBuffer.BindBuffer(boneTransforms);

		RenderStateManager.VSSetConstantBuffer(0, FrameBuffer);
		RenderStateManager.PSSetConstantBuffer(0, FrameBuffer);

		ObjectBufferData.ToWorldFromObject = SMatrix();
		ObjectBuffer.BindBuffer(ObjectBufferData);

		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4AnimDataTrans);

		RenderStateManager.VSSetShader(EVertexShaders::EditorPreviewSkeletalMesh);
		RenderStateManager.PSSetShader(EPixelShaders::EditorPreview);
		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);
		
		RenderStateManager.VSSetConstantBuffer(2, BoneBuffer);

		for (U8 drawCallIndex = 0; drawCallIndex < STATIC_U8(meshAsset->DrawCallData.size()); drawCallIndex++)
		{
			const SDrawCallData& drawData = meshAsset->DrawCallData[drawCallIndex];
			const std::vector<CDataBuffer> buffers = { RenderStateManager.VertexBuffers[drawData.VertexBufferIndex], InstancedAnimationDataBuffer, InstancedTransformBuffer };
			const U32 strides[3] = { RenderStateManager.MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SVector2<U32>), sizeof(SMatrix) };
			const U32 offsets[3] = { RenderStateManager.MeshVertexOffsets[drawData.VertexOffsetIndex], 0, 0 };
			RenderStateManager.IASetVertexBuffers(0, 3, buffers, strides, offsets);
			RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[drawData.IndexBufferIndex]);
			RenderStateManager.DrawIndexed(drawData.IndexCount, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		RenderStateManager.ClearState();
		Backbuffer.SetAsActiveTarget();

		//void* resource = renderTexture.MoveShaderResourceView();
		//renderTexture.Release();
		renderDepth.Release();
		GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::RenderManagerRequestID);

		return std::move(renderTexture);
	}

	CRenderTexture CRenderManager::RenderSkeletalAnimationAssetTexture(const std::string& filePath, const std::vector<SMatrix>& boneTransforms)
	{
		CRenderTexture renderTexture = RenderTextureFactory.CreateTexture(SVector2<U16>(256), DXGI_FORMAT_R16G16B16A16_FLOAT);
		RenderSkeletalAnimationAssetTexture(renderTexture, filePath, boneTransforms);

		return std::move(renderTexture);
	}

	CRenderTexture CRenderManager::RenderMaterialAssetTexture(const std::string& filePath)
	{
		SGraphicsMaterialAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<SGraphicsMaterialAsset>(SAssetReference(filePath), CAssetRegistry::RenderManagerRequestID);
		CRenderTexture renderTexture = RenderTextureFactory.CreateTexture(SVector2<U16>(256), DXGI_FORMAT_R16G16B16A16_FLOAT);
		RenderMaterialTexture(renderTexture, asset->Material);
		GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::RenderManagerRequestID);
		return std::move(renderTexture);
	}

	void CRenderManager::RenderMaterialTexture(CRenderTexture& textureTarget, const SEngineGraphicsMaterial& material, const SVector& objectRotationEuler, const F32 zoomMultiplier)
	{
		// CameraDataStorage();
		const F32 radius = 2.0f * zoomMultiplier;
		SVector location = SVector(0.0f, 0.0f, radius);
		SMatrix camMatrix = SMatrix::LookAtLH(location, SVector(), SVector::Up).FastInverse();
		
		STransform camViewMatrix;
		camViewMatrix.SetMatrix(camMatrix);

		SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), 1.0f, 0.01f, 10.0f);

		FrameBufferData.ToCameraFromWorld = camViewMatrix.GetMatrix().FastInverse();
		FrameBufferData.ToWorldFromCamera = camViewMatrix.GetMatrix();
		FrameBufferData.ToProjectionFromCamera = camProjection;
		FrameBufferData.ToCameraFromProjection = camProjection.Inverse();
		FrameBufferData.CameraPosition = camViewMatrix.GetMatrix().GetTranslation4();
		FrameBuffer.BindBuffer(FrameBufferData);

		RenderStateManager.VSSetConstantBuffer(0, FrameBuffer);
		RenderStateManager.PSSetConstantBuffer(0, FrameBuffer);

		//GBufferDataInstanced();
		CGBuffer gBuffer = RenderTextureFactory.CreateGBuffer(SVector2<U16>(256));
		gBuffer.SetAsActiveTarget();
		RenderStateManager.RSSetViewports(1, &gBuffer.GetViewport());

		SMatrix objectMatrix = SMatrix::CreateRotationFromEuler(SVector(objectRotationEuler.X, objectRotationEuler.Y, 0.0f)).FastInverse();
		ObjectBufferData.ToWorldFromObject = objectMatrix;
		ObjectBuffer.BindBuffer(ObjectBufferData);

		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2);

		RenderStateManager.VSSetShader(EVertexShaders::StaticMesh);
		RenderStateManager.PSSetShader(EPixelShaders::GBuffer);
		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);
		RenderStateManager.PSSetSampler(1, ESamplers::DefaultBorder);

		MaterialBufferData = SMaterialBufferData(material);
		std::vector<ID3D11ShaderResourceView*> resourceViewPointers;
		std::map<U32, F32> textureUIDToShaderIndex;
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoR)], resourceViewPointers, textureUIDToShaderIndex);
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoG)], resourceViewPointers, textureUIDToShaderIndex);
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoB)], resourceViewPointers, textureUIDToShaderIndex);
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoA)], resourceViewPointers, textureUIDToShaderIndex);
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalX)], resourceViewPointers, textureUIDToShaderIndex);
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalY)], resourceViewPointers, textureUIDToShaderIndex);
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalZ)], resourceViewPointers, textureUIDToShaderIndex);
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AmbientOcclusion)], resourceViewPointers, textureUIDToShaderIndex);
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Metalness)], resourceViewPointers, textureUIDToShaderIndex);
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Roughness)], resourceViewPointers, textureUIDToShaderIndex);
		MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Emissive)], resourceViewPointers, textureUIDToShaderIndex);

		for (auto const& [textureUID, shaderIndex] : textureUIDToShaderIndex)
			GEngine::GetAssetRegistry()->UnrequestAsset(textureUID, CAssetRegistry::RenderManagerRequestID);

		MaterialBuffer.BindBuffer(MaterialBufferData);

		RenderStateManager.PSSetResources(5, STATIC_U8(resourceViewPointers.size()), resourceViewPointers.data());
		RenderStateManager.PSSetConstantBuffer(8, MaterialBuffer);

		RenderStateManager.IASetVertexBuffer(0, RenderStateManager.VertexBuffers[STATIC_U8(EVertexBufferPrimitives::Icosphere)], RenderStateManager.MeshVertexStrides[0], RenderStateManager.MeshVertexOffsets[0]);
		RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[STATIC_U8(EIndexBufferPrimitives::Icosphere)]);
		RenderStateManager.DrawIndexed(STATIC_U32(GeometryPrimitives::Icosphere.Indices.size()), 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;

		//PreLightingPass();
		// === SSAO ===
		// TODO.NW: As part of next rendering refactor we should make it easier to set up render passes, somehow indicating the necessary resources e.g. render textures
		SSAOBuffer.SetAsActiveTarget();
		gBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Normal, 2);
		IntermediateDepth.SetAsPSResourceOnSlot(21);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::SSAO);
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::Disable);

		SSAOBlurTexture.SetAsActiveTarget();
		SSAOBuffer.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::SSAOBlur);
		// === !SSAO ===

		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);
		RenderStateManager.PSSetSampler(1, ESamplers::DefaultBorder);

		RenderStateManager.GSSetShader(EGeometryShaders::Null);

		textureTarget.ClearTexture();
		textureTarget.SetAsActiveTarget();
		gBuffer.SetAllAsResources(1);
		IntermediateDepth.SetAsPSResourceOnSlot(21);
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		
		//DeferredLightingDirectional();
		// add Alpha blend PS shader
		ShadowAtlasDepth.SetAsPSResourceOnSlot(22);
		SSAOBlurTexture.SetAsPSResourceOnSlot(23);

		STextureAsset* cubemapTexture = GEngine::GetAssetRegistry()->RequestAssetData<STextureAsset>(SAssetReference("Resources/DefaultSkybox.hva"), CAssetRegistry::RenderManagerRequestID);
		RenderStateManager.PSSetResources(0, 1, cubemapTexture->RenderTexture.GetShaderResourceView());

		const SVector4 directionalLightDirection = { -1.0f, 0.0f, -1.0f, 0.0f };
		const SVector4 directionalLightColor = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 0.25f };

		// Update lightbufferdata and fill lightbuffer
		DirectionalLightBufferData.DirectionalLightDirection = directionalLightDirection;
		DirectionalLightBufferData.DirectionalLightColor = directionalLightColor;
		DirectionalLightBuffer.BindBuffer(DirectionalLightBufferData);
		RenderStateManager.PSSetConstantBuffer(2, DirectionalLightBuffer);

		ShadowmapBufferData.ToShadowmapView = SMatrix::Identity;
		ShadowmapBufferData.ToShadowmapProjection = SMatrix::OrthographicLH(8.0f, 8.0f, -8.0f, 8.0f);
		ShadowmapBufferData.ShadowmapPosition = SVector4::Zero;

		const auto& shadowmapViewport = RenderStateManager.Viewports[0];
		ShadowmapBufferData.ShadowmapResolution = { shadowmapViewport.Width, shadowmapViewport.Height };
		ShadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
		ShadowmapBufferData.ShadowmapStartingUV = { shadowmapViewport.TopLeftX / ShadowAtlasResolution.X, shadowmapViewport.TopLeftY / ShadowAtlasResolution.Y };
		ShadowmapBufferData.ShadowTestTolerance = 0.001f;

		ShadowmapBuffer.BindBuffer(ShadowmapBufferData);
		RenderStateManager.PSSetConstantBuffer(5, ShadowmapBuffer);

		// Emissive Post Processing 
		EmissiveBufferData.EmissiveStrength = FullscreenRenderer.PostProcessingBufferData.EmissiveStrength;
		EmissiveBuffer.BindBuffer(EmissiveBufferData);
		RenderStateManager.PSSetConstantBuffer(7, EmissiveBuffer);

		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Null);
		RenderStateManager.IASetVertexBuffer(0, CDataBuffer::Null, 0, 0);
		RenderStateManager.IASetIndexBuffer(CDataBuffer::Null);

		RenderStateManager.VSSetShader(EVertexShaders::Fullscreen);
		RenderStateManager.PSSetShader(EPixelShaders::DeferredDirectional);

		RenderStateManager.Draw(3, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;

		//PostBaseLightingPass();
		CRenderTexture targetCopy = RenderTextureFactory.CreateTexture(SVector2<U16>(256), DXGI_FORMAT_R16G16B16A16_FLOAT);
		targetCopy.SetAsActiveTarget();
		textureTarget.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		//RenderBloom();

		//Tonemapping();
		textureTarget.SetAsActiveTarget();
		targetCopy.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Tonemap);
		
		//AntiAliasing();
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::Disable);
		RenderStateManager.OMSetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);
		targetCopy.SetAsActiveTarget();
		textureTarget.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::FXAA);

		//GammaCorrection();
		textureTarget.SetAsActiveTarget();
		targetCopy.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		ID3D11ShaderResourceView* nullView = NULL;
		RenderStateManager.PSSetResources(1, 1, &nullView);
		RenderStateManager.PSSetResources(2, 1, &nullView);
		RenderStateManager.PSSetResources(3, 1, &nullView);
		RenderStateManager.PSSetResources(4, 1, &nullView);
		RenderStateManager.PSSetResources(21, 1, &nullView);
		RenderStateManager.PSSetResources(22, 1, &nullView);
		RenderStateManager.PSSetResources(23, 1, &nullView);

		RenderStateManager.ClearState();
		Backbuffer.SetAsActiveTarget();

		gBuffer.ReleaseResources();
		targetCopy.Release();
		GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference("Resources/DefaultSkybox.hva"), CAssetRegistry::RenderManagerRequestID);
	}

	void CRenderManager::RenderSkeletalAnimationAssetTexture(CRenderTexture& assetTextureTarget, const std::string& filePath, const std::vector<SMatrix>& boneTransforms)
	{
		SSkeletalAnimationAsset* animationAsset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalAnimationAsset>(SAssetReference(filePath), CAssetRegistry::RenderManagerRequestID);
		SSkeletalMeshAsset* meshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalMeshAsset>(SAssetReference(animationAsset->RigPath), CAssetRegistry::RenderManagerRequestID);

		F32 aspectRatio = 1.0f;
		F32 marginPercentage = 1.5f;
		SVector2<F32> fov = { aspectRatio * 70.0f, 70.0f };

		STransform camTransform;
		camTransform.Orbit(SVector4(), SMatrix::CreateRotationFromEuler(30.0f, 30.0f, 0.0f));
		camTransform.Translate(SVector(meshAsset->BoundsCenter.X, meshAsset->BoundsCenter.Y, -UMathUtilities::GetFocusDistanceForBounds(meshAsset->BoundsCenter, SVector::GetAbsMaxKeepValue(meshAsset->BoundsMax, meshAsset->BoundsMin), fov, marginPercentage)));
		SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(fov.Y), aspectRatio, 0.001f, 100.0f);

		CRenderTexture renderDepth = RenderTextureFactory.CreateDepth(SVector2<U16>(256), DXGI_FORMAT_R24G8_TYPELESS);

		const std::vector<SMatrix>& matrices = { SMatrix::Identity };
		InstancedTransformBuffer.BindBuffer(matrices);

		// TODO.NW: Figure out why the depth doesn't work
		assetTextureTarget.ClearTexture();
		ID3D11RenderTargetView* renderTargets[1] = { assetTextureTarget.GetRenderTargetView() };
		RenderStateManager.OMSetRenderTargets(1, renderTargets, nullptr/*renderDepth.GetDepthStencilView()*/);
		RenderStateManager.RSSetViewports(1, renderDepth.GetViewport());

		FrameBufferData.ToCameraFromWorld = camTransform.GetMatrix().FastInverse();
		FrameBufferData.ToWorldFromCamera = camTransform.GetMatrix();
		FrameBufferData.ToProjectionFromCamera = camProjection;
		FrameBufferData.ToCameraFromProjection = camProjection.Inverse();
		FrameBufferData.CameraPosition = camTransform.GetMatrix().GetTranslation4();
		FrameBuffer.BindBuffer(FrameBufferData);

		std::vector<SMatrix> bones = boneTransforms;
		if (bones.empty())
			bones.resize(64, SMatrix::Identity);
		BoneBuffer.BindBuffer(bones);

		RenderStateManager.VSSetConstantBuffer(0, FrameBuffer);
		RenderStateManager.PSSetConstantBuffer(0, FrameBuffer);

		ObjectBufferData.ToWorldFromObject = SMatrix();
		ObjectBuffer.BindBuffer(ObjectBufferData);

		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4AnimDataTrans);

		RenderStateManager.VSSetShader(EVertexShaders::EditorPreviewSkeletalMesh);
		RenderStateManager.PSSetShader(EPixelShaders::EditorPreview);
		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		RenderStateManager.VSSetConstantBuffer(2, BoneBuffer);

		for (U8 drawCallIndex = 0; drawCallIndex < STATIC_U8(meshAsset->DrawCallData.size()); drawCallIndex++)
		{
			const SDrawCallData& drawData = meshAsset->DrawCallData[drawCallIndex];
			const std::vector<CDataBuffer> buffers = { RenderStateManager.VertexBuffers[drawData.VertexBufferIndex], InstancedAnimationDataBuffer, InstancedTransformBuffer };
			const U32 strides[3] = { RenderStateManager.MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SVector2<U32>), sizeof(SMatrix) };
			const U32 offsets[3] = { RenderStateManager.MeshVertexOffsets[drawData.VertexOffsetIndex], 0, 0 };
			RenderStateManager.IASetVertexBuffers(0, 3, buffers, strides, offsets);
			RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[drawData.IndexBufferIndex]);
			RenderStateManager.DrawIndexed(drawData.IndexCount, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		RenderStateManager.ClearState();
		Backbuffer.SetAsActiveTarget();

		renderDepth.Release();

		GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(animationAsset->RigPath), CAssetRegistry::RenderManagerRequestID);
		GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::RenderManagerRequestID);
	}

	CRenderTexture CRenderManager::CreateRenderTextureFromSource(const std::string& filePath)
	{
		return std::move(RenderTextureFactory.CreateSRVFromSource(filePath));
	}

	CRenderTexture CRenderManager::CreateRenderTextureFromAsset(const std::string& filePath)
	{
		return std::move(RenderTextureFactory.CreateSRVFromAsset(filePath));
	}

	U64 CRenderManager::GetEntityGUIDFromData(U64 dataIndex) const
	{
		if (dataIndex < EntityPerPixelDataSize)
		{
			U64* dataCopy = reinterpret_cast<U64*>(EntityPerPixelData);
			return dataCopy[dataIndex];
		}

		return 0;
	}

	U32 CRenderManager::WriteToAnimationDataTexture(const std::string& /*animationName*/)
	{
		//if (!LoadedSkeletalAnims.contains(animationName))
		//	return 0;

		//SSkeletalAnimationAsset& asset = LoadedSkeletalAnims.at(animationName);
		//SystemSkeletalAnimationBoneData = asset.EncodedBoneAnimTransforms.data();
		//SkeletalAnimationBoneDataSize = sizeof(SBoneAnimDataTransform) * asset.EncodedBoneAnimTransforms.size();

		// TODO.NR: Return index of animation in texture
		return 0;
	}

	bool Havtorn::CRenderManager::IsStaticMeshInInstancedRenderList(const U32 meshUID)
	{
		return SystemStaticMeshInstanceData.contains(meshUID);
	}

	void CRenderManager::AddStaticMeshToInstancedRenderList(const U32 meshUID, const STransformComponent* component)
	{
		if (!SystemStaticMeshInstanceData.contains(meshUID))
			SystemStaticMeshInstanceData.emplace(meshUID, SStaticMeshInstanceData());

		SystemStaticMeshInstanceData[meshUID].Transforms.emplace_back(component->Transform.GetMatrix());
		SystemStaticMeshInstanceData[meshUID].Entities.emplace_back(component->Owner);
	}

	void CRenderManager::SwapStaticMeshInstancedRenderLists()
	{
		std::swap(SystemStaticMeshInstanceData, RendererStaticMeshInstanceData);
	}

	void CRenderManager::ClearSystemStaticMeshInstanceData()
	{
		SystemStaticMeshInstanceData.clear();
	}

	bool CRenderManager::IsSkeletalMeshInInstancedRenderList(const U32 meshUID)
	{
		return SystemSkeletalMeshInstanceData.contains(meshUID);
	}

	void CRenderManager::AddSkeletalMeshToInstancedRenderList(const U32 meshUID, const STransformComponent* transformComponent, const SSkeletalAnimationComponent* animationComponent)
	{
		if (!SystemSkeletalMeshInstanceData.contains(meshUID))
			SystemSkeletalMeshInstanceData.emplace(meshUID, SSkeletalMeshInstanceData());

		SystemSkeletalMeshInstanceData[meshUID].Transforms.emplace_back(transformComponent->Transform.GetMatrix());
		SystemSkeletalMeshInstanceData[meshUID].Entities.emplace_back(transformComponent->Owner);

		if (SComponent::IsValid(animationComponent))
			SystemSkeletalMeshInstanceData[meshUID].Bones = animationComponent->Bones;
		//else
		//	SystemSkeletalMeshInstanceData[meshName].Bones.emplace_back({});
	}

	void CRenderManager::SwapSkeletalMeshInstancedRenderLists()
	{
		std::swap(SystemSkeletalMeshInstanceData, RendererSkeletalMeshInstanceData);
	}

	void CRenderManager::ClearSystemSkeletalMeshInstanceData()
	{
		SystemSkeletalMeshInstanceData.clear();
	}

	bool CRenderManager::IsSpriteInWorldSpaceInstancedRenderList(const U32 textureBankIndex)
	{
		return SystemWorldSpaceSpriteInstanceData.contains(textureBankIndex);
	}

	void CRenderManager::AddSpriteToWorldSpaceInstancedRenderList(const U32 textureBankIndex, const STransformComponent* worldSpaceTransform, const SSpriteComponent* spriteComponent)
	{
		if (!SystemWorldSpaceSpriteInstanceData.contains(textureBankIndex))
			SystemWorldSpaceSpriteInstanceData.emplace(textureBankIndex, SSpriteInstanceData());

		SystemWorldSpaceSpriteInstanceData[textureBankIndex].Transforms.emplace_back(worldSpaceTransform->Transform.GetMatrix());	
		SystemWorldSpaceSpriteInstanceData[textureBankIndex].UVRects.emplace_back(spriteComponent->UVRect);
		SystemWorldSpaceSpriteInstanceData[textureBankIndex].Colors.emplace_back(spriteComponent->Color.AsVector4());
		SystemWorldSpaceSpriteInstanceData[textureBankIndex].Entities.emplace_back(spriteComponent->Owner);
	}

	void CRenderManager::AddSpriteToWorldSpaceInstancedRenderList(const U32 textureBankIndex, const STransformComponent* worldSpaceTransform, const STransformComponent* cameraTransform)
	{
		if (!SystemWorldSpaceSpriteInstanceData.contains(textureBankIndex))
			SystemWorldSpaceSpriteInstanceData.emplace(textureBankIndex, SSpriteInstanceData());

		const SMatrix cameraMatrix = cameraTransform->Transform.GetMatrix();
		SMatrix orientedMatrix = worldSpaceTransform->Transform.GetMatrix();

		SVector location = orientedMatrix.GetTranslation();
		SVector euler = cameraMatrix.GetEuler();
		constexpr F32 distanceNormalization = 7.0f;
		constexpr F32 scaleMin = 0.15f;
		constexpr F32 scaleMax = 0.5f;
		F32 dist = cameraMatrix.GetTranslation().Distance(location);
		F32 eased = UMath::EaseInOutQuad(dist / distanceNormalization);
		F32 scaling = UMath::Remap(0.0f, 1.0f, scaleMin, scaleMax, eased);
		SVector scale = SVector(scaling, scaling, 1.0f);
		SMatrix::Recompose(location, euler, scale, orientedMatrix);
		
		SystemWorldSpaceSpriteInstanceData[textureBankIndex].Transforms.emplace_back(orientedMatrix);
		SystemWorldSpaceSpriteInstanceData[textureBankIndex].UVRects.emplace_back(SVector4(0.0f, 0.0f, 1.0f, 1.0f));
		SystemWorldSpaceSpriteInstanceData[textureBankIndex].Colors.emplace_back(SVector4(1.0f, 1.0f, 1.0f, 1.0f));
		SystemWorldSpaceSpriteInstanceData[textureBankIndex].Entities.emplace_back(worldSpaceTransform->Owner);
	}

	void CRenderManager::SwapSpriteWorldInstancedRenderLists()
	{
		std::swap(SystemWorldSpaceSpriteInstanceData, RendererWorldSpaceSpriteInstanceData);
	}

	void CRenderManager::ClearSystemWorldSpaceSpriteInstanceData()
	{
		SystemWorldSpaceSpriteInstanceData.clear();
	}

	bool CRenderManager::IsSpriteInScreenSpaceInstancedRenderList(const U32 textureBankIndex)
	{
		return SystemScreenSpaceSpriteInstanceData.contains(textureBankIndex);
	}

	void CRenderManager::AddSpriteToScreenSpaceInstancedRenderList(const U32 textureBankIndex, const STransform2DComponent* screenSpaceTransform, const SSpriteComponent* spriteComponent)
	{
		if (!SystemScreenSpaceSpriteInstanceData.contains(textureBankIndex))
			SystemScreenSpaceSpriteInstanceData.emplace(textureBankIndex, SSpriteInstanceData());

		SMatrix screenSpaceMatrix;
		screenSpaceMatrix.SetScale(screenSpaceTransform->Scale.X, screenSpaceTransform->Scale.Y, 1.0f);
		screenSpaceMatrix *= SMatrix::CreateRotationAroundZ(UMath::DegToRad(screenSpaceTransform->DegreesRoll));
		screenSpaceMatrix.SetTranslation({ screenSpaceTransform->Position.X, screenSpaceTransform->Position.Y, 0.0f });

		SystemScreenSpaceSpriteInstanceData[textureBankIndex].Transforms.emplace_back(screenSpaceMatrix);
		SystemScreenSpaceSpriteInstanceData[textureBankIndex].UVRects.emplace_back(spriteComponent->UVRect);
		SystemScreenSpaceSpriteInstanceData[textureBankIndex].Colors.emplace_back(spriteComponent->Color.AsVector4());
		SystemScreenSpaceSpriteInstanceData[textureBankIndex].Entities.emplace_back(spriteComponent->Owner);
	}

	void CRenderManager::SwapSpriteScreenInstancedRenderLists()
	{
		std::swap(SystemScreenSpaceSpriteInstanceData, RendererScreenSpaceSpriteInstanceData);
	}

	void CRenderManager::ClearSystemScreenSpaceSpriteInstanceData()
	{
		SystemScreenSpaceSpriteInstanceData.clear();
	}

	void CRenderManager::SyncCrossThreadResources(const CWorld* world)
	{
		SwapRenderCommandBuffers();
		SwapStaticMeshInstancedRenderLists();
		SwapSkeletalMeshInstancedRenderLists();
		SwapSpriteWorldInstancedRenderLists();
		SwapSpriteScreenInstancedRenderLists();
		std::swap(SystemSkeletalAnimationBoneData, RendererSkeletalAnimationBoneData);
		SetWorldPlayState(world->GetWorldPlayState());
	}

	void CRenderManager::SetWorldPlayState(EWorldPlayState playState)
	{
		WorldPlayState = playState;
	}

	const CRenderTexture& CRenderManager::GetRenderedSceneTexture() const
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

	const SVector2<U16>& CRenderManager::GetCurrentWindowResolution() const
	{
		return CurrentWindowResolution;
	}

	const SVector2<F32>& CRenderManager::GetShadowAtlasResolution() const
	{
		return ShadowAtlasResolution;
	}

	void CRenderManager::Clear(SVector4 /*clearColor*/)
	{
		//Backbuffer.ClearTexture(clearColor);
		//IntermediateDepth.ClearDepth();
	}

	void CRenderManager::InitDataBuffers()
	{
		FrameBuffer.CreateBuffer("Frame Buffer", Framework, sizeof(SFrameBufferData));
		ObjectBuffer.CreateBuffer("Object Buffer", Framework, sizeof(SObjectBufferData));
		MaterialBuffer.CreateBuffer("Material Buffer", Framework, sizeof(SMaterialBufferData));
		DebugShapeObjectBuffer.CreateBuffer("Debug Shape Object Buffer", Framework, sizeof(SDebugShapeObjectBufferData));
		DecalBuffer.CreateBuffer("Decal Buffer", Framework, sizeof(SDecalBufferData));
		SpriteBuffer.CreateBuffer("Sprite Buffer", Framework, sizeof(SSpriteBufferData));
		DirectionalLightBuffer.CreateBuffer("Directional Light Buffer", Framework, sizeof(SDirectionalLightBufferData));
		PointLightBuffer.CreateBuffer("Point Light Buffer", Framework, sizeof(SPointLightBufferData));
		SpotLightBuffer.CreateBuffer("Spot Light Buffer", Framework, sizeof(SSpotLightBufferData));
		ShadowmapBuffer.CreateBuffer("Shadowmap Buffer", Framework, sizeof(SShadowmapBufferData) * 6);
		VolumetricLightBuffer.CreateBuffer("Volumetric Light Buffer", Framework, sizeof(SVolumetricLightBufferData));
		EmissiveBuffer.CreateBuffer("Emissive Buffer", Framework, sizeof(SEmissiveBufferData));
		BoneBuffer.CreateBuffer("Bone Buffer", Framework, sizeof(SBoneBufferData));

		InstancedTransformBuffer.CreateBuffer("Instanced Transform Buffer", Framework, sizeof(SMatrix) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);
		InstancedAnimationDataBuffer.CreateBuffer("Instanced Animation Data Buffer", Framework, sizeof(SVector2<U32>) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);
		InstancedEntityIDBuffer.CreateBuffer("Instanced Entity ID Buffer", Framework, sizeof(U64) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);
		InstancedUVRectBuffer.CreateBuffer("Instanced UV Rect Buffer", Framework, sizeof(SVector4) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);
		InstancedColorBuffer.CreateBuffer("Instanced Color Buffer", Framework, sizeof(SVector4) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);
	}

	void CRenderManager::ShadowAtlasPrePassDirectional(const SRenderCommand& command)
	{
		const auto& shadowViewData = command.ShadowmapViews[0];
		FrameBufferData.ToCameraFromWorld = shadowViewData.ShadowViewMatrix;
		FrameBufferData.ToWorldFromCamera = shadowViewData.ShadowViewMatrix.FastInverse();
		FrameBufferData.ToProjectionFromCamera = shadowViewData.ShadowProjectionMatrix;
		FrameBufferData.ToCameraFromProjection = shadowViewData.ShadowProjectionMatrix.Inverse();
		FrameBufferData.CameraPosition = shadowViewData.ShadowPosition;
		FrameBuffer.BindBuffer(FrameBufferData);

		RenderStateManager.RSSetViewports(1, &RenderStateManager.Viewports[shadowViewData.ShadowmapViewportIndex]);

		// =============
		ObjectBufferData.ToWorldFromObject = command.Matrices[0];
		ObjectBuffer.BindBuffer(ObjectBufferData);

		const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceData[command.U32s[0]].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans);

		RenderStateManager.VSSetShader(EVertexShaders::StaticMeshInstanced);
		RenderStateManager.PSSetShader(EPixelShaders::Null);

		for (U8 drawCallIndex = 0; drawCallIndex < STATIC_U8(command.DrawCallData.size()); drawCallIndex++)
		{
			const SDrawCallData& drawData = command.DrawCallData[drawCallIndex];
			const std::vector<CDataBuffer> buffers = { RenderStateManager.VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
			const U32 strides[2] = { RenderStateManager.MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
			const U32 offsets[2] = { RenderStateManager.MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
			RenderStateManager.IASetVertexBuffers(0, 2, buffers, strides, offsets);
			RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[drawData.IndexBufferIndex]);
			RenderStateManager.DrawIndexedInstanced(drawData.IndexCount, STATIC_U32(matrices.size()), 0, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::ShadowAtlasPrePassPoint(const SRenderCommand& command)
	{
		// TODO.NR: Not needed for instanced rendering?
		ObjectBufferData.ToWorldFromObject = command.Matrices[0];
		ObjectBuffer.BindBuffer(ObjectBufferData);

		const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceData[command.U32s[0]].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans);

		RenderStateManager.VSSetShader(EVertexShaders::StaticMeshInstanced);
		RenderStateManager.PSSetShader(EPixelShaders::Null);

		for (const auto& shadowmapView : command.ShadowmapViews)
		{
			FrameBufferData.ToCameraFromWorld = shadowmapView.ShadowViewMatrix;
			FrameBufferData.ToWorldFromCamera = shadowmapView.ShadowViewMatrix.FastInverse();
			FrameBufferData.ToProjectionFromCamera = shadowmapView.ShadowProjectionMatrix;
			FrameBufferData.ToCameraFromProjection = shadowmapView.ShadowProjectionMatrix.Inverse();
			FrameBufferData.CameraPosition = shadowmapView.ShadowPosition;

			FrameBuffer.BindBuffer(FrameBufferData);
			RenderStateManager.VSSetConstantBuffer(0, FrameBuffer);

			RenderStateManager.RSSetViewports(1, &RenderStateManager.Viewports[shadowmapView.ShadowmapViewportIndex]);

			// =============

			for (U8 drawCallIndex = 0; drawCallIndex < STATIC_U8(command.DrawCallData.size()); drawCallIndex++)
			{
				const SDrawCallData& drawData = command.DrawCallData[drawCallIndex];
				const std::vector<CDataBuffer> buffers = { RenderStateManager.VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
				const U32 strides[2] = { RenderStateManager.MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
				const U32 offsets[2] = { RenderStateManager.MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
				RenderStateManager.IASetVertexBuffers(0, 2, buffers, strides, offsets);
				RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[drawData.IndexBufferIndex]);
				RenderStateManager.DrawIndexedInstanced(drawData.IndexCount, STATIC_U32(matrices.size()), 0, 0, 0);
				CRenderManager::NumberOfDrawCallsThisFrame++;
			}
		}
	}

	void CRenderManager::ShadowAtlasPrePassSpot(const SRenderCommand& command)
	{
		const auto& shadowViewData = command.ShadowmapViews[0];
		FrameBufferData.ToCameraFromWorld = shadowViewData.ShadowViewMatrix;
		FrameBufferData.ToWorldFromCamera = shadowViewData.ShadowViewMatrix.FastInverse();
		FrameBufferData.ToProjectionFromCamera = shadowViewData.ShadowProjectionMatrix;
		FrameBufferData.ToCameraFromProjection = shadowViewData.ShadowProjectionMatrix.Inverse();
		FrameBufferData.CameraPosition = shadowViewData.ShadowPosition;
		FrameBuffer.BindBuffer(FrameBufferData);

		ObjectBufferData.ToWorldFromObject = command.Matrices[0];
		ObjectBuffer.BindBuffer(ObjectBufferData);

		RenderStateManager.VSSetConstantBuffer(0, FrameBuffer);
		RenderStateManager.RSSetViewports(1, &RenderStateManager.Viewports[shadowViewData.ShadowmapViewportIndex]);

		// =============

		const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceData[command.U32s[0]].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans);

		RenderStateManager.VSSetShader(EVertexShaders::StaticMeshInstanced);
		RenderStateManager.PSSetShader(EPixelShaders::Null);

		for (U8 drawCallIndex = 0; drawCallIndex < STATIC_U8(command.DrawCallData.size()); drawCallIndex++)
		{
			const SDrawCallData& drawData = command.DrawCallData[drawCallIndex];
			const std::vector<CDataBuffer> buffers = { RenderStateManager.VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
			const U32 strides[2] = { RenderStateManager.MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
			const U32 offsets[2] = { RenderStateManager.MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
			RenderStateManager.IASetVertexBuffers(0, 2, buffers, strides, offsets);
			RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[drawData.IndexBufferIndex]);
			RenderStateManager.DrawIndexedInstanced(drawData.IndexCount, STATIC_U32(matrices.size()), 0, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::CameraDataStorage(const SRenderCommand& command)
	{
		GBuffer.SetAsActiveTarget(&IntermediateDepth, true);

		const auto& objectMatrix = command.Matrices[0];
		const auto& projectionMatrix = command.Matrices[1];
		FrameBufferData.ToWorldFromCamera = objectMatrix;
		FrameBufferData.ToCameraFromWorld = objectMatrix.FastInverse();
		FrameBufferData.ToProjectionFromCamera = projectionMatrix;
		FrameBufferData.ToCameraFromProjection = projectionMatrix.Inverse();
		FrameBufferData.CameraPosition = objectMatrix.GetTranslation4();
		FrameBuffer.BindBuffer(FrameBufferData);

		RenderStateManager.VSSetConstantBuffer(0, FrameBuffer);
		RenderStateManager.PSSetConstantBuffer(0, FrameBuffer);
		RenderStateManager.GSSetConstantBuffer(0, FrameBuffer);
	}

	void CRenderManager::GBufferDataInstanced(const SRenderCommand& command)
	{
		const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceData[command.U32s[0]].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Trans);

		RenderStateManager.VSSetShader(EVertexShaders::StaticMeshInstanced);
		RenderStateManager.PSSetShader(EPixelShaders::GBuffer);
		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		for (U8 drawCallIndex = 0; drawCallIndex < STATIC_U8(command.DrawCallData.size()); drawCallIndex++)
		{
			if (!UMath::IsWithin(command.DrawCallData[drawCallIndex].MaterialIndex, STATIC_U16(0), STATIC_U16(command.Materials.size())))
				continue;

			MaterialBufferData = SMaterialBufferData(command.Materials[command.DrawCallData[drawCallIndex].MaterialIndex]);
			std::vector<ID3D11ShaderResourceView*> resourceViewPointers;
			std::map<U32, F32> textureUIDToShaderIndex;
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoR)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoG)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoB)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoA)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalX)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalY)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalZ)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AmbientOcclusion)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Metalness)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Roughness)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Emissive)], resourceViewPointers, textureUIDToShaderIndex);
			
			for (auto const& [textureUID, shaderIndex] : textureUIDToShaderIndex)
				GEngine::GetAssetRegistry()->UnrequestAsset(textureUID, CAssetRegistry::RenderManagerRequestID);

			MaterialBuffer.BindBuffer(MaterialBufferData);

			RenderStateManager.PSSetResources(5, STATIC_U8(resourceViewPointers.size()), resourceViewPointers.data());
			RenderStateManager.PSSetConstantBuffer(8, MaterialBuffer);

			const SDrawCallData& drawData = command.DrawCallData[drawCallIndex];
			const std::vector<CDataBuffer> buffers = { RenderStateManager.VertexBuffers[drawData.VertexBufferIndex], InstancedTransformBuffer };
			const U32 strides[2] = { RenderStateManager.MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SMatrix) };
			const U32 offsets[2] = { RenderStateManager.MeshVertexOffsets[drawData.VertexOffsetIndex], 0 };
			RenderStateManager.IASetVertexBuffers(0, 2, buffers, strides, offsets);
			RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[drawData.IndexBufferIndex]);
			RenderStateManager.DrawIndexedInstanced(drawData.IndexCount, STATIC_U32(matrices.size()), 0, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::GBufferDataInstancedEditor(const SRenderCommand& command)
	{
		const std::vector<SMatrix>& matrices = RendererStaticMeshInstanceData[command.U32s[0]].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		const std::vector<SEntity>& entities = RendererStaticMeshInstanceData[command.U32s[0]].Entities;
		InstancedEntityIDBuffer.BindBuffer(entities);

		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2Entity2Trans);

		RenderStateManager.VSSetShader(EVertexShaders::StaticMeshInstancedEditor);
		RenderStateManager.PSSetShader(EPixelShaders::GBufferInstanceEditor);
		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		for (U8 drawCallIndex = 0; drawCallIndex < STATIC_U8(command.DrawCallData.size()); drawCallIndex++)
		{
			if (!UMath::IsWithin(command.DrawCallData[drawCallIndex].MaterialIndex, STATIC_U16(0), STATIC_U16(command.Materials.size())))
				continue;

			MaterialBufferData = SMaterialBufferData(command.Materials[command.DrawCallData[drawCallIndex].MaterialIndex]);
			std::vector<ID3D11ShaderResourceView*> resourceViewPointers;
			std::map<U32, F32> textureUIDToShaderIndex;
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoR)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoG)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoB)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoA)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalX)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalY)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalZ)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AmbientOcclusion)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Metalness)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Roughness)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Emissive)], resourceViewPointers, textureUIDToShaderIndex);

			for (auto const& [textureUID, shaderIndex] : textureUIDToShaderIndex)
				GEngine::GetAssetRegistry()->UnrequestAsset(textureUID, CAssetRegistry::RenderManagerRequestID);

			MaterialBuffer.BindBuffer(MaterialBufferData);

			RenderStateManager.PSSetResources(5, STATIC_U8(resourceViewPointers.size()), resourceViewPointers.data());
			RenderStateManager.PSSetConstantBuffer(8, MaterialBuffer);

			const SDrawCallData& drawData = command.DrawCallData[drawCallIndex];
			const std::vector<CDataBuffer> buffers = { RenderStateManager.VertexBuffers[drawData.VertexBufferIndex], InstancedEntityIDBuffer, InstancedTransformBuffer };
			const U32 strides[3] = { RenderStateManager.MeshVertexStrides[drawData.VertexStrideIndex], sizeof(U64), sizeof(SMatrix) };
			const U32 offsets[3] = { RenderStateManager.MeshVertexOffsets[drawData.VertexOffsetIndex], 0, 0 };
			RenderStateManager.IASetVertexBuffers(0, 3, buffers, strides, offsets);
			RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[drawData.IndexBufferIndex]);
			RenderStateManager.DrawIndexedInstanced(drawData.IndexCount, STATIC_U32(matrices.size()), 0, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::GBufferSkeletalInstanced(const SRenderCommand& command)
	{
		SkeletalAnimationDataTextureGPU.CopyFromTexture(SkeletalAnimationDataTextureCPU.GetTexture());

		const std::vector<SMatrix>& matrices = RendererSkeletalMeshInstanceData[command.U32s[0]].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		//const std::vector<SVector2<U32>>& animationData = RendererSkeletalMeshInstanceData[command.U32s[0]].AnimationData;
		//InstancedAnimationDataBuffer.BindBuffer(animationData);

		SkeletalAnimationDataTextureGPU.SetAsVSResourceOnSlot(24);
		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4AnimDataTrans);

		RenderStateManager.VSSetShader(EVertexShaders::SkeletalMeshInstanced);
		RenderStateManager.PSSetShader(EPixelShaders::GBuffer);
		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		for (U8 drawCallIndex = 0; drawCallIndex < STATIC_U8(command.DrawCallData.size()); drawCallIndex++)
		{
			if (!UMath::IsWithin(command.DrawCallData[drawCallIndex].MaterialIndex, STATIC_U16(0), STATIC_U16(command.Materials.size())))
				continue;

			MaterialBufferData = SMaterialBufferData(command.Materials[command.DrawCallData[drawCallIndex].MaterialIndex]);
			std::vector<ID3D11ShaderResourceView*> resourceViewPointers;
			std::map<U32, F32> textureUIDToShaderIndex;
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoR)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoG)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoB)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoA)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalX)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalY)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalZ)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AmbientOcclusion)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Metalness)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Roughness)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Emissive)], resourceViewPointers, textureUIDToShaderIndex);

			for (auto const& [textureUID, shaderIndex] : textureUIDToShaderIndex)
				GEngine::GetAssetRegistry()->UnrequestAsset(textureUID, CAssetRegistry::RenderManagerRequestID);

			MaterialBuffer.BindBuffer(MaterialBufferData);

			RenderStateManager.PSSetResources(5, STATIC_U8(resourceViewPointers.size()), resourceViewPointers.data());
			RenderStateManager.PSSetConstantBuffer(8, MaterialBuffer);

			const SDrawCallData& drawData = command.DrawCallData[drawCallIndex];
			const std::vector<CDataBuffer> buffers = { RenderStateManager.VertexBuffers[drawData.VertexBufferIndex], InstancedAnimationDataBuffer, InstancedTransformBuffer };
			const U32 strides[3] = { RenderStateManager.MeshVertexStrides[drawData.VertexStrideIndex], sizeof(SVector2<U32>), sizeof(SMatrix) };
			const U32 offsets[3] = { RenderStateManager.MeshVertexOffsets[drawData.VertexOffsetIndex], 0, 0 };
			RenderStateManager.IASetVertexBuffers(0, 3, buffers, strides, offsets);
			RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[drawData.IndexBufferIndex]);
			RenderStateManager.DrawIndexedInstanced(drawData.IndexCount, STATIC_U32(matrices.size()), 0, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::GBufferSkeletalInstancedEditor(const SRenderCommand& command)
	{
		//SkeletalAnimationDataTextureGPU.CopyFromTexture(SkeletalAnimationDataTextureCPU.GetTexture());

		const std::vector<SMatrix>& matrices = RendererSkeletalMeshInstanceData[command.U32s[0]].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		//const std::vector<SVector2<U32>>& animationData = RendererSkeletalMeshInstanceData[command.U32s[0]].AnimationData;
		//InstancedAnimationDataBuffer.BindBuffer(animationData);

		const std::vector<SEntity>& entities = RendererSkeletalMeshInstanceData[command.U32s[0]].Entities;
		InstancedEntityIDBuffer.BindBuffer(entities);

		const std::vector<SMatrix>& boneMatrices = RendererSkeletalMeshInstanceData[command.U32s[0]].Bones;
		BoneBuffer.BindBuffer(boneMatrices);

		//SkeletalAnimationDataTextureGPU.SetAsVSResourceOnSlot(24);
		RenderStateManager.VSSetConstantBuffer(1, ObjectBuffer);
		RenderStateManager.VSSetConstantBuffer(6, BoneBuffer);
		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2BoneID4BoneWeight4Entity2AnimDataTrans);

		RenderStateManager.VSSetShader(EVertexShaders::SkeletalMeshInstancedEditor);
		RenderStateManager.PSSetShader(EPixelShaders::GBufferInstanceEditor);
		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		for (U8 drawCallIndex = 0; drawCallIndex < STATIC_U8(command.DrawCallData.size()); drawCallIndex++)
		{
			if (!UMath::IsWithin(command.DrawCallData[drawCallIndex].MaterialIndex, STATIC_U16(0), STATIC_U16(command.Materials.size())))
				continue;

			MaterialBufferData = SMaterialBufferData(command.Materials[command.DrawCallData[drawCallIndex].MaterialIndex]);
			std::vector<ID3D11ShaderResourceView*> resourceViewPointers;
			std::map<U32, F32> textureUIDToShaderIndex;
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoR)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoG)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoB)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AlbedoA)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalX)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalY)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::NormalZ)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::AmbientOcclusion)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Metalness)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Roughness)], resourceViewPointers, textureUIDToShaderIndex);
			MapRuntimeMaterialProperty(MaterialBufferData.Properties[STATIC_U8(EMaterialProperty::Emissive)], resourceViewPointers, textureUIDToShaderIndex);

			for (auto const& [textureUID, shaderIndex] : textureUIDToShaderIndex)
				GEngine::GetAssetRegistry()->UnrequestAsset(textureUID, CAssetRegistry::RenderManagerRequestID);

			MaterialBuffer.BindBuffer(MaterialBufferData);

			RenderStateManager.PSSetResources(5, STATIC_U8(resourceViewPointers.size()), resourceViewPointers.data());
			RenderStateManager.PSSetConstantBuffer(8, MaterialBuffer);

			const SDrawCallData& drawData = command.DrawCallData[drawCallIndex];
			const std::vector<CDataBuffer> buffers = { RenderStateManager.VertexBuffers[drawData.VertexBufferIndex], InstancedEntityIDBuffer, InstancedAnimationDataBuffer, InstancedTransformBuffer };
			const U32 strides[4] = { RenderStateManager.MeshVertexStrides[drawData.VertexStrideIndex], sizeof(U64), sizeof(SVector2<U32>), sizeof(SMatrix)};
			const U32 offsets[4] = { RenderStateManager.MeshVertexOffsets[drawData.VertexOffsetIndex], 0, 0, 0 };
			RenderStateManager.IASetVertexBuffers(0, 4, buffers, strides, offsets);
			RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[drawData.IndexBufferIndex]);
			RenderStateManager.DrawIndexedInstanced(drawData.IndexCount, STATIC_U32(matrices.size()), 0, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::GBufferSpriteInstanced(const SRenderCommand& command)
	{
		// TODO.NR: Fix transparency
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::GBufferAlphaBlend);

		const auto& textureUID = command.U32s[0];
		const std::vector<SMatrix>& matrices = RendererWorldSpaceSpriteInstanceData[textureUID].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		const std::vector<SVector4>& uvRects = RendererWorldSpaceSpriteInstanceData[textureUID].UVRects;
		InstancedUVRectBuffer.BindBuffer(uvRects);

		const std::vector<SVector4>& colors = RendererWorldSpaceSpriteInstanceData[textureUID].Colors;
		InstancedColorBuffer.BindBuffer(colors);

		RenderStateManager.IASetTopology(ETopologies::PointList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::TransUVRectColor);

		RenderStateManager.VSSetShader(EVertexShaders::SpriteInstanced);
		RenderStateManager.GSSetShader(EGeometryShaders::SpriteWorldSpace);
		RenderStateManager.PSSetShader(EPixelShaders::SpriteWorldSpace);

		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		STextureAsset* spriteTexture = assetRegistry->RequestAssetData<STextureAsset>(textureUID, CAssetRegistry::RenderManagerRequestID);
		RenderStateManager.PSSetResources(0, 1, spriteTexture->RenderTexture.GetShaderResourceView());
		assetRegistry->UnrequestAsset(textureUID, CAssetRegistry::RenderManagerRequestID);

		const std::vector<CDataBuffer> buffers = { InstancedTransformBuffer, InstancedUVRectBuffer, InstancedColorBuffer };
		constexpr U32 strides[3] = { sizeof(SMatrix), sizeof(SVector4), sizeof(SVector4) };
		constexpr U32 offsets[3] = { 0, 0, 0 };
		RenderStateManager.IASetVertexBuffers(0, 3, buffers, strides, offsets);
		RenderStateManager.IASetIndexBuffer(CDataBuffer::Null);
		RenderStateManager.DrawInstanced(1, STATIC_U32(matrices.size()), 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CRenderManager::GBufferSpriteInstancedEditor(const SRenderCommand& command)
	{
		// TODO.NR: Fix transparency
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::GBufferAlphaBlend);

		const auto& textureUID = command.U32s[0];
		const std::vector<SMatrix>& matrices = RendererWorldSpaceSpriteInstanceData[textureUID].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		const std::vector<SVector4>& uvRects = RendererWorldSpaceSpriteInstanceData[textureUID].UVRects;
		InstancedUVRectBuffer.BindBuffer(uvRects);

		const std::vector<SVector4>& colors = RendererWorldSpaceSpriteInstanceData[textureUID].Colors;
		InstancedColorBuffer.BindBuffer(colors);

		const std::vector<SEntity>& entities = RendererWorldSpaceSpriteInstanceData[textureUID].Entities;
		InstancedEntityIDBuffer.BindBuffer(entities);

		RenderStateManager.IASetTopology(ETopologies::PointList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::TransUVRectColorEntity2);

		RenderStateManager.VSSetShader(EVertexShaders::SpriteInstancedEditor);
		RenderStateManager.GSSetShader(EGeometryShaders::SpriteWorldSpaceEditor);
		RenderStateManager.PSSetShader(EPixelShaders::SpriteWorldSpaceEditor);

		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		STextureAsset* spriteTexture = assetRegistry->RequestAssetData<STextureAsset>(textureUID, CAssetRegistry::RenderManagerRequestID);
		RenderStateManager.PSSetResources(0, 1, spriteTexture->RenderTexture.GetShaderResourceView());
		assetRegistry->UnrequestAsset(textureUID, CAssetRegistry::RenderManagerRequestID);

		const std::vector<CDataBuffer> buffers = { InstancedTransformBuffer, InstancedUVRectBuffer, InstancedColorBuffer, InstancedEntityIDBuffer };
		constexpr U32 strides[4] = { sizeof(SMatrix), sizeof(SVector4), sizeof(SVector4), sizeof(SEntity) };
		constexpr U32 offsets[4] = { 0, 0, 0, 0 };
		RenderStateManager.IASetVertexBuffers(0, 4, buffers, strides, offsets);
		RenderStateManager.IASetIndexBuffer(CDataBuffer::Null);
		RenderStateManager.DrawInstanced(1, STATIC_U32(matrices.size()), 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CRenderManager::DecalDepthCopy(const SRenderCommand& /*command*/)
	{
		DepthCopy.SetAsActiveTarget();
		IntermediateDepth.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::CopyDepth);
	}

	void CRenderManager::DeferredDecal(const SRenderCommand& command)
	{
		RenderStateManager.OMSetDepthStencilState(CRenderStateManager::EDepthStencilStates::OnlyRead);
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);
		GBuffer.SetAsActiveTarget(&IntermediateDepth);
		DepthCopy.SetAsPSResourceOnSlot(21);

		const auto& objectMatrix = command.Matrices[0];
		DecalBufferData.ToWorld = objectMatrix;
		DecalBufferData.ToObjectSpace = objectMatrix.Inverse();

		DecalBuffer.BindBuffer(DecalBufferData);

		RenderStateManager.VSSetConstantBuffer(1, DecalBuffer);
		RenderStateManager.PSSetConstantBuffer(1, DecalBuffer);

		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Pos3Nor3Tan3Bit3UV2);
		RenderStateManager.IASetVertexBuffer(0, RenderStateManager.VertexBuffers[0], RenderStateManager.MeshVertexStrides[0], RenderStateManager.MeshVertexOffsets[0]);
		RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[STATIC_U8(EDefaultIndexBuffers::DecalProjector)]);

		RenderStateManager.VSSetShader(EVertexShaders::Decal);
		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		const auto shouldRenderAlbedo = command.Flags[0];
		if (shouldRenderAlbedo)
		{
			STextureAsset* texture = assetRegistry->RequestAssetData<STextureAsset>(command.U32s[0], CAssetRegistry::RenderManagerRequestID);
			RenderStateManager.PSSetResources(5, 1, texture->RenderTexture.GetShaderResourceView());
			assetRegistry->UnrequestAsset(command.U32s[0], CAssetRegistry::RenderManagerRequestID);
			RenderStateManager.PSSetShader(EPixelShaders::DecalAlbedo);
			RenderStateManager.DrawIndexed(36, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		const auto shouldRenderMaterial = command.Flags[1];
		if (shouldRenderMaterial)
		{
			STextureAsset* texture = assetRegistry->RequestAssetData<STextureAsset>(command.U32s[1], CAssetRegistry::RenderManagerRequestID);
			RenderStateManager.PSSetResources(6, 1, texture->RenderTexture.GetShaderResourceView());
			assetRegistry->UnrequestAsset(command.U32s[1], CAssetRegistry::RenderManagerRequestID);
			RenderStateManager.PSSetShader(EPixelShaders::DecalMaterial);
			RenderStateManager.DrawIndexed(36, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}

		const auto shouldRenderNormal = command.Flags[2];
		if (shouldRenderNormal)
		{
			STextureAsset* texture = assetRegistry->RequestAssetData<STextureAsset>(command.U32s[2], CAssetRegistry::RenderManagerRequestID);
			RenderStateManager.PSSetResources(7, 1, texture->RenderTexture.GetShaderResourceView());
			assetRegistry->UnrequestAsset(command.U32s[2], CAssetRegistry::RenderManagerRequestID);
			RenderStateManager.PSSetShader(EPixelShaders::DecalNormal);
			RenderStateManager.DrawIndexed(36, 0, 0);
			CRenderManager::NumberOfDrawCallsThisFrame++;
		}
	}

	void CRenderManager::PreLightingPass(const SRenderCommand& /*command*/)
	{
		// === SSAO ===
		SSAOBuffer.SetAsActiveTarget();
		GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Normal, 2);
		IntermediateDepth.SetAsPSResourceOnSlot(21);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::SSAO);
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::Disable);

		SSAOBlurTexture.SetAsActiveTarget();
		SSAOBuffer.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::SSAOBlur);
		// === !SSAO ===

		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);
		RenderStateManager.PSSetSampler(1, ESamplers::DefaultBorder);

		RenderStateManager.GSSetShader(EGeometryShaders::Null);

		LitScene.SetAsActiveTarget();
		GBuffer.SetAllAsResources(1);
		IntermediateDepth.SetAsPSResourceOnSlot(21);
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
	}

	void CRenderManager::DeferredLightingDirectional(const SRenderCommand& command)
	{
		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		STextureAsset* cubemapTexture = assetRegistry->RequestAssetData<STextureAsset>(command.U32s[0], CAssetRegistry::RenderManagerRequestID);
		if (cubemapTexture == nullptr)
			return;

		// add Alpha blend PS shader

		ShadowAtlasDepth.SetAsPSResourceOnSlot(22);
		SSAOBlurTexture.SetAsPSResourceOnSlot(23);

		RenderStateManager.PSSetResources(0, 1, cubemapTexture->RenderTexture.GetShaderResourceView());
		assetRegistry->UnrequestAsset(command.U32s[0], CAssetRegistry::RenderManagerRequestID);

		// Update lightbufferdata and fill lightbuffer
		DirectionalLightBufferData.DirectionalLightDirection = command.Vectors[0];
		DirectionalLightBufferData.DirectionalLightColor = command.Colors[0].AsVector4();
		DirectionalLightBuffer.BindBuffer(DirectionalLightBufferData);
		RenderStateManager.PSSetConstantBuffer(2, DirectionalLightBuffer);

		const auto& shadowViewData = command.ShadowmapViews[0];
		ShadowmapBufferData.ToShadowmapView = shadowViewData.ShadowViewMatrix;
		ShadowmapBufferData.ToShadowmapProjection = shadowViewData.ShadowProjectionMatrix;
		ShadowmapBufferData.ShadowmapPosition = shadowViewData.ShadowPosition;

		const auto& viewport = RenderStateManager.Viewports[shadowViewData.ShadowmapViewportIndex];
		ShadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
		ShadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
		ShadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
		ShadowmapBufferData.ShadowTestTolerance = 0.001f;

		ShadowmapBuffer.BindBuffer(ShadowmapBufferData);
		RenderStateManager.PSSetConstantBuffer(5, ShadowmapBuffer);

		// Emissive Post Processing 
		EmissiveBufferData.EmissiveStrength = FullscreenRenderer.PostProcessingBufferData.EmissiveStrength;
		EmissiveBuffer.BindBuffer(EmissiveBufferData);
		RenderStateManager.PSSetConstantBuffer(7, EmissiveBuffer);

		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Null);
		RenderStateManager.IASetVertexBuffer(0, CDataBuffer::Null, 0, 0);
		RenderStateManager.IASetIndexBuffer(CDataBuffer::Null);

		RenderStateManager.VSSetShader(EVertexShaders::Fullscreen);
		RenderStateManager.PSSetShader(EPixelShaders::DeferredDirectional);

		RenderStateManager.Draw(3, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CRenderManager::DeferredLightingPoint(const SRenderCommand& command)
	{
		ShadowAtlasDepth.SetAsPSResourceOnSlot(22);
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);

		// Update lightbufferdata and fill lightbuffer
		PointLightBufferData.ToWorldFromObject = command.Matrices[0];
		PointLightBufferData.ColorAndIntensity = command.Colors[0].AsVector4();
		PointLightBufferData.ColorAndIntensity.W = command.F32s[0];
		const SVector& position = command.Matrices[0].GetTranslation();
		PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, command.F32s[1] };
		PointLightBuffer.BindBuffer(PointLightBufferData);
		RenderStateManager.VSSetConstantBuffer(3, PointLightBuffer);
		RenderStateManager.PSSetConstantBuffer(3, PointLightBuffer);

		SShadowmapBufferData shadowmapBufferData[6];
		for (U8 shadowmapViewIndex = 0; shadowmapViewIndex < 6; shadowmapViewIndex++)
		{
			shadowmapBufferData[shadowmapViewIndex].ToShadowmapView = command.ShadowmapViews[shadowmapViewIndex].ShadowViewMatrix;
			shadowmapBufferData[shadowmapViewIndex].ToShadowmapProjection = command.ShadowmapViews[shadowmapViewIndex].ShadowProjectionMatrix;
			shadowmapBufferData[shadowmapViewIndex].ShadowmapPosition = command.ShadowmapViews[shadowmapViewIndex].ShadowPosition;

			const auto& viewport = RenderStateManager.Viewports[command.ShadowmapViews[shadowmapViewIndex].ShadowmapViewportIndex];
			shadowmapBufferData[shadowmapViewIndex].ShadowmapResolution = { viewport.Width, viewport.Height };
			shadowmapBufferData[shadowmapViewIndex].ShadowAtlasResolution = ShadowAtlasResolution;
			shadowmapBufferData[shadowmapViewIndex].ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
			shadowmapBufferData[shadowmapViewIndex].ShadowTestTolerance = 0.00001f;// TODO: make a constant somewhere
		}

		ShadowmapBuffer.BindBuffer(shadowmapBufferData);
		RenderStateManager.PSSetConstantBuffer(5, ShadowmapBuffer);

		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Position4);
		RenderStateManager.IASetVertexBuffer(0, RenderStateManager.VertexBuffers[STATIC_U8(EVertexBufferPrimitives::PointLightCube)], RenderStateManager.MeshVertexStrides[1], RenderStateManager.MeshVertexOffsets[0]);
		RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[STATIC_U8(EDefaultIndexBuffers::PointLightCube)]);

		RenderStateManager.VSSetShader(EVertexShaders::PointAndSpotLight);
		RenderStateManager.PSSetShader(EPixelShaders::DeferredPoint);

		RenderStateManager.DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
	}

	void CRenderManager::DeferredLightingSpot(const SRenderCommand& command)
	{
		ShadowAtlasDepth.SetAsPSResourceOnSlot(22);
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);

		// Update lightbufferdata and fill lightbuffer
		PointLightBufferData.ToWorldFromObject = command.Matrices[0];
		PointLightBufferData.ColorAndIntensity = command.Colors[0].AsVector4();
		PointLightBufferData.ColorAndIntensity.W = command.F32s[0];
		const SVector position = command.Matrices[0].GetTranslation();
		PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, command.F32s[1] };

		PointLightBuffer.BindBuffer(PointLightBufferData);
		RenderStateManager.VSSetConstantBuffer(3, PointLightBuffer);

		SpotLightBufferData.ColorAndIntensity = PointLightBufferData.ColorAndIntensity;
		SpotLightBufferData.PositionAndRange = PointLightBufferData.PositionAndRange;
		SpotLightBufferData.Direction = command.Vectors[0];
		SpotLightBufferData.DirectionNormal1 = command.Vectors[1];
		SpotLightBufferData.DirectionNormal2 = command.Vectors[2];
		SpotLightBufferData.OuterAngle = command.F32s[2];
		SpotLightBufferData.InnerAngle = command.F32s[3];

		SpotLightBuffer.BindBuffer(SpotLightBufferData);
		RenderStateManager.PSSetConstantBuffer(3, SpotLightBuffer);

		const auto& shadowViewData = command.ShadowmapViews[0];
		SShadowmapBufferData shadowmapBufferData;
		shadowmapBufferData.ToShadowmapView = shadowViewData.ShadowViewMatrix;
		shadowmapBufferData.ToShadowmapProjection = shadowViewData.ShadowProjectionMatrix;
		shadowmapBufferData.ShadowmapPosition = shadowViewData.ShadowPosition;

		const auto& viewport = RenderStateManager.Viewports[shadowViewData.ShadowmapViewportIndex];
		shadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
		shadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
		shadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
		shadowmapBufferData.ShadowTestTolerance = 0.00001f;

		ShadowmapBuffer.BindBuffer(shadowmapBufferData);
		RenderStateManager.PSSetConstantBuffer(5, ShadowmapBuffer);

		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Position4);
		RenderStateManager.IASetVertexBuffer(0, RenderStateManager.VertexBuffers[1], RenderStateManager.MeshVertexStrides[1], RenderStateManager.MeshVertexOffsets[0]);
		RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[STATIC_U8(EDefaultIndexBuffers::PointLightCube)]);

		// Use Point Light Vertex Shader
		RenderStateManager.VSSetShader(EVertexShaders::PointAndSpotLight);
		RenderStateManager.PSSetShader(EPixelShaders::DeferredSpot);

		RenderStateManager.DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
	}

	inline void CRenderManager::Skybox(const SRenderCommand& command)
	{
		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		STextureAsset* cubemapTexture = assetRegistry->RequestAssetData<STextureAsset>(command.U32s[0], CAssetRegistry::RenderManagerRequestID);
		if (cubemapTexture == nullptr)
			return;

		ID3D11ShaderResourceView* nullView = NULL;
		RenderStateManager.PSSetResources(21, 1, &nullView);

		LitScene.SetAsActiveTarget(&IntermediateDepth);

		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::Disable);
		RenderStateManager.OMSetDepthStencilState(CRenderStateManager::EDepthStencilStates::DepthFirst);
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);

		RenderStateManager.PSSetResources(0, 1, cubemapTexture->RenderTexture.GetShaderResourceView());
		assetRegistry->UnrequestAsset(command.U32s[0], CAssetRegistry::RenderManagerRequestID);

		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Position4);
		RenderStateManager.IASetVertexBuffer(0, RenderStateManager.VertexBuffers[STATIC_U8(EVertexBufferPrimitives::SkyboxCube)], RenderStateManager.MeshVertexStrides[1], RenderStateManager.MeshVertexOffsets[0]);
		RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[STATIC_U8(EDefaultIndexBuffers::SkyboxCube)]);

		RenderStateManager.VSSetShader(EVertexShaders::Skybox);
		RenderStateManager.PSSetShader(EPixelShaders::Skybox);

		RenderStateManager.DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
		
		RenderStateManager.OMSetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
	}

	void CRenderManager::PostBaseLightingPass(const SRenderCommand& /*command*/)
	{
		RenderedScene.SetAsActiveTarget();
		LitScene.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
	}

	void CRenderManager::VolumetricLightingDirectional(const SRenderCommand& command)
	{
		VolumetricAccumulationBuffer.SetAsActiveTarget();
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::Default);
		IntermediateDepth.SetAsPSResourceOnSlot(21);
		ShadowAtlasDepth.SetAsPSResourceOnSlot(22);

		// Lightbuffer
		DirectionalLightBufferData.DirectionalLightDirection = command.Vectors[0];
		DirectionalLightBufferData.DirectionalLightColor = command.Colors[0].AsVector4();
		DirectionalLightBuffer.BindBuffer(DirectionalLightBufferData);
		RenderStateManager.PSSetConstantBuffer(1, DirectionalLightBuffer);

		// Volumetric buffer
		VolumetricLightBufferData.NumberOfSamplesReciprocal = (1.0f / command.F32s[0]);
		VolumetricLightBufferData.LightPower = command.F32s[1];
		VolumetricLightBufferData.ScatteringProbability = command.F32s[2];
		VolumetricLightBufferData.HenyeyGreensteinGValue = command.F32s[3];

		VolumetricLightBuffer.BindBuffer(VolumetricLightBufferData);
		RenderStateManager.PSSetConstantBuffer(4, VolumetricLightBuffer);

		// Shadowbuffer
		const auto& shadowViewData = command.ShadowmapViews[0];
		ShadowmapBufferData.ToShadowmapView = shadowViewData.ShadowViewMatrix;
		ShadowmapBufferData.ToShadowmapProjection = shadowViewData.ShadowProjectionMatrix;
		ShadowmapBufferData.ShadowmapPosition = shadowViewData.ShadowPosition;

		const auto& viewport = RenderStateManager.Viewports[shadowViewData.ShadowmapViewportIndex];
		ShadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
		ShadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
		ShadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
		ShadowmapBufferData.ShadowTestTolerance = 0.001f;

		ShadowmapBuffer.BindBuffer(ShadowmapBufferData);
		RenderStateManager.PSSetConstantBuffer(5, ShadowmapBuffer);

		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Null);
		RenderStateManager.IASetVertexBuffer(0, CDataBuffer::Null, 0, 0);
		RenderStateManager.IASetIndexBuffer(CDataBuffer::Null);

		RenderStateManager.VSSetShader(EVertexShaders::Fullscreen);
		RenderStateManager.PSSetShader(EPixelShaders::VolumetricDirectional);

		RenderStateManager.Draw(3, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;

		ShouldBlurVolumetricBuffer = true;
	}

	void CRenderManager::VolumetricLightingPoint(const SRenderCommand& command)
	{
		VolumetricAccumulationBuffer.SetAsActiveTarget();
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);
		IntermediateDepth.SetAsPSResourceOnSlot(21);
		ShadowAtlasDepth.SetAsPSResourceOnSlot(22);

		// Light Buffer
		PointLightBufferData.ToWorldFromObject = command.Matrices[0];
		PointLightBufferData.ColorAndIntensity = command.Colors[0].AsVector4();
		PointLightBufferData.ColorAndIntensity.W = command.F32s[0];
		const SVector& position = command.Matrices[0].GetTranslation();
		PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, command.F32s[1] };
		PointLightBuffer.BindBuffer(PointLightBufferData);
		RenderStateManager.VSSetConstantBuffer(3, PointLightBuffer);
		RenderStateManager.PSSetConstantBuffer(3, PointLightBuffer);

		// Volumetric buffer
		VolumetricLightBufferData.NumberOfSamplesReciprocal = (1.0f / command.F32s[2]);
		VolumetricLightBufferData.LightPower = command.F32s[3];
		VolumetricLightBufferData.ScatteringProbability = command.F32s[4];
		VolumetricLightBufferData.HenyeyGreensteinGValue = command.F32s[5];

		VolumetricLightBuffer.BindBuffer(VolumetricLightBufferData);
		RenderStateManager.PSSetConstantBuffer(4, VolumetricLightBuffer);

		// Shadow Buffer
		SShadowmapBufferData shadowmapBufferData[6];
		for (U8 shadowmapViewIndex = 0; shadowmapViewIndex < 6; shadowmapViewIndex++)
		{
			shadowmapBufferData[shadowmapViewIndex].ToShadowmapView = command.ShadowmapViews[shadowmapViewIndex].ShadowViewMatrix;
			shadowmapBufferData[shadowmapViewIndex].ToShadowmapProjection = command.ShadowmapViews[shadowmapViewIndex].ShadowProjectionMatrix;
			shadowmapBufferData[shadowmapViewIndex].ShadowmapPosition = command.ShadowmapViews[shadowmapViewIndex].ShadowPosition;

			const auto& viewport = RenderStateManager.Viewports[command.ShadowmapViews[shadowmapViewIndex].ShadowmapViewportIndex];
			shadowmapBufferData[shadowmapViewIndex].ShadowmapResolution = { viewport.Width, viewport.Height };
			shadowmapBufferData[shadowmapViewIndex].ShadowAtlasResolution = ShadowAtlasResolution;
			shadowmapBufferData[shadowmapViewIndex].ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
			shadowmapBufferData[shadowmapViewIndex].ShadowTestTolerance = 0.00001f;
		}

		ShadowmapBuffer.BindBuffer(shadowmapBufferData);
		RenderStateManager.PSSetConstantBuffer(5, ShadowmapBuffer);

		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Position4);
		RenderStateManager.IASetVertexBuffer(0, RenderStateManager.VertexBuffers[1], RenderStateManager.MeshVertexStrides[1], RenderStateManager.MeshVertexOffsets[0]);
		RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[STATIC_U8(EDefaultIndexBuffers::PointLightCube)]);

		RenderStateManager.VSSetShader(EVertexShaders::PointAndSpotLight);
		RenderStateManager.PSSetShader(EPixelShaders::VolumetricPoint);

		RenderStateManager.DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::Default);

		ShouldBlurVolumetricBuffer = true;
	}

	void CRenderManager::VolumetricLightingSpot(const SRenderCommand& command)
	{
		VolumetricAccumulationBuffer.SetAsActiveTarget();
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::FrontFaceCulling);
		IntermediateDepth.SetAsPSResourceOnSlot(21);
		ShadowAtlasDepth.SetAsPSResourceOnSlot(22);

		// Light Buffer
		SVector position = command.Matrices[0].GetTranslation();
		PointLightBufferData.ToWorldFromObject = command.Matrices[0];
		PointLightBufferData.ColorAndIntensity = command.Colors[0].AsVector4();
		PointLightBufferData.ColorAndIntensity.W = command.F32s[0];
		PointLightBufferData.PositionAndRange = { position.X, position.Y, position.Z, command.F32s[1] };

		PointLightBuffer.BindBuffer(PointLightBufferData);
		RenderStateManager.VSSetConstantBuffer(3, PointLightBuffer);

		SpotLightBufferData.ColorAndIntensity = PointLightBufferData.ColorAndIntensity;
		SpotLightBufferData.PositionAndRange = PointLightBufferData.PositionAndRange;
		SpotLightBufferData.Direction = command.Vectors[0];
		SpotLightBufferData.DirectionNormal1 = command.Vectors[1];
		SpotLightBufferData.DirectionNormal2 = command.Vectors[2];
		SpotLightBufferData.OuterAngle = command.F32s[2];
		SpotLightBufferData.InnerAngle = command.F32s[3];

		SpotLightBuffer.BindBuffer(SpotLightBufferData);
		RenderStateManager.PSSetConstantBuffer(3, SpotLightBuffer);

		// Volumetric buffer
		VolumetricLightBufferData.NumberOfSamplesReciprocal = (1.0f / command.F32s[4]);
		VolumetricLightBufferData.LightPower = command.F32s[5];
		VolumetricLightBufferData.ScatteringProbability = command.F32s[6];
		VolumetricLightBufferData.HenyeyGreensteinGValue = command.F32s[7];

		VolumetricLightBuffer.BindBuffer(VolumetricLightBufferData);
		RenderStateManager.PSSetConstantBuffer(4, VolumetricLightBuffer);

		// Shadow Buffer
		const auto& shadowViewData = command.ShadowmapViews[0];
		SShadowmapBufferData shadowmapBufferData;
		shadowmapBufferData.ToShadowmapView = shadowViewData.ShadowViewMatrix;
		shadowmapBufferData.ToShadowmapProjection = shadowViewData.ShadowProjectionMatrix;
		shadowmapBufferData.ShadowmapPosition = shadowViewData.ShadowPosition;

		const auto& viewport = RenderStateManager.Viewports[shadowViewData.ShadowmapViewportIndex];
		shadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
		shadowmapBufferData.ShadowAtlasResolution = ShadowAtlasResolution;
		shadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / ShadowAtlasResolution.X, viewport.TopLeftY / ShadowAtlasResolution.Y };
		shadowmapBufferData.ShadowTestTolerance = 0.00001f;

		ShadowmapBuffer.BindBuffer(shadowmapBufferData);
		RenderStateManager.PSSetConstantBuffer(5, ShadowmapBuffer);

		RenderStateManager.IASetTopology(ETopologies::TriangleList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Position4);
		RenderStateManager.IASetVertexBuffer(0, RenderStateManager.VertexBuffers[1], RenderStateManager.MeshVertexStrides[1], RenderStateManager.MeshVertexOffsets[0]);
		RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[STATIC_U8(EDefaultIndexBuffers::PointLightCube)]);

		RenderStateManager.VSSetShader(EVertexShaders::PointAndSpotLight);
		RenderStateManager.PSSetShader(EPixelShaders::VolumetricSpot);

		RenderStateManager.DrawIndexed(36, 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
		RenderStateManager.RSSetRasterizerState(CRenderStateManager::ERasterizerStates::Default);

		ShouldBlurVolumetricBuffer = true;
	}

	void CRenderManager::VolumetricBlur(const SRenderCommand& /*command*/)
	{
		if (!ShouldBlurVolumetricBuffer)
			return;

		// Downsampling and Blur
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::Disable);
		DownsampledDepth.SetAsActiveTarget();
		IntermediateDepth.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::DownsampleDepth);

		// Blur
		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralHorizontal);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::BilateralVertical);

		// Upsampling
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AdditiveBlend);
		RenderedScene.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
		DownsampledDepth.SetAsPSResourceOnSlot(1);
		IntermediateDepth.SetAsPSResourceOnSlot(2);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::DepthAwareUpsampling);
	}

	inline void CRenderManager::ForwardTransparency(const SRenderCommand& /*command*/)
	{
		//RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);
	}

	void CRenderManager::ScreenSpaceSprite(const SRenderCommand& command)
	{
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);

		const auto textureUID = command.U32s[0];
		const std::vector<SMatrix>& matrices = RendererScreenSpaceSpriteInstanceData[textureUID].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		const std::vector<SVector4>& uvRects = RendererScreenSpaceSpriteInstanceData[textureUID].UVRects;
		InstancedUVRectBuffer.BindBuffer(uvRects);

		const std::vector<SVector4>& colors = RendererScreenSpaceSpriteInstanceData[textureUID].Colors;
		InstancedColorBuffer.BindBuffer(colors);

		RenderStateManager.IASetTopology(ETopologies::PointList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::TransUVRectColor);

		RenderStateManager.VSSetShader(EVertexShaders::SpriteInstanced);
		RenderStateManager.GSSetShader(EGeometryShaders::SpriteScreenSpace);
		RenderStateManager.PSSetShader(EPixelShaders::SpriteScreenSpace);

		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		STextureAsset* spriteTexture = assetRegistry->RequestAssetData<STextureAsset>(textureUID, CAssetRegistry::RenderManagerRequestID);
		RenderStateManager.PSSetResources(0, 1, spriteTexture->RenderTexture.GetShaderResourceView());
		assetRegistry->UnrequestAsset(textureUID, CAssetRegistry::RenderManagerRequestID);

		const std::vector<CDataBuffer> buffers = { InstancedTransformBuffer, InstancedUVRectBuffer, InstancedColorBuffer };
		constexpr U32 strides[3] = { sizeof(SMatrix), sizeof(SVector4), sizeof(SVector4) };
		constexpr U32 offsets[3] = { 0, 0, 0 };
		RenderStateManager.IASetVertexBuffers(0, 3, buffers, strides, offsets);
		RenderStateManager.IASetIndexBuffer(CDataBuffer::Null);
		RenderStateManager.DrawInstanced(1, STATIC_U32(matrices.size()), 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;

		RenderStateManager.VSSetConstantBuffer(0, CDataBuffer::Null);
	}

	inline void CRenderManager::WorldSpaceSpriteEditorWidget(const SRenderCommand& command)
	{
		ID3D11RenderTargetView* renderTargets[2] = { RenderedScene.GetRenderTargetView(), GBuffer.GetEditorDataRenderTarget()};
		RenderStateManager.OMSetRenderTargets(2, renderTargets, EditorWidgetDepth.GetDepthStencilView());
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);
		RenderStateManager.OMSetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);

		const auto& textureUID = command.U32s[0];
		const std::vector<SMatrix>& matrices = RendererWorldSpaceSpriteInstanceData[textureUID].Transforms;
		InstancedTransformBuffer.BindBuffer(matrices);

		const std::vector<SVector4>& uvRects = RendererWorldSpaceSpriteInstanceData[textureUID].UVRects;
		InstancedUVRectBuffer.BindBuffer(uvRects);

		const std::vector<SVector4>& colors = RendererWorldSpaceSpriteInstanceData[textureUID].Colors;
		InstancedColorBuffer.BindBuffer(colors);

		const std::vector<SEntity>& entities = RendererWorldSpaceSpriteInstanceData[textureUID].Entities;
		InstancedEntityIDBuffer.BindBuffer(entities);

		RenderStateManager.IASetTopology(ETopologies::PointList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::TransUVRectColorEntity2);

		RenderStateManager.VSSetShader(EVertexShaders::SpriteInstancedEditor);
		RenderStateManager.GSSetShader(EGeometryShaders::SpriteWorldSpaceEditor);
		RenderStateManager.PSSetShader(EPixelShaders::SpriteWorldSpaceEditorWidget);

		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);

		// TODO.NW: We have requested the asset be loaded in the RenderSystem, so it should already be loaded here. We're using RenderManagerRequestID
		// in this scope so that we don't get sticky requests between threads
		CAssetRegistry* registry = GEngine::GetAssetRegistry();
		STextureAsset* spriteTexture = registry->RequestAssetData<STextureAsset>(textureUID, CAssetRegistry::RenderManagerRequestID);
		RenderStateManager.PSSetResources(0, 1, spriteTexture->RenderTexture.GetShaderResourceView());
		registry->UnrequestAsset(textureUID, CAssetRegistry::RenderManagerRequestID);

		const std::vector<CDataBuffer> buffers = { InstancedTransformBuffer, InstancedUVRectBuffer, InstancedColorBuffer, InstancedEntityIDBuffer };
		constexpr U32 strides[4] = { sizeof(SMatrix), sizeof(SVector4), sizeof(SVector4), sizeof(SEntity) };
		constexpr U32 offsets[4] = { 0, 0, 0, 0 };
		RenderStateManager.IASetVertexBuffers(0, 4, buffers, strides, offsets);
		RenderStateManager.IASetIndexBuffer(CDataBuffer::Null);
		RenderStateManager.DrawInstanced(1, STATIC_U32(matrices.size()), 0, 0);
		CRenderManager::NumberOfDrawCallsThisFrame++;
	}

	void CRenderManager::RenderBloom(const SRenderCommand& /*command*/)
	{
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::Disable);
		RenderStateManager.OMSetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);

		HalfSizeTexture.SetAsActiveTarget();
		RenderedScene.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		QuarterSizeTexture.SetAsActiveTarget();
		HalfSizeTexture.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		BlurTexture1.SetAsActiveTarget();
		QuarterSizeTexture.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		BlurTexture2.SetAsActiveTarget();
		BlurTexture1.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianHorizontal);

		BlurTexture1.SetAsActiveTarget();
		BlurTexture2.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianVertical);

		BlurTexture2.SetAsActiveTarget();
		BlurTexture1.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianHorizontal);

		BlurTexture1.SetAsActiveTarget();
		BlurTexture2.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GaussianVertical);

		QuarterSizeTexture.SetAsActiveTarget();
		BlurTexture1.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		HalfSizeTexture.SetAsActiveTarget();
		QuarterSizeTexture.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		VignetteTexture.SetAsActiveTarget();
		RenderedScene.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);

		RenderedScene.SetAsActiveTarget();
		VignetteTexture.SetAsPSResourceOnSlot(0);
		HalfSizeTexture.SetAsPSResourceOnSlot(1);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Bloom);
	}

	inline void CRenderManager::Tonemapping(const SRenderCommand& /*command*/)
	{
		TonemappedTexture.SetAsActiveTarget();
		RenderedScene.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Tonemap);
	}

	inline void CRenderManager::AntiAliasing(const SRenderCommand& /*command*/)
	{
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::Disable);
		RenderStateManager.OMSetDepthStencilState(CRenderStateManager::EDepthStencilStates::Default);
		
		AntiAliasedTexture.SetAsActiveTarget();
		TonemappedTexture.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::FXAA);
	}

	inline void CRenderManager::GammaCorrection(const SRenderCommand& /*command*/)
	{
		RenderedScene.SetAsActiveTarget();
		AntiAliasedTexture.SetAsPSResourceOnSlot(0);
		FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::GammaCorrection);
	}

	inline void CRenderManager::RendererDebug(const SRenderCommand& /*command*/)
	{
		//DebugShadowAtlas();
	}

	inline void CRenderManager::PreDebugShapes(const SRenderCommand& /*command*/)
	{
		RenderStateManager.OMSetDepthStencilState(CRenderStateManager::EDepthStencilStates::OnlyRead);
		RenderStateManager.OMSetBlendState(CRenderStateManager::EBlendStates::AlphaBlend);

		RenderStateManager.IASetTopology(ETopologies::LineList);
		RenderStateManager.IASetInputLayout(EInputLayoutType::Position4);
		
		RenderStateManager.GSSetShader(EGeometryShaders::Line);
		RenderStateManager.VSSetShader(EVertexShaders::Line);
		RenderStateManager.PSSetShader(EPixelShaders::Line);
	}

	inline void CRenderManager::PostTonemappingUseDepth(const SRenderCommand& /*command*/)
	{
		TonemappedTexture.SetAsActiveTarget(&IntermediateDepth);
	}

	inline void CRenderManager::PostTonemappingIgnoreDepth(const SRenderCommand& /*command*/)
	{
		TonemappedTexture.SetAsActiveTarget();
	}

	inline void CRenderManager::DebugShapes(const SRenderCommand& command)
	{
		DebugShapeObjectBufferData.ToWorldFromObject = command.Matrices[0];
		DebugShapeObjectBufferData.Color = command.Vectors[0];
		DebugShapeObjectBufferData.HalfThickness = command.F32s[0] /** 0.5f?*/;

		DebugShapeObjectBuffer.BindBuffer(DebugShapeObjectBufferData);

		RenderStateManager.IASetVertexBuffer(0, RenderStateManager.VertexBuffers[command.U8s[0]], RenderStateManager.MeshVertexStrides[1], RenderStateManager.MeshVertexOffsets[0]);
		RenderStateManager.IASetIndexBuffer(RenderStateManager.IndexBuffers[command.U8s[1]]);

		RenderStateManager.GSSetConstantBuffer(1, DebugShapeObjectBuffer);

		RenderStateManager.VSSetConstantBuffer(1, DebugShapeObjectBuffer);
		RenderStateManager.DrawIndexed(command.U16s[0], 0, 0);
		NumberOfDrawCallsThisFrame++;
	}

	void CRenderManager::DebugShadowAtlas()
	{
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = 256.0f;
		viewport.Height = 256.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		RenderStateManager.RSSetViewports(1, &viewport);
		ShadowAtlasDepth.SetAsPSResourceOnSlot(0);
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
			DepthCopy.SetAsPSResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::CopyDepth);
		}
		break;
		case Havtorn::ERenderPass::GBufferAlbedo:
		{
			RenderedScene.SetAsActiveTarget();
			GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Albedo, 0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::GBufferNormals:
		{
			RenderedScene.SetAsActiveTarget();
			GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Normal, 0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::GBufferMaterials:
		{
			RenderedScene.SetAsActiveTarget();
			GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Material, 0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::SSAO:
		{
			RenderedScene.SetAsActiveTarget();
			SSAOBlurTexture.SetAsPSResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::DeferredLighting:
		{
			RenderedScene.SetAsActiveTarget();
			LitScene.SetAsPSResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::VolumetricLighting:
		{
			RenderedScene.SetAsActiveTarget();
			VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::Bloom:
		{
			RenderedScene.SetAsActiveTarget();
			VignetteTexture.SetAsPSResourceOnSlot(0);
			HalfSizeTexture.SetAsPSResourceOnSlot(1);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Difference);
		}
		break;
		case Havtorn::ERenderPass::Tonemapping:
		{
			RenderedScene.SetAsActiveTarget();
			TonemappedTexture.SetAsPSResourceOnSlot(0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Copy);
		}
		break;
		case Havtorn::ERenderPass::Antialiasing:
		{
			RenderedScene.SetAsActiveTarget();
			AntiAliasedTexture.SetAsPSResourceOnSlot(0);
			TonemappedTexture.SetAsPSResourceOnSlot(1);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::Difference);
		}
		break;
		case Havtorn::ERenderPass::EditorData:
		{
			RenderedScene.SetAsActiveTarget();
			GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::EditorData, 0);
			FullscreenRenderer.Render(CFullscreenRenderer::EFullscreenShader::EditorData);
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

		U8 currentRunningRenderPassIndex = STATIC_U8(CurrentRunningRenderPass);
		switch (payload.Event)
		{
		case EInputActionEvent::CycleRenderPassForward:
			currentRunningRenderPassIndex = (currentRunningRenderPassIndex + 1) % STATIC_U8(ERenderPass::Count);
			break;

		case EInputActionEvent::CycleRenderPassBackward:
		{
			U8 maxIndex = STATIC_U8(ERenderPass::Count) - 1;
			currentRunningRenderPassIndex = UMath::Min(--currentRunningRenderPassIndex, maxIndex);
		}
			break;

		case EInputActionEvent::CycleRenderPassReset:
			currentRunningRenderPassIndex = STATIC_U8(ERenderPass::All);
			break;

		default:
			break;
		}

		// TODO.NR: Add debug print on screen indicating what render pass is shown
		CurrentRunningRenderPass = static_cast<ERenderPass>(currentRunningRenderPassIndex);
	}

	void CRenderManager::MapRuntimeMaterialProperty(SRuntimeGraphicsMaterialProperty& property, std::vector<ID3D11ShaderResourceView*>& runtimeArray, std::map<U32, F32>& runtimeMap)
	{
		if (property.TextureChannelIndex <= -1.0f)
			return;

		if (!runtimeMap.contains(property.TextureUID))
		{
			STextureAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<STextureAsset>(property.TextureUID, CAssetRegistry::RenderManagerRequestID);
			if (asset == nullptr)
				return;

			runtimeArray.emplace_back(asset->RenderTexture.GetShaderResource());
			runtimeMap.emplace(property.TextureUID, STATIC_F32(runtimeArray.size() - 1));
		}

		property.TextureIndex = runtimeMap[property.TextureUID];
	}

	bool SRenderCommandComparer::operator()(const SRenderCommand& a, const SRenderCommand& b) const
	{
		return 	STATIC_U16(a.Type) > STATIC_U16(b.Type);
	}
}
