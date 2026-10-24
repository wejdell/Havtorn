// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "EditorRenderSystem.h"

#include "EditorManager.h"

#include "ECS/ECSInclude.h"
#include "ECS/ComponentAlgo.h"
#include "ECS/Systems/RenderSystem.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/RenderResourceRegistry.h"
#include "Assets/AssetRegistry.h"
#include "Scene/Scene.h"

#include <MathTypes/Frustum.h>

#include <hvpch.h>

namespace Havtorn
{
	// TODO.NW: Read from config?
	static const SAssetReference CameraWidgetReference = SAssetReference("Resources/Assets/CameraIcon.hva");
	static const SAssetReference ColliderWidgetReference = SAssetReference("Resources/Assets/ColliderIcon.hva");
	static const SAssetReference DecalWidgetReference = SAssetReference("Resources/Assets/DecalIcon.hva");
	static const SAssetReference DirectionalLightWidgetReference = SAssetReference("Resources/Assets/DirectionalLightIcon.hva");
	static const SAssetReference EnvironmentLightWidgetReference = SAssetReference("Resources/Assets/EnvironmentLightIcon.hva");
	static const SAssetReference PointLightWidgetReference = SAssetReference("Resources/Assets/PointLightIcon.hva");
	static const SAssetReference PrefabWidgetReference = SAssetReference("Resources/Assets/PrefabWidgetIcon.hva");
	static const SAssetReference SpotlightWidgetReference = SAssetReference("Resources/Assets/SpotlightIcon.hva");

	CEditorRenderSystem::CEditorRenderSystem(CRenderManager* renderManager, CWorld* world, CEditorManager* editorManager)
		: ISystem()
		, RenderManager(renderManager)
		, Manager(editorManager)
		, World(world)
	{
	}

	void CEditorRenderSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		// NW: There is only one editor data texture set currently, for the main camera. 
		const SEntity mainCamera = World->GetMainCamera();
		const SCameraData mainCameraData = UComponentAlgo::GetCameraData(mainCamera, scenes);

		// TODO: consider color coding or fading out widgets in scenes other than CurrentWorkingScene // Aki
		if (!mainCameraData.IsValid())
			return;

		if (World->GetWorldPlayState() == EWorldPlayState::Playing)
			return;

		const SMatrix viewMatrix = mainCameraData.TransformComponent->Transform.GetMatrix();
		const SMatrix projectionMatrix = mainCameraData.CameraComponent->ProjectionMatrix;
		SRenderView* renderView = RenderManager->GetRenderView(mainCamera.GUID);

		for (Ptr<CScene>& scene : scenes)
		{
			ProcessRenderView(renderView, scene.get(), { viewMatrix, projectionMatrix }, true);
		}

		struct SEditorWidgetPassData : public SRenderPassParams
		{
			SRenderResourceHandle ViewDataBufferHandle;
			SRenderResourceHandle TransformBufferHandle;
			SRenderResourceHandle UVRectBufferHandle;
			SRenderResourceHandle ColorBufferHandle;
			SRenderResourceHandle EntityBufferHandle;

			SRenderResourceHandle RenderTargetHandle;
			SRenderResourceHandle GBufferWorldPositionTargetHandle;
			SRenderResourceHandle GBufferEditorDataTargetHandle;
			SRenderResourceHandle DepthTargetHandle;
		};
		renderView->RenderGraph.AddPass<SEditorWidgetPassData>("EditorWidgetPass",
			[](CRenderResourceRegistry& registry)
			{
				SEditorWidgetPassData passData;
				passData.SetIsRootPass();
				passData.AddDependency<STonemappingPassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.TransformBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.UVRectBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SVector4>("Instanced UV Rect Buffer", InstancedDrawInstanceLimit));
				passData.ColorBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SVector4>("Instanced Color Buffer", InstancedDrawInstanceLimit));
				passData.EntityBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SEntity>("Instanced Entity Buffer", InstancedDrawInstanceLimit));

				passData.RenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float));
				passData.GBufferWorldPositionTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer World Position", ERenderResourceFormat::R32G32B32A32_Float));
				passData.GBufferEditorDataTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Editor Data", ERenderResourceFormat::R32G32_UnsignedInt));

				// NW: This was a separate depth called EditorWidgetDepth before, does it need to be clear for some reason?
				passData.DepthTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless));

				return passData;
			},
			[](const SEditorWidgetPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData constantRenderData;
				constantRenderData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				constantRenderData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.RenderTargetHandle);
				targetData.DepthTexture = registry.GetResource(data.DepthTargetHandle);

				SEditorRenderData editorData;
				editorData.WorldPositionRenderTarget = registry.GetResource(data.GBufferWorldPositionTargetHandle);
				editorData.EditorDataRenderTarget = registry.GetResource(data.GBufferEditorDataTargetHandle);
				editorData.EntityBuffer = registry.GetResource(data.EntityBufferHandle);

				renderManager->SpriteRenderer.SetupWidgetRender(constantRenderData, targetData, editorData);

				{
					SSpriteRenderData rendererData;
					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.UVRectBuffer = registry.GetResource(data.UVRectBufferHandle);
					rendererData.ColorBuffer = registry.GetResource(data.ColorBufferHandle);

					for (auto& [spriteID, drawData] : renderView.WidgetSpriteDrawData)
					{
						rendererData.TransformBuffer->DataBuffer->BindBuffer(drawData.Transforms);
						rendererData.UVRectBuffer->DataBuffer->BindBuffer(drawData.UVRects);
						rendererData.ColorBuffer->DataBuffer->BindBuffer(drawData.Colors);
						rendererData.InstanceCount = STATIC_U32(drawData.Transforms.size());

						rendererData.SpriteTexture = registry.GetResource(drawData.SpriteTextureHandle);

						editorData.EntityBuffer->DataBuffer->BindBuffer(drawData.Entities);

						renderManager->SpriteRenderer.RenderEditorWorldSpaceWidget(rendererData, editorData);
					}
				}
			}
		);
	}

	void CEditorRenderSystem::ProcessRenderView(SRenderView* renderView, CScene* scene, const SViewData& viewData, const bool doCulling)
	{
		if (!scene->OnEntityPreDestroy.IsBoundTo(Handle))
		{
			Handle = scene->OnEntityPreDestroy.AddMember(this, &CEditorRenderSystem::OnEntityPreDestroy);
		}

		const SFrustum viewFrustum = doCulling ? SFrustum(viewData.ViewMatrix, viewData.ProjectionMatrix) : SFrustum(SMatrix::Identity, SMatrix::Identity);

		auto tryAddComponentWidgets = [&]<typename T>(T&& /*emptyComponent*/, const SAssetReference& assetReference, std::function<bool(const T*)> internalEvaluator = {})
		{
			for (const T* component : scene->GetComponents<T>())
			{
				if (!SComponent::IsValid(component))
					continue;

				const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(component);
				if (!SComponent::IsValid(transformComp))
					continue;

				STextureAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<STextureAsset>(assetReference, component->Owner.GUID);
				if (asset == nullptr)
					continue;

				if (internalEvaluator)
				{
					if (!internalEvaluator(component))
						continue;
				}

				SMatrix orientedMatrix = transformComp->Transform.GetMatrix();

				const SVector location = orientedMatrix.GetTranslation();
				const SVector euler = viewData.ViewMatrix.GetEuler();
				constexpr F32 distanceNormalization = 7.0f;
				constexpr F32 scaleMin = 0.15f;
				constexpr F32 scaleMax = 0.5f;
				const F32 dist = viewData.ViewMatrix.GetTranslation().Distance(location);
				const F32 eased = UMath::EaseInOutQuad(dist / distanceNormalization);
				const F32 scaling = UMath::Remap(0.0f, 1.0f, scaleMin, scaleMax, eased);
				const SVector scale = SVector(scaling, scaling, 1.0f);
				SMatrix::Recompose(location, euler, scale, orientedMatrix);

				if (doCulling)
				{
					// NW: These are assumed correct from the form of GeometryPrimitives::SpriteQuad
					const SVector quadObjectMin = SVector(-0.5f, -0.5f, 0.0f);
					const SVector quadObjectMax = SVector(0.5f, 0.5f, 0.0f);
					const SVector quadObjectCenter = SVector::Zero;

					// NW: This seems to give the right results, notice the Hadamard multiplication between bounds vector and scale
					const F32 radius = ((quadObjectMax - quadObjectMin) * orientedMatrix.GetScale()).Size();
					const SSphere quadBoundingSphere = SSphere((SVector4(quadObjectCenter, 1.0f) * orientedMatrix).ToVector3(), radius);

					if (!viewFrustum.Intersects(quadBoundingSphere))
						continue;
				}

				const U32 drawID = assetReference.UID;
				if (!renderView->WidgetSpriteDrawData.contains(drawID))
				{
					// TODO.NW: Need unique names for all external assets that are declared, maybe if they are external they
					// can just use the asset ref ID as resource ID?
					SRenderResourceHandle spriteTextureHandle = RenderManager->GetRenderResourceRegistry().DeclareRenderTexture(asset->Name.c_str(), &asset->RenderTexture);
					renderView->WidgetSpriteDrawData.emplace(drawID, SSpriteDrawData{ .SpriteTextureHandle = spriteTextureHandle });
				}

				SSpriteDrawData& spriteDrawData = renderView->WidgetSpriteDrawData.at(drawID);
				spriteDrawData.Transforms.push_back(orientedMatrix);
				spriteDrawData.UVRects.emplace_back(0.0f, 0.0f, 1.0f, 1.0f);
				spriteDrawData.Colors.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
				spriteDrawData.Entities.push_back(component->Owner);
			}
		};

		tryAddComponentWidgets(SCameraComponent(), CameraWidgetReference);
		tryAddComponentWidgets(SDecalComponent(), DecalWidgetReference);
		tryAddComponentWidgets(SEnvironmentLightComponent(), EnvironmentLightWidgetReference);
		tryAddComponentWidgets(SDirectionalLightComponent(), DirectionalLightWidgetReference);
		tryAddComponentWidgets(SPointLightComponent(), PointLightWidgetReference);
		tryAddComponentWidgets(SPrefabComponent(), PrefabWidgetReference);
		tryAddComponentWidgets(SSpotLightComponent(), SpotlightWidgetReference);
		tryAddComponentWidgets(SPhysics3DComponent(), ColliderWidgetReference, std::function<bool(const SPhysics3DComponent*)>([](const SPhysics3DComponent* component) { return component->IsTrigger; }));
	}

	void CEditorRenderSystem::OnEntityPreDestroy(const SEntity entity)
	{
		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		assetRegistry->UnrequestAsset(CameraWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(ColliderWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(DecalWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(DirectionalLightWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(EnvironmentLightWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(PointLightWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(PrefabWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(SpotlightWidgetReference, entity.GUID);
	}
}
