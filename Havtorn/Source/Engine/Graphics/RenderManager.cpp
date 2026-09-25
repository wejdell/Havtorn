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
		ENGINE_ERROR_BOOL_MESSAGE(FullscreenRenderer.Init(rhi, this), "Failed to Init Fullscreen Renderer.");
		ENGINE_ERROR_BOOL_MESSAGE(RenderStateManager.Init(rhi), "Failed to Init Render State Manager.");

		InitRenderTextures(rhi, platformManager->GetResolution());

		InitDataBuffers();

		BindRenderFunctions();

		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassForward).AddMember(this, &CRenderManager::CycleRenderPass);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassBackward).AddMember(this, &CRenderManager::CycleRenderPass);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::CycleRenderPassReset).AddMember(this, &CRenderManager::CycleRenderPass);
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

	void CRenderManager::BindRenderFunctions()
	{
		RenderFunctions[ERenderCommandType::CameraDataStorage] =				std::bind(&CRenderManager::CameraDataStorage, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::DecalDepthCopy] =					std::bind(&CRenderManager::DecalDepthCopy, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PreLightingPass] =					std::bind(&CRenderManager::PreLightingPass, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PostBaseLightingPass] =				std::bind(&CRenderManager::PostBaseLightingPass, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::VolumetricBufferBlurPass] =			std::bind(&CRenderManager::VolumetricBlur, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::ForwardTransparency] =				std::bind(&CRenderManager::ForwardTransparency, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::Bloom] =							std::bind(&CRenderManager::RenderBloom, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::Tonemapping] =						std::bind(&CRenderManager::Tonemapping, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PreDebugShape] =					std::bind(&CRenderManager::PreDebugShapes, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PostToneMappingUseDepth] =			std::bind(&CRenderManager::PostTonemappingUseDepth, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::PostToneMappingIgnoreDepth] =		std::bind(&CRenderManager::PostTonemappingIgnoreDepth, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::AntiAliasing] =						std::bind(&CRenderManager::AntiAliasing, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::GammaCorrection] =					std::bind(&CRenderManager::GammaCorrection, this, std::placeholders::_1);
		RenderFunctions[ERenderCommandType::TextureDraw] =						std::bind(&CRenderManager::TextureDraw, this, std::placeholders::_1);
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
				if (view.RenderCommands.empty())
					continue;

				ShadowAtlasDepth.ClearDepth();
				SSAOBuffer.ClearTexture();
				view.RenderTarget.ClearTexture();

				LitScene.ClearTexture();
				IntermediateTexture.ClearTexture();
				IntermediateDepth.ClearDepth();
				VolumetricAccumulationBuffer.ClearTexture();

				GBuffer.ClearTextures(ClearColor, renderViewID == WorldMainCameraEntity.GUID);
				ShadowAtlasDepth.SetAsDepthTarget(&IntermediateTexture);

				while (!view.RenderCommands.empty())
				{
					SRenderCommand currentCommand = view.RenderCommands.top();
					RenderFunctions[currentCommand.Type](currentCommand);
					view.RenderCommands.pop();
				}
				
				CheckIsolatedRenderPass(renderViewID);
			}

			RenderGraph.Execute(this);

			// RenderTarget should be complete as that is the texture we send to the viewport
			Backbuffer.SetAsActiveTarget();
			if (RenderThreadRenderViews->contains(WorldMainCameraEntity.GUID))
			{
				RenderThreadRenderViews->at(WorldMainCameraEntity.GUID).RenderTarget.SetAsPSResourceOnSlot(0);
				RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
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

	bool CRenderManager::IsMeshShadowCastingForType(const U32 meshID, const ERenderCommandType commandType, const U64 renderViewID) const
	{
		if (!GameThreadShadowCasters.contains(renderViewID))
			return false;

		return std::ranges::find_if(GameThreadShadowCasters.at(renderViewID), 
			[meshID, commandType](const SShadowCasterCategory& category) 
			{ 
				return category.CommandType == commandType && std::ranges::find(category.MeshIDs, meshID) != category.MeshIDs.end(); 
			}) != GameThreadShadowCasters.at(renderViewID).end();
	}

	void CRenderManager::AddMeshShadowCastingForType(const U32 meshID, const ERenderCommandType commandType, const U64 renderViewID)
	{
		if (!GameThreadShadowCasters.contains(renderViewID))
			GameThreadShadowCasters.emplace(renderViewID, std::vector<SShadowCasterCategory>());

		std::vector<SShadowCasterCategory>& categoriesInView = GameThreadShadowCasters.at(renderViewID);
		auto category = std::ranges::find_if(categoriesInView, [meshID, commandType](const SShadowCasterCategory& category) { return category.CommandType == commandType; });
		if (category == categoriesInView.end())
			GameThreadShadowCasters.at(renderViewID).push_back(SShadowCasterCategory{ .CommandType = commandType, .MeshIDs = { meshID } });
		else
			category->MeshIDs.push_back(meshID);
	}

	bool Havtorn::CRenderManager::IsStaticMeshInInstancedRenderList(const U32 meshUID, const U64 renderViewID)
	{
		// TODO.NW: Maybe move this to RenderView class
		if (GameThreadRenderViews->contains(renderViewID))
			return GameThreadRenderViews->at(renderViewID).StaticMeshInstanceData.contains(meshUID);

		return false;
	}

	void CRenderManager::AddStaticMeshToInstancedRenderList(const U32 meshUID, const STransformComponent* component, const U64 renderViewID)
	{
		std::unordered_map<U32, SStaticMeshInstanceData>* renderList = nullptr;

		if (GameThreadRenderViews->contains(renderViewID))
			renderList = &GameThreadRenderViews->at(renderViewID).StaticMeshInstanceData;
		else
			return;

		if (!renderList->contains(meshUID))
			renderList->emplace(meshUID, SStaticMeshInstanceData());

		renderList->at(meshUID).Transforms.emplace_back(component->Transform.GetMatrix());
		renderList->at(meshUID).Entities.emplace_back(component->Owner);
	}

	bool CRenderManager::IsSkeletalMeshInInstancedRenderList(const U32 meshUID, const U64 renderViewID)
	{
		if (GameThreadRenderViews->contains(renderViewID))
			return GameThreadRenderViews->at(renderViewID).SkeletalMeshInstanceData.contains(meshUID);

		return false;
	}

	void CRenderManager::AddSkeletalMeshToInstancedRenderList(const U32 meshUID, const STransformComponent* transformComponent, const SSkeletalAnimationComponent* animationComponent, const U64 renderViewID)
	{
		std::unordered_map<U32, SSkeletalMeshInstanceData>* renderList = nullptr;

		if (GameThreadRenderViews->contains(renderViewID))
			renderList = &GameThreadRenderViews->at(renderViewID).SkeletalMeshInstanceData;
		else
			return;

		if (!renderList->contains(meshUID))
			renderList->emplace(meshUID, SSkeletalMeshInstanceData());

		renderList->at(meshUID).Transforms.emplace_back(transformComponent->Transform.GetMatrix());
		renderList->at(meshUID).Entities.emplace_back(transformComponent->Owner);

		if (SComponent::IsValid(animationComponent))
			renderList->at(meshUID).Bones = animationComponent->Bones;
		//else
		//	SystemSkeletalMeshInstanceData[meshName].Bones.emplace_back({});
	}

	bool CRenderManager::IsSpriteInWorldSpaceInstancedRenderList(const U32 assetReferenceUID, const U64 renderViewID)
	{
		if (GameThreadRenderViews->contains(renderViewID))
			return GameThreadRenderViews->at(renderViewID).WorldSpaceSpriteInstanceData.contains(assetReferenceUID);

		return false;
	}

	void CRenderManager::AddSpriteToWorldSpaceInstancedRenderList(const U32 assetReferenceUID, const STransformComponent* worldSpaceTransform, const SSpriteComponent* spriteComponent, const U64 renderViewID)
	{
		std::unordered_map<U32, SSpriteInstanceData>* renderList = nullptr;

		if (GameThreadRenderViews->contains(renderViewID))
			renderList = &GameThreadRenderViews->at(renderViewID).WorldSpaceSpriteInstanceData;
		else
			return;

		if (!renderList->contains(assetReferenceUID))
			renderList->emplace(assetReferenceUID, SSpriteInstanceData());

		SSpriteInstanceData& instanceData = renderList->at(assetReferenceUID);
		instanceData.Transforms.emplace_back(worldSpaceTransform->Transform.GetMatrix());
		instanceData.UVRects.emplace_back(spriteComponent->UVRect);
		instanceData.Colors.emplace_back(spriteComponent->Color.AsVector4());
		instanceData.Entities.emplace_back(spriteComponent->Owner);
	}

	void CRenderManager::AddSpriteToWorldSpaceInstancedRenderList(const U32 assetReferenceUID, const U64& entityGUID, const SMatrix& entityMatrix, const SMatrix& cameraMatrix, const U64 renderViewID)
	{
		std::unordered_map<U32, SSpriteInstanceData>* renderList = nullptr;

		if (GameThreadRenderViews->contains(renderViewID))
			renderList = &GameThreadRenderViews->at(renderViewID).WorldSpaceSpriteInstanceData;
		else
			return;

		if (!renderList->contains(assetReferenceUID))
			renderList->emplace(assetReferenceUID, SSpriteInstanceData());

		SMatrix orientedMatrix = entityMatrix;

		const SVector location = orientedMatrix.GetTranslation();
		const SVector euler = cameraMatrix.GetEuler();
		constexpr F32 distanceNormalization = 7.0f;
		constexpr F32 scaleMin = 0.15f;
		constexpr F32 scaleMax = 0.5f;
		const F32 dist = cameraMatrix.GetTranslation().Distance(location);
		const F32 eased = UMath::EaseInOutQuad(dist / distanceNormalization);
		const F32 scaling = UMath::Remap(0.0f, 1.0f, scaleMin, scaleMax, eased);
		const SVector scale = SVector(scaling, scaling, 1.0f);
		SMatrix::Recompose(location, euler, scale, orientedMatrix);

		SSpriteInstanceData& instanceData = renderList->at(assetReferenceUID);
		instanceData.Transforms.emplace_back(orientedMatrix);
		instanceData.UVRects.emplace_back(SVector4(0.0f, 0.0f, 1.0f, 1.0f));
		instanceData.Colors.emplace_back(SVector4(1.0f, 1.0f, 1.0f, 1.0f));
		instanceData.Entities.emplace_back(entityGUID);
	}

	void CRenderManager::AddSpriteToWorldSpaceInstancedRenderList(const U32 assetReferenceUID, const STransformComponent* worldSpaceTransform, const STransformComponent* cameraTransform, const U64 renderViewID)
	{
		AddSpriteToWorldSpaceInstancedRenderList(assetReferenceUID, worldSpaceTransform->Owner.GUID, worldSpaceTransform->Transform.GetMatrix(), cameraTransform->Transform.GetMatrix(), renderViewID);
	}

	bool CRenderManager::IsSpriteInScreenSpaceInstancedRenderList(const U32 assetReferenceUID, const U64 renderViewID)
	{
		if (GameThreadRenderViews->contains(renderViewID))
			return GameThreadRenderViews->at(renderViewID).ScreenSpaceSpriteInstanceData.contains(assetReferenceUID);

		return false;
	}

	void CRenderManager::AddSpriteToScreenSpaceInstancedRenderList(const U32 assetReferenceUID, const STransform2DComponent* screenSpaceTransform, const SSpriteComponent* spriteComponent, const U64 renderViewID)
	{
		std::unordered_map<U32, SSpriteInstanceData>* renderList = nullptr;

		if (GameThreadRenderViews->contains(renderViewID))
			renderList = &GameThreadRenderViews->at(renderViewID).ScreenSpaceSpriteInstanceData;
		else
			return;

		if (!renderList->contains(assetReferenceUID))
			renderList->emplace(assetReferenceUID, SSpriteInstanceData());

		SMatrix screenSpaceMatrix;
		screenSpaceMatrix.SetScale(screenSpaceTransform->Scale.X, screenSpaceTransform->Scale.Y, 1.0f);
		screenSpaceMatrix *= SMatrix::CreateRotationAroundZ(UMath::DegToRad(screenSpaceTransform->DegreesRoll));
		screenSpaceMatrix.SetTranslation({ screenSpaceTransform->Position.X, screenSpaceTransform->Position.Y, 0.0f });

		SSpriteInstanceData& instanceData = renderList->at(assetReferenceUID);
		instanceData.Transforms.emplace_back(screenSpaceMatrix);
		instanceData.UVRects.emplace_back(spriteComponent->UVRect);
		instanceData.Colors.emplace_back(spriteComponent->Color.AsVector4());
		instanceData.Entities.emplace_back(spriteComponent->Owner);
	}

	void CRenderManager::AddSpriteToScreenSpaceInstancedRenderList(const U32 assetReferenceUID, const STransform2DComponent* screenSpaceTransform, const SUIElement& uiElement, const U64 renderViewID)
	{
		if (uiElement.UVRects.size() != STATIC_U64(EUIElementState::Count))
			return;

		std::unordered_map<U32, SSpriteInstanceData>* renderList = nullptr;

		if (GameThreadRenderViews->contains(renderViewID))
			renderList = &GameThreadRenderViews->at(renderViewID).ScreenSpaceSpriteInstanceData;
		else
			return;

		if (!renderList->contains(assetReferenceUID))
			renderList->emplace(assetReferenceUID, SSpriteInstanceData());

		SMatrix screenSpaceMatrix;
		screenSpaceMatrix.SetScale(screenSpaceTransform->Scale.X * uiElement.LocalScale.X, screenSpaceTransform->Scale.Y * uiElement.LocalScale.Y, 1.0f);
		screenSpaceMatrix *= SMatrix::CreateRotationAroundZ(UMath::DegToRad(screenSpaceTransform->DegreesRoll + uiElement.LocalDegreesRoll));
		screenSpaceMatrix.SetTranslation({ screenSpaceTransform->Position.X + uiElement.LocalPosition.X, screenSpaceTransform->Position.Y + uiElement.LocalPosition.Y, 0.0f });

		SSpriteInstanceData& instanceData = renderList->at(assetReferenceUID);
		instanceData.Transforms.emplace_back(screenSpaceMatrix);
		instanceData.UVRects.emplace_back(uiElement.UVRects[STATIC_U8(uiElement.State)]);
		instanceData.Colors.emplace_back(uiElement.Color.AsVector4());
		instanceData.Entities.emplace_back(screenSpaceTransform->Owner);
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
		GameThreadShadowCasters.clear();
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

	void CRenderManager::PushRenderCommand(SRenderCommand command, const U64 renderViewID)
	{
		command.RenderViewID = renderViewID;

		if (!GameThreadRenderViews->contains(renderViewID))
			return;

		GameThreadRenderViews->at(renderViewID).RenderCommands.push(command);
	}

	void CRenderManager::AddRenderGraphPass(CHavtornStaticString<RenderDebugNameMaxSize> name, const std::function<SRenderPassResourceDeclaration()> setup, std::function<void(CRenderManager*)>&& execution)
	{
		RenderGraph.AddPass(name, setup, std::move(execution));
	}

	CRenderGraph& CRenderManager::GetRenderGraph()
	{
		return RenderGraph;
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
			renderViewPair.second.StaticMeshInstanceData.clear();
			renderViewPair.second.SkeletalMeshInstanceData.clear();
			renderViewPair.second.WorldSpaceSpriteInstanceData.clear();
			renderViewPair.second.ScreenSpaceSpriteInstanceData.clear();
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
		GameThreadRenderViews->emplace(renderViewID, SRenderView());
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

	U32 CRenderManager::GetNumberOfRenderViews() const
	{
		return STATIC_U32(GameThreadRenderViews->size());
	}

	void CRenderManager::SetRenderPass(const ERenderPass renderPass)
	{
		CurrentRunningRenderPass = renderPass;
	}

	ERenderPass CRenderManager::GetRenderPass() const
	{
		return CurrentRunningRenderPass;
	}

	void CRenderManager::SetPSOFromRenderCommandType(const ERenderCommandType commandType)
	{
		RenderStateManager.TrySetPipelineStateObject(RenderCommandToPSOIndex.at(commandType));
	}

	void CRenderManager::SetPSOFromAssetType(const EAssetType assetType)
	{
		RenderStateManager.TrySetPipelineStateObject(AssetTypeToPSOIndex.at(assetType));
	}

	void CRenderManager::RenderFullscreenPass(const EPixelShaders pixelShader, const EBlendStates blendState)
	{
		FullscreenRenderer.Render(pixelShader, blendState);
	}

	void CRenderManager::Clear(SVector4 /*clearColor*/)
	{
		//Backbuffer.ClearTexture(clearColor);
		//IntermediateDepth.ClearDepth();
	}

	void CRenderManager::InitDataBuffers()
	{
		FrameBuffer.CreateBuffer("Frame Buffer", RHI, sizeof(SFrameBufferData));
		ObjectBuffer.CreateBuffer("Object Buffer", RHI, sizeof(SObjectBufferData));
		MaterialBuffer.CreateBuffer("Material Buffer", RHI, sizeof(SMaterialBufferData));
		DebugShapeObjectBuffer.CreateBuffer("Debug Shape Object Buffer", RHI, sizeof(SDebugShapeObjectBufferData));
		DecalBuffer.CreateBuffer("Decal Buffer", RHI, sizeof(SDecalBufferData));
		SpriteBuffer.CreateBuffer("Sprite Buffer", RHI, sizeof(SSpriteBufferData));
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

	void CRenderManager::CameraDataStorage(const SRenderCommand& command)
	{
		if (!RenderThreadRenderViews->contains(command.RenderViewID))
			return;

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

	void CRenderManager::DecalDepthCopy(const SRenderCommand& /*command*/)
	{
		DepthCopy.SetAsActiveTarget();
		IntermediateDepth.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenCopyDepth, EBlendStates::Disable);
	}

	void CRenderManager::PreLightingPass(const SRenderCommand& /*command*/)
	{
		// === SSAO ===
		SSAOBuffer.SetAsActiveTarget();
		GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Normal, 2);
		IntermediateDepth.SetAsPSResourceOnSlot(21);
		RenderFullscreenPass(EPixelShaders::FullscreenSSAO, EBlendStates::Disable);

		SSAOBlurTexture.SetAsActiveTarget();
		SSAOBuffer.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenSSAOBlur, EBlendStates::Disable);

		// === !SSAO ===

		RenderStateManager.PSSetSampler(0, ESamplers::DefaultWrap);
		RenderStateManager.PSSetSampler(1, ESamplers::DefaultBorder);

		// SYNC
		LitScene.SetAsActiveTarget();
		GBuffer.SetAllAsResources(1);
		IntermediateDepth.SetAsPSResourceOnSlot(21);
	}

	void CRenderManager::PostBaseLightingPass(const SRenderCommand& command)
	{
		if (!RenderThreadRenderViews->contains(command.RenderViewID))
			return;

		RenderThreadRenderViews->at(command.RenderViewID).RenderTarget.SetAsActiveTarget();
		LitScene.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::AdditiveBlend);
	}

	void CRenderManager::VolumetricBlur(const SRenderCommand& command)
	{
		if (!ShouldBlurVolumetricBuffer)
			return;

		// Downsampling and Blur
		DownsampledDepth.SetAsActiveTarget();
		IntermediateDepth.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenDownsampleDepth, EBlendStates::Disable);

		// Blur
		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenBilateralHorizontal, EBlendStates::Disable);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenBilateralVertical, EBlendStates::Disable);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenBilateralHorizontal, EBlendStates::Disable);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenBilateralVertical, EBlendStates::Disable);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenBilateralHorizontal, EBlendStates::Disable);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenBilateralVertical, EBlendStates::Disable);

		VolumetricBlurTexture.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenBilateralHorizontal, EBlendStates::Disable);

		VolumetricAccumulationBuffer.SetAsActiveTarget();
		VolumetricBlurTexture.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenBilateralVertical, EBlendStates::Disable);

		// Upsampling
		RenderThreadRenderViews->at(command.RenderViewID).RenderTarget.SetAsActiveTarget();
		VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
		DownsampledDepth.SetAsPSResourceOnSlot(1);
		IntermediateDepth.SetAsPSResourceOnSlot(2);
		RenderFullscreenPass(EPixelShaders::FullscreenDepthAwareUpsampling, EBlendStates::AdditiveBlend);
	}

	inline void CRenderManager::ForwardTransparency(const SRenderCommand& /*command*/)
	{
		//RenderStateManager.OMSetBlendState(EBlendStates::AlphaBlend);
	}

	void CRenderManager::RenderBloom(const SRenderCommand& command)
	{
		if (!RenderThreadRenderViews->contains(command.RenderViewID))
			return;

		HalfSizeTexture.SetAsActiveTarget();
		RenderThreadRenderViews->at(command.RenderViewID).RenderTarget.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);

		QuarterSizeTexture.SetAsActiveTarget();
		HalfSizeTexture.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);

		BlurTexture1.SetAsActiveTarget();
		QuarterSizeTexture.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);

		BlurTexture2.SetAsActiveTarget();
		BlurTexture1.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenGaussianHorizontal, EBlendStates::Disable);

		BlurTexture1.SetAsActiveTarget();
		BlurTexture2.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenGaussianVertical, EBlendStates::Disable);

		BlurTexture2.SetAsActiveTarget();
		BlurTexture1.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenGaussianHorizontal, EBlendStates::Disable);

		BlurTexture1.SetAsActiveTarget();
		BlurTexture2.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenGaussianVertical, EBlendStates::Disable);

		QuarterSizeTexture.SetAsActiveTarget();
		BlurTexture1.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);

		HalfSizeTexture.SetAsActiveTarget();
		QuarterSizeTexture.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);

		VignetteTexture.SetAsActiveTarget();
		RenderThreadRenderViews->at(command.RenderViewID).RenderTarget.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);

		RenderThreadRenderViews->at(command.RenderViewID).RenderTarget.SetAsActiveTarget();
		VignetteTexture.SetAsPSResourceOnSlot(0);
		HalfSizeTexture.SetAsPSResourceOnSlot(1);
		RenderFullscreenPass(EPixelShaders::FullscreenBloom, EBlendStates::Disable);
	}

	inline void CRenderManager::Tonemapping(const SRenderCommand& command)
	{
		if (!RenderThreadRenderViews->contains(command.RenderViewID))
			return;

		TonemappedTexture.SetAsActiveTarget();
		RenderThreadRenderViews->at(command.RenderViewID).RenderTarget.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenTonemap, EBlendStates::Disable);
	}

	inline void CRenderManager::AntiAliasing(const SRenderCommand& /*command*/)
	{
		AntiAliasedTexture.SetAsActiveTarget();
		TonemappedTexture.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenFXAA, EBlendStates::Disable);
	}

	inline void CRenderManager::GammaCorrection(const SRenderCommand& command)
	{
		if (!RenderThreadRenderViews->contains(command.RenderViewID))
			return;

		RenderThreadRenderViews->at(command.RenderViewID).RenderTarget.SetAsActiveTarget();
		AntiAliasedTexture.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenGammaCorrection, EBlendStates::Disable);
	}

	inline void CRenderManager::RendererDebug(const SRenderCommand& /*command*/)
	{
		//DebugShadowAtlas();
	}

	inline void CRenderManager::PreDebugShapes(const SRenderCommand& /*command*/)
	{
		SetPSOFromRenderCommandType(ERenderCommandType::PreDebugShape);
	}

	inline void CRenderManager::PostTonemappingUseDepth(const SRenderCommand& /*command*/)
	{
		TonemappedTexture.SetAsActiveTarget(&IntermediateDepth);
	}

	inline void CRenderManager::PostTonemappingIgnoreDepth(const SRenderCommand& /*command*/)
	{
		TonemappedTexture.SetAsActiveTarget();
	}

	inline void CRenderManager::TextureDraw(const SRenderCommand& command)
	{
		command.RenderTextures[0].SetAsPSResourceOnSlot(0);
		RenderThreadRenderViews->at(command.RenderViewID).RenderTarget.SetAsActiveTarget();
		RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
	}

	void CRenderManager::DebugShadowAtlas()
	{
		CRenderViewport viewport = CRenderViewport(RHI, 0.0f, 0.0f, 256.0f, 256.0f, 0.0f, 1.0f);
		viewport.SetViewport();
		ShadowAtlasDepth.SetAsPSResourceOnSlot(0);
		RenderFullscreenPass(EPixelShaders::FullscreenCopyDepth, EBlendStates::Disable);
	}

	void CRenderManager::CheckIsolatedRenderPass(const U64 renderViewID)
	{
		if (!RenderThreadRenderViews->contains(renderViewID))
			return;

		switch (CurrentRunningRenderPass)
		{
		case Havtorn::ERenderPass::All:
			break;
		case Havtorn::ERenderPass::Depth:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			DepthCopy.SetAsPSResourceOnSlot(0);
			RenderFullscreenPass(EPixelShaders::FullscreenCopyDepth, EBlendStates::Disable);
		}
		break;
		case Havtorn::ERenderPass::GBufferAlbedo:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Albedo, 0);
			RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
		}
		break;
		case Havtorn::ERenderPass::GBufferNormals:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Normal, 0);
			RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
		}
		break;
		case Havtorn::ERenderPass::GBufferMaterials:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::Material, 0);
			RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
		}
		break;
		case Havtorn::ERenderPass::SSAO:
		{
			//RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			//SSAOBlurTexture.SetAsPSResourceOnSlot(0);
			//RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);

			AddRenderGraphPass(CHavtornStaticString<RenderDebugNameMaxSize>("SSAO Isolated")
			, [this, renderViewID]()
			{
				SRenderPassResourceDeclaration data;
				data.Inputs.emplace_back(SRenderResourceDescription{ .Name = {"SSAO Blur Texture"}, .PipelineStage = EShaderType::Pixel, .Type = ERenderResourceType::ShaderResourceView, .BindSlot = 0 }, &SSAOBlurTexture);
				data.Outputs.emplace_back(SRenderResourceDescription{ .Name = {"Main Render Target"}, .Type = ERenderResourceType::RenderTargetView }, &RenderThreadRenderViews->at(renderViewID).RenderTarget);
				return data;
			}
			, [](CRenderManager* renderManager)
			{
				renderManager->RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
			}
			);
		}
		break;
		case Havtorn::ERenderPass::DeferredLighting:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			LitScene.SetAsPSResourceOnSlot(0);
			RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
		}
		break;
		case Havtorn::ERenderPass::VolumetricLighting:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			VolumetricAccumulationBuffer.SetAsPSResourceOnSlot(0);
			RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
		}
		break;
		case Havtorn::ERenderPass::Bloom:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			VignetteTexture.SetAsPSResourceOnSlot(0);
			HalfSizeTexture.SetAsPSResourceOnSlot(1);
			RenderFullscreenPass(EPixelShaders::FullscreenDifference, EBlendStates::Disable);
		}
		break;
		case Havtorn::ERenderPass::Tonemapping:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			TonemappedTexture.SetAsPSResourceOnSlot(0);
			RenderFullscreenPass(EPixelShaders::FullscreenCopy, EBlendStates::Disable);
		}
		break;
		case Havtorn::ERenderPass::Antialiasing:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			AntiAliasedTexture.SetAsPSResourceOnSlot(0);
			TonemappedTexture.SetAsPSResourceOnSlot(1);
			RenderFullscreenPass(EPixelShaders::FullscreenDifference, EBlendStates::Disable);
		}
		break;
		case Havtorn::ERenderPass::EditorData:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::EditorData, 0);
			RenderFullscreenPass(EPixelShaders::FullscreenEditorData, EBlendStates::Disable);
		}
		break;
		case Havtorn::ERenderPass::WorldPosition:
		{
			RenderThreadRenderViews->at(renderViewID).RenderTarget.SetAsActiveTarget();
			GBuffer.SetAsPSResourceOnSlot(CGBuffer::EGBufferTextures::WorldPosition, 0);
			RenderFullscreenPass(EPixelShaders::FullscreenWorldPosition, EBlendStates::Disable);
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

	void CRenderManager::MapRuntimeMaterialProperty(SRuntimeGraphicsMaterialProperty& property, std::vector<ID3D11ShaderResourceView*>& runtimeArray, std::map<U32, F32>& runtimeMap, const std::map<U32, CStaticRenderTexture>& textureMap)
	{
		if (property.TextureChannelIndex <= -1.0f)
			return;

		if (!runtimeMap.contains(property.TextureUID))
		{
			if (!textureMap.contains(property.TextureUID))
				return;

			runtimeArray.emplace_back(textureMap.at(property.TextureUID).GetShaderResource());
			runtimeMap.emplace(property.TextureUID, STATIC_F32(runtimeArray.size() - 1));
		}

		property.TextureIndex = runtimeMap[property.TextureUID];
	}

	CRenderStateManager& CRenderManager::GetRenderStateManager()
	{
		return RenderStateManager;
	}

	bool SRenderCommandComparer::operator()(const SRenderCommand& a, const SRenderCommand& b) const
	{

		return 	STATIC_U16(a.Type) > STATIC_U16(b.Type) || (STATIC_U16(a.Type) == STATIC_U16(b.Type) && a.InternalPriority > b.InternalPriority);
	}
}
