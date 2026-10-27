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
		Backbuffer.Release();
		RHI->GetSwapChain()->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

		GameThreadResourceRegistry->Deallocate(ERenderResourceDeallocationStrategy::ForceClear);
		RenderThreadResourceRegistry->Deallocate(ERenderResourceDeallocationStrategy::ForceClear);
	}

	bool CRenderManager::Init(CRHI* rhi, CPlatformManager* platformManager)
	{
		RHI = rhi;
		ResourceRegistryA.RenderManager = this;
		ResourceRegistryA.RHI = RHI;
		ResourceRegistryB.RenderManager = this;
		ResourceRegistryB.RHI = RHI;

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
		
		CurrentWindowResolution = platformManager->GetResolution();
		Backbuffer = RenderTextureFactory.CreateTexture(rhi->GetBackbufferTexture());
		InitShadowmapAtlas(ShadowAtlasResolution);

		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderViewModeForward).AddMember(this, &CRenderManager::CycleRenderViewMode);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderViewModeBackward).AddMember(this, &CRenderManager::CycleRenderViewMode);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderViewModeReset).AddMember(this, &CRenderManager::CycleRenderViewMode);
		// TODO.NW: Bind to on resolution changed?

		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);
		RenderStateManager.PSSetSampler(1, ESamplers::DefaultClamp);
		RenderStateManager.PSSetSampler(2, ESamplers::DefaultBorder);

		return true;
	}

	void CRenderManager::InitShadowmapAtlas(SVector2<F32> atlasResolution)
	{
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

			// TODO.NW: Add these to a specific "shadowmap viewports" collection to avoid confusion?
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

			//if (RendererSkeletalAnimationBoneData != nullptr)
			//	SkeletalAnimationDataTextureCPU.WriteToCPUTexture(RendererSkeletalAnimationBoneData, SkeletalAnimationBoneDataSize);

			for (auto& [renderViewID, view] : (*RenderThreadRenderViews))
			{
				view.RenderResourceRegistry->ClearResources();
				view.RenderTarget.ClearTexture();

				CheckRenderViewMode(renderViewID);
				view.RenderGraph.Execute(this, view);

				if (WorldPlayState != EWorldPlayState::Playing)
					view.ExtractEditorData();
			}

			// RenderTarget should be complete as that is the texture we send to the viewport
			Backbuffer.SetAsActiveTarget();
			if (RenderThreadRenderViews->contains(WorldMainCameraEntity.GUID))
			{
				RenderThreadRenderViews->at(WorldMainCameraEntity.GUID).RenderTarget.SetAsPSResourceOnSlot(0);
				FullscreenRenderer.Render(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
			}

			GTime::EndTracking(ETimerCategory::GPU);

			CThreadManager::RenderThreadStatus = ERenderThreadStatus::PostRender;
			uniqueLock.unlock();
			CThreadManager::RenderCondition.notify_one();
		}
	}

	void CRenderManager::UpdateResolution(SVector2<U16> newResolution)
	{
		CurrentWindowResolution = newResolution;

		RenderStateManager.ClearState();
		Backbuffer.Release();
		RHI->GetSwapChain()->ResizeBuffers(0, newResolution.X, newResolution.Y, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		Backbuffer = RenderTextureFactory.CreateTexture(RHI->GetBackbufferTexture());

		for (auto& [id, renderView] : (*GameThreadRenderViews))
		{
			renderView.ReleaseViewResources();
			renderView.ViewSize = CurrentWindowResolution;
			renderView.RenderGraph = CRenderGraph();
			renderView.RenderGraph.ResourceRegistry = GameThreadResourceRegistry;
			renderView.RenderTarget = RenderTextureFactory.CreateTexture(renderView.ViewSize, DXGI_FORMAT_R16G16B16A16_FLOAT);
			renderView.EditorDataTexture = RenderTextureFactory.CreateTexture(renderView.ViewSize, DXGI_FORMAT_R32G32_UINT, true);
			renderView.WorldPositionTexture = RenderTextureFactory.CreateTexture(renderView.ViewSize, DXGI_FORMAT_R32G32B32A32_FLOAT, true);
		}

		for (auto& [id, renderView] : (*RenderThreadRenderViews))
		{
			renderView.ReleaseViewResources();
			renderView.ViewSize = CurrentWindowResolution;
			renderView.RenderGraph = CRenderGraph();
			renderView.RenderGraph.ResourceRegistry = GameThreadResourceRegistry;
			renderView.RenderTarget = RenderTextureFactory.CreateTexture(renderView.ViewSize, DXGI_FORMAT_R16G16B16A16_FLOAT);
			renderView.EditorDataTexture = RenderTextureFactory.CreateTexture(renderView.ViewSize, DXGI_FORMAT_R32G32_UINT, true);
			renderView.WorldPositionTexture = RenderTextureFactory.CreateTexture(renderView.ViewSize, DXGI_FORMAT_R32G32B32A32_FLOAT, true);
		}

		GameThreadResourceRegistry->Deallocate(ERenderResourceDeallocationStrategy::ForceClearTexturesOnly);
		RenderThreadResourceRegistry->Deallocate(ERenderResourceDeallocationStrategy::ForceClearTexturesOnly);
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

	U64 CRenderManager::GetEntityGUIDFromData(const U64 renderViewID, const U64 dataIndex) const
	{
		if (!GameThreadRenderViews->contains(renderViewID))
			return 0;
			
		const SRenderView& renderView = GameThreadRenderViews->at(renderViewID);

		if (dataIndex < renderView.EntityPerPixelDataSize)
		{
			U64* dataCopy = reinterpret_cast<U64*>(renderView.EntityPerPixelData);
			return dataCopy[dataIndex];
		}

		return 0;
	}

	SVector4 CRenderManager::GetWorldPositionFromData(const U64 renderViewID, const U64 dataIndex) const
	{
		if (!GameThreadRenderViews->contains(renderViewID))
			return SVector4::Zero;

		const SRenderView& renderView = GameThreadRenderViews->at(renderViewID);

		if (dataIndex < renderView.WorldPositionPerPixelDataSize)
		{
			SVector4* dataCopy = reinterpret_cast<SVector4*>(renderView.WorldPositionPerPixelData);
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

	bool CRenderManager::HasRenderView(const U64 id)
	{
		return GameThreadRenderViews->contains(id);
	}

	std::map<U64, SRenderView>* CRenderManager::GetRenderViews()
	{
		return GameThreadRenderViews;
	}

	SPostProcessingBufferData CRenderManager::GetPostProcessingBufferData() const
	{
		return FullscreenRenderer.GetPostProcessBuffer();
	}

	void CRenderManager::SetPostProcessingBufferData(const SPostProcessingBufferData& data)
	{
		FullscreenRenderer.SetPostProcessBuffer(data);
	}

	void CRenderManager::SyncCrossThreadResources(const CWorld* world, const SVector2<U16>& windowResizeTarget)
	{
		SwapRenderViews(windowResizeTarget);
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
		return *GameThreadResourceRegistry;
	}

	CRenderTextureFactory& CRenderManager::GetRenderTextureFactory()
	{
		return RenderTextureFactory;
	}

	CRenderStateManager& CRenderManager::GetRenderStateManager()
	{
		return RenderStateManager;
	}

	void CRenderManager::SwapRenderViews(const SVector2<U16>& windowResizeTarget)
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

		if (windowResizeTarget.LengthSquared() > 0)
			UpdateResolution(windowResizeTarget);
		
		RenderThreadResourceRegistry->Deallocate(ERenderResourceDeallocationStrategy::CheckLifetime);

		std::swap(GameThreadRenderViews, RenderThreadRenderViews);
		std::swap(GameThreadResourceRegistry, RenderThreadResourceRegistry);
		for (const U64& id : idsToErase)
		{
			GameThreadRenderViews->erase(id);
			RenderThreadRenderViews->erase(id);
		}

		for (auto& [id, view] : *RenderThreadRenderViews)
		{
			view.RenderResourceRegistry = RenderThreadResourceRegistry;
			view.RenderGraph.ResourceRegistry = RenderThreadResourceRegistry;
			RequestRenderView(id);
		}

		idsToErase = {};
		for (auto& [id, view] : *GameThreadRenderViews)
		{
			view.DrawData = SRenderViewDrawData();
			view.RenderResourceRegistry = GameThreadResourceRegistry;
			view.RenderGraph.ResourceRegistry = GameThreadResourceRegistry;
			if (!RenderThreadRenderViews->contains(id))
				idsToErase.push_back(id);
		}

		for (const U64& id : idsToErase)
			UnrequestRenderView(id);
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
		// TODO.NW: Find a way to keep just one registry for a View and RenderGraph within it
		GameThreadRenderViews->emplace(renderViewID, SRenderView(&RenderStateManager, GEngine::GetAssetRegistry(), GameThreadResourceRegistry));
		// NW: A move constructor was attempted here, but I gave up as it seemed too volatile to move render textures around (from being constructed inline in the emplace
		// to then be moved into the map).
		SRenderView& view = GameThreadRenderViews->at(renderViewID);
		view.ViewSize = CurrentWindowResolution;
		view.RenderGraph.ResourceRegistry = GameThreadResourceRegistry;
		view.RenderTarget = RenderTextureFactory.CreateTexture(view.ViewSize, DXGI_FORMAT_R16G16B16A16_FLOAT);
		view.EditorDataTexture = RenderTextureFactory.CreateTexture(view.ViewSize, DXGI_FORMAT_R32G32_UINT, true);
		// TODO.NW: Try 16bit for world position?
		view.WorldPositionTexture = RenderTextureFactory.CreateTexture(view.ViewSize, DXGI_FORMAT_R32G32B32A32_FLOAT, true);

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

		GameThreadResourceRegistry->RemoveRenderViewRenderTarget(&GameThreadRenderViews->at(renderViewID));
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

	void CRenderManager::DebugLiveRenderObjects() const
	{
		RHI->DebugLiveObjects();
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

	SRenderView::SRenderView(CRenderStateManager* stateManager, CAssetRegistry* assetRegistry, CRenderResourceRegistry* renderResourceRegistry)
		: StateManager(stateManager)
		, AssetRegistry(assetRegistry)
		, RenderResourceRegistry(renderResourceRegistry)
	{
	}

	SRenderView::~SRenderView()
	{
		ReleaseViewResources();
	}

	void SRenderView::ReleaseViewResources()
	{
		RenderTarget.Release();
		EditorDataTexture.Release();
		WorldPositionTexture.Release();
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

		if (!DrawData.StaticMeshDrawData.contains(drawID))
			DrawData.StaticMeshDrawData.emplace(drawID, SStaticMeshDrawData{});

		SStaticMeshDrawData& meshData = DrawData.StaticMeshDrawData.at(drawID);

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
			const char* renderResourceSuffix = std::to_string(meshAssetID + STATIC_U32(index)).c_str();
			meshData.DrawData[index] = ConstructMeshDrawData(asset->DrawCallData[index], renderResourceSuffix, materialAssets, renderViewID);
		}
	}

	void SRenderView::AddSkeletalMeshDrawData(const U32 renderViewID, const U32 meshAssetID, const std::vector<U32>& materialAssetIDs, const std::vector<SMatrix>& transforms, const std::vector<SEntity>& entities, const std::vector<SMatrix>& bones, const bool deferCommand)
	{
		U32 drawID = meshAssetID + std::ranges::fold_left(materialAssetIDs, 0, std::plus<U32>());
		if (deferCommand)
			drawID += 1000;

		if (!DrawData.SkeletalMeshDrawData.contains(drawID))
			DrawData.SkeletalMeshDrawData.emplace(drawID, SSkeletalMeshDrawData{});

		SSkeletalMeshDrawData& meshData = DrawData.SkeletalMeshDrawData.at(drawID);

		meshData.Transforms = transforms;
		meshData.Entities = entities;
		meshData.Bones = bones;

		SSkeletalMeshAsset* asset = AssetRegistry->RequestAssetData<SSkeletalMeshAsset>(meshAssetID, renderViewID);
		std::vector<SGraphicsMaterialAsset*> materialAssets = AssetRegistry->RequestAssetData<SGraphicsMaterialAsset>(materialAssetIDs, renderViewID);

		meshData.DrawData.clear(); // Might clear this in Execute and unregister there
		meshData.DrawData.resize(asset->DrawCallData.size());

		for (U64 index = 0; index < asset->DrawCallData.size(); index++)
		{
			const char* renderResourceSuffix = std::to_string(meshAssetID + STATIC_U32(index)).c_str();
			meshData.DrawData[index] = ConstructMeshDrawData(asset->DrawCallData[index], renderResourceSuffix, materialAssets, renderViewID);
		}
	}

	void SRenderView::ExtractEditorData()
	{
		const U32 size = (ViewSize.X * ViewSize.Y);
		
		const SRenderResourceHandle editorDataTextureHandle = RenderResourceRegistry->DeclareRenderTexture("GBuffer Editor Data", ERenderResourceFormat::R32G32_UnsignedInt, this);
		const SRenderResource* editorDataRenderResource = RenderResourceRegistry->GetResource(editorDataTextureHandle);
		if (editorDataRenderResource == nullptr || !editorDataRenderResource->Description.IsAllocated)
			return;

		void* editorData = EditorDataTexture.MapToCPUFromGPUTexture(editorDataRenderResource->RenderTexture->GetTexture());
		if (editorData != nullptr)
		{
			EntityPerPixelData = std::move(editorData);
			EntityPerPixelDataSize = size;
		}
		EditorDataTexture.UnmapFromCPU();

		const SRenderResourceHandle worldPositionTextureHandle = RenderResourceRegistry->DeclareRenderTexture("GBuffer World Position", ERenderResourceFormat::R32G32B32A32_Float, this);
		const SRenderResource* worldPositionRenderResource = RenderResourceRegistry->GetResource(worldPositionTextureHandle);
		if (worldPositionRenderResource == nullptr || !worldPositionRenderResource->Description.IsAllocated)
			return;

		void* worldPositionData = WorldPositionTexture.MapToCPUFromGPUTexture(worldPositionRenderResource->RenderTexture->GetTexture());
		if (worldPositionData != nullptr)
		{
			WorldPositionPerPixelData = std::move(worldPositionData);
			WorldPositionPerPixelDataSize = size;
		}
		WorldPositionTexture.UnmapFromCPU();
	}
}
