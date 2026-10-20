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
#include "Assets/AssetFileHeader.h"

#include <algorithm>
#include <future>
#include <ranges>

#include <PlatformManager.h>
#include <RHI/RHI.h>

#include "Threading/ThreadManager.h"

#include <DirectXTex/DirectXTex.h>
#include <set>

namespace Havtorn
{
	U32 CRenderManager::NumberOfDrawCallsThisFrame = 0;

	CRenderManager::~CRenderManager()
	{
		Release(SVector2<U16>::Zero);
	}

	bool CRenderManager::Init(CRHI* rhi, CPlatformManager* platformManager)
	{
		RHI = rhi;

		// Init renderer with texture factory, create noise texture as rendertexture
		ENGINE_ERROR_BOOL_MESSAGE(RenderTextureFactory.Init(rhi), "Failed to Init Fullscreen Texture Factory.");
		ENGINE_ERROR_BOOL_MESSAGE(RenderStateManager.Init(rhi), "Failed to Init Render State Manager.");
		ENGINE_ERROR_BOOL_MESSAGE(CustomRenderer.Init(rhi, this), "Failed to Init Custom Renderer.");
		ENGINE_ERROR_BOOL_MESSAGE(DebugRenderer.Init(rhi, this), "Failed to Init Debug Renderer.");
		ENGINE_ERROR_BOOL_MESSAGE(FullscreenRenderer.Init(rhi, this), "Failed to Init Fullscreen Renderer.");
		ENGINE_ERROR_BOOL_MESSAGE(GeometryRenderer.Init(rhi, this), "Failed to Init Geometry Renderer.");
		ENGINE_ERROR_BOOL_MESSAGE(LightRenderer.Init(rhi, this), "Failed to Init Light Renderer.");
		ENGINE_ERROR_BOOL_MESSAGE(SpriteRenderer.Init(rhi, this), "Failed to Init Sprite Renderer.");
		ENGINE_ERROR_BOOL_MESSAGE(ThumbnailRenderer.Init(rhi, this), "Failed to Init Thumbnail Renderer.");
		
		InitRenderTextures(rhi, platformManager->GetResolution());
		InitDataBuffers();

		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderViewModeForward).AddMember(this, &CRenderManager::CycleRenderViewMode);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderViewModeBackward).AddMember(this, &CRenderManager::CycleRenderViewMode);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderViewModeReset).AddMember(this, &CRenderManager::CycleRenderViewMode);
		// TODO.NW: Bind to on resolution changed?

		return true;
	}

	bool CRenderManager::ReInit(CRHI* rhi, SVector2<U16> newResolution)
	{
		ENGINE_ERROR_BOOL_MESSAGE(RenderStateManager.Init(rhi), "Failed to Init Render State Manager.");
		InitRenderTextures(rhi, newResolution);

		return true;
	}

	void CRenderManager::InitRenderTextures(CRHI* rhi, SVector2<U16> windowResolution)
	{
		Backbuffer.ReleaseTexture();
		rhi->GetSwapChain()->ResizeBuffers(0, windowResolution.X, windowResolution.Y, DXGI_FORMAT_UNKNOWN, 0);

		ID3D11Texture2D* backbufferTexture = rhi->GetBackbufferTexture();
		Backbuffer = RenderTextureFactory.CreateTexture(backbufferTexture);

		CurrentWindowResolution = windowResolution;

		// TODO.NW: Release all of these as we reinit?
		for (auto& renderView : (*GameThreadRenderViews))
		{
			renderView.second.RenderTarget = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);
		}
		for (auto& renderView : (*RenderThreadRenderViews))
		{
			renderView.second.RenderTarget = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);
		}

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
		WorldPositionTexture = RenderTextureFactory.CreateTexture(windowResolution, DXGI_FORMAT_R32G32B32A32_FLOAT, true);
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

	void CRenderManager::Render()
	{
		while (CThreadManager::RunRenderThread)
		{
			std::unique_lock<std::mutex> uniqueLock(CThreadManager::RenderMutex);
			CThreadManager::RenderCondition.wait(uniqueLock, []
				{ return CThreadManager::RenderThreadStatus == ERenderThreadStatus::ReadyToRender
				|| !CThreadManager::RunRenderThread; });

			GTime::BeginTracking(ETimerCategory::GPU);

			CRenderManager::NumberOfDrawCallsThisFrame = 0;
			RenderStateManager.ResetPSOHash();

			Backbuffer.ClearTexture();

			if (WorldPlayState != EWorldPlayState::Playing)
			{
				const U32 size = (CurrentWindowResolution.X * CurrentWindowResolution.Y);

				// TODO.NW: Would be very useful to have a set of data per render view. That way we could unlock picking and dragging into the prefab tool window
				void* editorData = EditorDataTexture.MapToCPUFromGPUTexture(GBuffer.GetEditorDataTexture());
				if (editorData != nullptr)
				{
					EntityPerPixelData = std::move(editorData);
					EntityPerPixelDataSize = size;
				}

				EditorDataTexture.UnmapFromCPU();

				void* worldPositionData = WorldPositionTexture.MapToCPUFromGPUTexture(GBuffer.GetEditorWorldPositionTexture());
				if (worldPositionData != nullptr)
				{
					WorldPositionPerPixelData = std::move(worldPositionData);
					WorldPositionPerPixelDataSize = size;
				}

				WorldPositionTexture.UnmapFromCPU();
			}

			if (RendererSkeletalAnimationBoneData != nullptr)
				SkeletalAnimationDataTextureCPU.WriteToCPUTexture(RendererSkeletalAnimationBoneData, SkeletalAnimationBoneDataSize);

			EditorWidgetDepth.ClearDepth();

			for (auto& [renderViewID, view] : (*RenderThreadRenderViews))
			{
				// TODO.NW: Hopefully we can remove all of these textures and buffers
				ShadowAtlasDepth.ClearDepth();
				SSAOBuffer.ClearTexture();
				view.RenderTarget.ClearTexture();

				LitScene.ClearTexture();
				IntermediateTexture.ClearTexture();
				IntermediateDepth.ClearDepth();
				VolumetricAccumulationBuffer.ClearTexture();

				GBuffer.ClearTextures(ClearColor, renderViewID == WorldMainCameraEntity.GUID);
				ShadowAtlasDepth.SetAsDepthTarget(&IntermediateTexture);
				
				CheckRenderViewMode(renderViewID);
				view.RenderGraph.Execute(this, view);
			}

			// TODO.NW: See if this works
			// RenderTarget should be complete as that is the texture we send to the viewport
			if (RenderThreadRenderViews->contains(WorldMainCameraEntity.GUID))
			{
				Backbuffer.CopyFromTexture(RenderThreadRenderViews->at(WorldMainCameraEntity.GUID).RenderTarget.GetTexture());
			}

			GTime::EndTracking(ETimerCategory::GPU);

			CThreadManager::RenderThreadStatus = ERenderThreadStatus::PostRender;
			uniqueLock.unlock();
			CThreadManager::RenderCondition.notify_one();
		}
	}

	void CRenderManager::Release(SVector2<U16> newResolution)
	{
		Clear(ClearColor);
		GEngine::Instance->RHI->GetContext()->OMSetRenderTargets(0, 0, 0);
		GEngine::Instance->RHI->GetContext()->OMGetDepthStencilState(0, 0);
		GEngine::Instance->RHI->GetContext()->ClearState();

		// TODO.NR: Implement this properly for window resizing

		//Backbuffer.ReleaseTexture();

		Backbuffer.Release();
		IntermediateTexture.Release();
		IntermediateDepth.Release();
		EditorWidgetDepth.Release();
		ShadowAtlasDepth.Release();
		DepthCopy.Release();

		HalfSizeTexture.Release();
		QuarterSizeTexture.Release();
		BlurTexture1.Release();
		BlurTexture2.Release();
		VignetteTexture.Release();

		LitScene.Release();
		VolumetricAccumulationBuffer.Release();
		VolumetricBlurTexture.Release();
		SSAOBuffer.Release();
		SSAOBlurTexture.Release();
		DownsampledDepth.Release();
		TonemappedTexture.Release();
		AntiAliasedTexture.Release();
		EditorDataTexture.Release();
		WorldPositionTexture.Release();
		SkeletalAnimationDataTextureCPU.Release();
		SkeletalAnimationDataTextureGPU.Release();
		//TODO.AS: Is this ultra deep call really neccesary to do here? Context: We need to specifically Resize the SwapChain Buffers Right after we Release
		//the Backbuffer texture. 
		// TODO.NW: Sounds like this should be done in the Resize context, not in this function
		GEngine::Instance->RHI->GetSwapChain()->ResizeBuffers(0, newResolution.X, newResolution.Y, DXGI_FORMAT_UNKNOWN, 0);
	}

	CRenderTexture CRenderManager::CreateRenderTextureFromSource(const std::string& filePath)
	{
		return std::move(RenderTextureFactory.CreateSRVFromSource(filePath));
	}

	CRenderTexture CRenderManager::CreateRenderTextureFromAsset(const std::string& filePath, const EAssetType assetType)
	{
		return std::move(RenderTextureFactory.CreateSRVFromAsset(filePath, assetType));
	}

	CRenderTexture CRenderManager::CreateRenderTextureFromData(const SVector2<U16> size, const DXGI_FORMAT format, void* data, const U64 elementSize)
	{
		return std::move(RenderTextureFactory.CreateTextureFromData(size, format, data, elementSize));
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

	SVector4 CRenderManager::GetWorldPositionFromData(U64 dataIndex) const
	{
		if (dataIndex < WorldPositionPerPixelDataSize)
		{
			SVector4* dataCopy = reinterpret_cast<SVector4*>(WorldPositionPerPixelData);
			return dataCopy[dataIndex];
		}

		return SVector4::Zero;
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

	SRenderView* CRenderManager::GetRenderView(const U64 id)
	{
		if (GameThreadRenderViews->contains(id))
			return &GameThreadRenderViews->at(id);

		return nullptr;
	}

	SPostProcessingBufferData CRenderManager::GetPostProcessingBufferData() const
	{
		return FullscreenRenderer.GetPostProcessBuffer();
	}

	void CRenderManager::SetPostProcessingBufferData(const SPostProcessingBufferData& data)
	{
		FullscreenRenderer.SetPostProcessBuffer(data);
	}

	void CRenderManager::SyncCrossThreadResources(const CWorld* world)
	{
		SwapRenderViews();
		std::swap(SystemSkeletalAnimationBoneData, RendererSkeletalAnimationBoneData);
		SetWorldMainCameraEntity(world->GetMainCamera());
		SetWorldEditorRenderExemptEntity(world->GetEditorRenderExemptEntity());
		SetWorldPlayState(world->GetWorldPlayState());
		RenderStateManager.FlushShaderChanges();
	}

	void CRenderManager::SetWorldMainCameraEntity(const SEntity& entity)
	{
		WorldMainCameraEntity = entity;
	}

	void CRenderManager::SetWorldEditorRenderExemptEntity(const SEntity& entity)
	{
		WorldEditorRenderExemptEntity = entity;
	}

	void CRenderManager::SetWorldPlayState(EWorldPlayState playState)
	{
		WorldPlayState = playState;
	}

	CRenderTexture* CRenderManager::GetRenderTargetTexture(const U64 renderViewID) const
	{
		if (GameThreadRenderViews->contains(renderViewID))
			return &GameThreadRenderViews->at(renderViewID).RenderTarget;

		return nullptr;
	}

	CRenderResourceRegistry& CRenderManager::GetRenderResourceRegistry()
	{
		return ResourceRegistry;
	}

	void CRenderManager::SwapRenderViews()
	{
		std::vector<U64> idsToErase = {};
		for (auto& [id, view] : *RenderThreadRenderViews)
		{
			if (RenderViewCallbacks.contains(id))
			{
				RenderViewCallbacks.at(id)(view.RenderTarget);
				RenderViewCallbacks.erase(id);
				idsToErase.push_back(id);
				continue;
			}
		}

		std::swap(GameThreadRenderViews, RenderThreadRenderViews);
		for (const U64& id : idsToErase)
		{
			GameThreadRenderViews->erase(id);
			RenderThreadRenderViews->at(id).RenderTarget.Release();
			RenderThreadRenderViews->erase(id);
		}

		ClearRenderViewInstanceData();

		for (auto& [id, view] : *RenderThreadRenderViews)
			RequestRenderView(id);

		idsToErase = {};
		for (auto& [id, view] : *GameThreadRenderViews)
		{
			if (!RenderThreadRenderViews->contains(id))
				idsToErase.push_back(id);
		}

		for (const U64& id : idsToErase)
			UnrequestRenderView(id);
	}

	void CRenderManager::ClearRenderViewInstanceData()
	{
		for (auto& renderViewPair : (*GameThreadRenderViews))
		{
			renderViewPair.second.StaticMeshDrawData.clear();
			renderViewPair.second.SkeletalMeshDrawData.clear();
			renderViewPair.second.WorldSpaceSpriteDrawData.clear();
			renderViewPair.second.ScreenSpaceSpriteDrawData.clear();
		}
	}

	void CRenderManager::RequestRenderView(const U64& renderViewID, std::optional<std::function<void(CRenderTexture)>> callback)
	{
		if (GameThreadRenderViews->contains(renderViewID))
		{
			if (RenderViewCallbacks.contains(renderViewID) && callback.has_value())
			{
				RenderViewCallbacks.at(renderViewID) = callback.value();
			}
			return;
		}

		// TODO.NW: Add size/dimensions to requests
		GameThreadRenderViews->emplace(renderViewID, SRenderView(this, &RenderStateManager, GEngine::GetAssetRegistry(), &ResourceRegistry));
		GameThreadRenderViews->at(renderViewID).RenderTarget = RenderTextureFactory.CreateTexture(CurrentWindowResolution, DXGI_FORMAT_R16G16B16A16_FLOAT);

		if (callback.has_value())
		{
			if (RenderViewCallbacks.contains(renderViewID))
				RenderViewCallbacks.at(renderViewID) = callback.value();
			else
				RenderViewCallbacks.emplace(renderViewID, callback.value());
		}
	}

	void CRenderManager::UnrequestRenderView(const U64& renderViewID)
	{
		if (!GameThreadRenderViews->contains(renderViewID))
			return;

		GameThreadRenderViews->at(renderViewID).RenderTarget.Release();
		GameThreadRenderViews->erase(renderViewID);

		RenderViewCallbacks.erase(renderViewID);
	}

	const SVector2<U16>& CRenderManager::GetCurrentWindowResolution() const
	{
		return CurrentWindowResolution;
	}

	const SVector2<F32>& CRenderManager::GetShadowAtlasResolution() const
	{
		return ShadowAtlasResolution;
	}

	F32 CRenderManager::GetShadowTestTolerance() const
	{
		return ShadowTestTolerance;
	}

	U32 CRenderManager::GetNumberOfRenderViews() const
	{
		return STATIC_U32(GameThreadRenderViews->size());
	}

	void CRenderManager::SetRenderViewMode(const ERenderViewMode renderPass)
	{
		CurrentRenderViewMode = renderPass;
	}

	ERenderViewMode CRenderManager::GetRenderViewMode() const
	{
		return CurrentRenderViewMode;
	}

	void CRenderManager::Clear(SVector4 /*clearColor*/)
	{
		//Backbuffer.ClearTexture(clearColor);
		//IntermediateDepth.ClearDepth();
	}

	void CRenderManager::InitDataBuffers()
	{
		ViewDataBuffer.CreateBuffer("Frame Buffer", RHI, sizeof(SFrameBufferData));
		MaterialBuffer.CreateBuffer("Material Buffer", RHI, sizeof(SMaterialBufferData));
		DebugShapeObjectBuffer.CreateBuffer("Debug Shape Object Buffer", RHI, sizeof(SDebugShapeObjectBufferData));
		ShadowmapBuffer.CreateBuffer("Shadowmap Buffer", RHI, sizeof(SShadowmapBufferData) * 6);
		VolumetricLightBuffer.CreateBuffer("Volumetric Light Buffer", RHI, sizeof(SVolumetricLightBufferData));
		EmissiveBuffer.CreateBuffer("Emissive Buffer", RHI, sizeof(SEmissiveBufferData));
		BoneBuffer.CreateBuffer("Bone Buffer", RHI, sizeof(SBoneBufferData));

		InstancedTransformBuffer.CreateBuffer("Instanced Transform Buffer", RHI, sizeof(SMatrix) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);
		InstancedAnimationDataBuffer.CreateBuffer("Instanced Animation Data Buffer", RHI, sizeof(SVector2<U32>) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);
		InstancedEntityIDBuffer.CreateBuffer("Instanced Entity ID Buffer", RHI, sizeof(U64) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);
		InstancedUVRectBuffer.CreateBuffer("Instanced UV Rect Buffer", RHI, sizeof(SVector4) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);
		InstancedColorBuffer.CreateBuffer("Instanced Color Buffer", RHI, sizeof(SVector4) * InstancedDrawInstanceLimit, nullptr, EDataBufferType::Vertex);
	}

	void CRenderManager::DebugShadowAtlas()
	{
		// TODO.NW: Figure out how to do this, potentially in the Finalize pass
		CRenderViewport viewport = CRenderViewport(RHI, 0.0f, 0.0f, 256.0f, 256.0f, 0.0f, 1.0f);
		viewport.SetViewport();
		ShadowAtlasDepth.SetAsPSResourceOnSlot(0);
		//RenderFullscreenPass(EPixelShaders::FullscreenCopyDepth, EBlendStates::Disable);
	}

	// TODO.NW: Figure out how to implement this as a render debug data feature in the finalize pass of default render passes
	void CRenderManager::CheckRenderViewMode(const U64 /*renderViewID*/)
	{
	//	if (!RenderThreadRenderViews->contains(renderViewID))
	//		return;

	//	switch (CurrentRenderViewMode)
	//	{
	//	case Havtorn::ERenderViewMode::All:
	//		break;
	//	case Havtorn::ERenderViewMode::Depth:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		DepthCopy.SetAsPSResourceOnSlot(0);
	//		RenderFullscreenPass(EPixelShaders::FullscreenCopyDepth, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::GBufferAlbedo:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Albedo, 0);
	//		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::GBufferNormals:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Normal, 0);
	//		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::GBufferMaterials:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Material, 0);
	//		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::SSAO:
	//	{
	//		//RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		//SSAOBlurTexture.SetAsPSResourceOnSlot(0);
	//		//RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);

	//		AddRenderGraphPass(CHavtornStaticString<RenderDebugNameMaxSize>("SSAO Isolated")
	//		, [this, renderViewID]()
	//		{
	//			SRenderPassResourceDeclaration data;
	//			data.Inputs.emplace_back(SRenderResourceDescription{ .Name = {"SSAO Blur Texture"}, .PipelineStage = EShaderType::Pixel, .Type = ERenderResourceType::ShaderResourceView, .BindSlot = 0 }, &SSAOBlurTexture);
	//			data.Outputs.emplace_back(SRenderResourceDescription{ .Name = {"Main Render Target"}, .Type = ERenderResourceType::RenderTargetView }, &RenderThreadRenderViews->at(renderViewID).RenderTarget);
	//			return data;
	//		}
	//		, [](CRenderManager* renderManager)
	//		{
	//			renderManager->RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
	//		}
	//		);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::DeferredLighting:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		LitScene.SetAsPSResourceOnSlot(0);
	//		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::VolumetricLighting:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
	//		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::Bloom:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		VignetteTexture.SetAsPSResourceOnSlot(0);
	//		HalfSizeTexture.SetAsPSResourceOnSlot(1);
	//		RenderFullscreenPass(EPixelShaders::FullscreenDifference, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::Tonemapping:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		TonemappedTexture.SetAsPSResourceOnSlot(0);
	//		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::Antialiasing:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		AntiAliasedTexture.SetAsPSResourceOnSlot(0);
	//		TonemappedTexture.SetAsPSResourceOnSlot(1);
	//		RenderFullscreenPass(EPixelShaders::FullscreenDifference, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::EditorData:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::EditorData, 0);
	//		RenderFullscreenPass(EPixelShaders::FullscreenEditorData, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::WorldPosition:
	//	{
	//		RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
	//		GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::WorldPosition, 0);
	//		RenderFullscreenPass(EPixelShaders::FullscreenWorldPosition, EBlendStates::Disable);
	//	}
	//	break;
	//	case Havtorn::ERenderViewMode::Count:
	//		break;
	//	default:
	//		break;
	//	}
	}

	void CRenderManager::CycleRenderViewMode(const SInputActionPayload payload)
	{
		if (!payload.IsPressed)
			return;

		U8 currentViewModeIndex = STATIC_U8(CurrentRenderViewMode);
		switch (payload.Event)
		{
		case EInputActionEvent::CycleRenderViewModeForward:
			currentViewModeIndex = (currentViewModeIndex + 1) % STATIC_U8(ERenderViewMode::Count);
			break;

		case EInputActionEvent::CycleRenderViewModeBackward:
		{
			U8 maxIndex = STATIC_U8(ERenderViewMode::Count) - 1;
			currentViewModeIndex = UMath::Min(--currentViewModeIndex, maxIndex);
		}
			break;

		case EInputActionEvent::CycleRenderViewModeReset:
			currentViewModeIndex = STATIC_U8(ERenderViewMode::All);
			break;

		default:
			break;
		}

		// TODO.NR: Add debug print on screen indicating what render pass is shown
		CurrentRenderViewMode = static_cast<ERenderViewMode>(currentViewModeIndex);
	}

	CRenderStateManager& CRenderManager::GetRenderStateManager()
	{
		return RenderStateManager;
	}

	SRenderView::SRenderView(CRenderManager* renderManager, CRenderStateManager* stateManager, CAssetRegistry* assetRegistry, CRenderResourceRegistry* renderResourceRegistry)
		: StateManager(stateManager)
		, AssetRegistry(assetRegistry)
		, RenderResourceRegistry(renderResourceRegistry)
		, RenderGraph(renderManager)
	{
	}

	SMeshDrawData SRenderView::ConstructMeshDrawData(const SDrawCallData& assetDrawCallData, const char* renderResourceSuffix, std::vector<SGraphicsMaterialAsset*>& materialAssets, const U32 renderViewID)
	{
		SMeshDrawData meshDrawData;
		meshDrawData.PrimitiveData.VertexBufferHandle = RenderResourceRegistry->DeclareVertexBuffer<SStaticMeshVertex>((std::string("VB ") + renderResourceSuffix).c_str(), &StateManager->VertexBuffers[assetDrawCallData.VertexBufferIndex]);
		meshDrawData.PrimitiveData.IndexBufferHandle = RenderResourceRegistry->DeclareIndexBuffer((std::string("IB ") + renderResourceSuffix).c_str(), &StateManager->IndexBuffers[assetDrawCallData.IndexBufferIndex]);
		meshDrawData.PrimitiveData.IndexCount = assetDrawCallData.IndexCount;

		SGraphicsMaterialAsset* materialAsset = materialAssets[assetDrawCallData.MaterialIndex];
		std::map<U32, CStaticRenderTexture*> mappedTextures = materialAsset->Material.GetRenderTexturePointers(renderViewID); // TODO.NW: Keep a close eye on this requesting here
		std::map<U32, F32> textureUIDToRuntimeIndex;
		for (auto& [uid, texture] : mappedTextures)
		{
			textureUIDToRuntimeIndex.emplace(uid, STATIC_F32(meshDrawData.MaterialTextureHandles.size()));
			meshDrawData.MaterialTextureHandles.emplace_back(RenderResourceRegistry->DeclareRenderTexture(std::to_string(uid).c_str(), texture));
		}

		meshDrawData.MaterialBufferData = SMaterialBufferData(materialAsset->Material);
		for (SRuntimeGraphicsMaterialProperty& property : meshDrawData.MaterialBufferData.Properties)
		{
			if (property.TextureChannelIndex > -1.0f)
				property.TextureIndex = textureUIDToRuntimeIndex.at(property.TextureUID);
		}

		return meshDrawData;
	}

	void SRenderView::AddStaticMeshDrawData(const U32 renderViewID, const U32 meshAssetID, const std::vector<U32>& materialAssetIDs, const std::vector<SMatrix>& transforms, const std::vector<SEntity>& entities, const bool deferCommand)
	{
		U32 drawID = meshAssetID + std::ranges::fold_left(materialAssetIDs, 0, std::plus<U32>());
		 if (deferCommand)
			 drawID += 1000;

		if (!StaticMeshDrawData.contains(drawID))
			StaticMeshDrawData.emplace(drawID, SStaticMeshDrawData{});

		SStaticMeshDrawData& meshData = StaticMeshDrawData.at(drawID);

		meshData.Transforms = transforms;
		meshData.Entities = entities;

		// TODO.NW: Make sure render view unrequests asset data if we do it like this. Maybe in execute?

		// This might be a mistake, took lengths to keep all asset requests on the game thread before. Have to see if this is reasonable
		SStaticMeshAsset* asset = AssetRegistry->RequestAssetData<SStaticMeshAsset>(meshAssetID, renderViewID);
		std::vector<SGraphicsMaterialAsset*> materialAssets = AssetRegistry->RequestAssetData<SGraphicsMaterialAsset>(materialAssetIDs, renderViewID);

		meshData.DrawData.clear(); // Might clear this in Execute and unregister there
		meshData.DrawData.resize(asset->DrawCallData.size());

		for (U64 index = 0; index < asset->DrawCallData.size(); index++)
		{
			const char* renderResourceSuffix = (std::to_string(meshAssetID + STATIC_U32(index))).c_str();
			meshData.DrawData[index] = ConstructMeshDrawData(asset->DrawCallData[index], renderResourceSuffix, materialAssets, renderViewID);
		}
	}

	void SRenderView::AddSkeletalMeshDrawData(const U32 renderViewID, const U32 meshAssetID, const std::vector<U32>& materialAssetIDs, const std::vector<SMatrix>& transforms, const std::vector<SEntity>& entities, const std::vector<SMatrix>& bones, const bool deferCommand)
	{
		U32 drawID = meshAssetID + std::ranges::fold_left(materialAssetIDs, 0, std::plus<U32>());
		if (deferCommand)
			drawID += 1000;

		if (!SkeletalMeshDrawData.contains(drawID))
			SkeletalMeshDrawData.emplace(drawID, SSkeletalMeshDrawData{});

		SSkeletalMeshDrawData& meshData = SkeletalMeshDrawData.at(drawID);

		meshData.Transforms = transforms;
		meshData.Entities = entities;
		meshData.Bones = bones;

		SSkeletalMeshAsset* asset = AssetRegistry->RequestAssetData<SSkeletalMeshAsset>(meshAssetID, renderViewID);
		std::vector<SGraphicsMaterialAsset*> materialAssets = AssetRegistry->RequestAssetData<SGraphicsMaterialAsset>(materialAssetIDs, renderViewID);

		meshData.DrawData.clear(); // Might clear this in Execute and unregister there
		meshData.DrawData.resize(asset->DrawCallData.size());

		for (U64 index = 0; index < asset->DrawCallData.size(); index++)
		{
			const char* renderResourceSuffix = (std::to_string(meshAssetID + STATIC_U32(index))).c_str();
			meshData.DrawData[index] = ConstructMeshDrawData(asset->DrawCallData[index], renderResourceSuffix, materialAssets, renderViewID);
		}
	}
}
